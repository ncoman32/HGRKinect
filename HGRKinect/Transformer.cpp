#include "Transformer.h"

Transformer::Transformer(int noOfPoints){
	this->noOfPoints = noOfPoints;
	this->inputData = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*noOfPoints);
	this->outputData = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*noOfPoints);
}

void Transformer::dftFolder(Path pathToFolder) {
	this->pathToFolder = pathToFolder;
	this->currentFilePath = this->pathToFolder.string() + "/sample/sample" + to_string(currentFileNo) + ".txt";
	this->currentFileToWrite = this->pathToFolder.string() + "/dft/dft" + to_string(currentFileNo) + ".txt";
	while (exists(currentFilePath)) {
		this->plan = fftw_plan_dft_1d(noOfPoints, inputData, outputData, FFTW_FORWARD, FFTW_MEASURE);
		updateStreams();
		for (int i = 0; i < this->noOfPoints; i++) {
			readingStream >> this->inputData[i][REAL];
			readingStream >> this->inputData[i][IMAG];
		}
		fftw_execute(this->plan);
		for (int i = 0; i < this->noOfPoints; i++) {
			writingStream << outputData[i][REAL] << " "
				<< outputData[i][IMAG] << endl;
		}
		incrementCurrentFileNumber();
		updateFileNames();
		fftw_destroy_plan(plan);
	}
	this->currentFileNo = 0;
}

svm_node* Transformer::getNodeForPrediction(vector<cv::Point> sampledPoints){
	// a feature vector has 398 features. allocate 399 because the end of the array
	// has to be marked as a -1 indexed node
	svm_node* toReturn =(svm_node*)malloc(399 * sizeof(svm_node));
	//apply dft
	this->plan = fftw_plan_dft_1d(this->noOfPoints, inputData, outputData, FFTW_FORWARD, FFTW_MEASURE);
	for (int i = 0; i < this->noOfPoints; i++) {
		this->inputData[i][REAL] = sampledPoints.at(i).x;
		this->inputData[i][IMAG] = sampledPoints.at(i).y;
	}
	fftw_execute(this->plan);
	
	vector<double> featureValues(398);
	//skip 2 values (coordinates of centroid)
	for (int i = 1; i < this->noOfPoints; i++) {
		featureValues.at(2 * (i - 1)) = outputData[i][REAL];
		featureValues.at(2 * (i - 1) + 1) = outputData[i][IMAG];
	}
	// scale data according to the feature ranges
	Instance instance = scale(Instance(-1, featureValues));
	//create the svm_node array. index starts at 1
	for (int k = 0; k < instance.instanceVector.size(); k++) {
		toReturn[k].index = k + 1;
		toReturn[k].value = instance.instanceVector.at(k);
	}
	// mark the final of the  the array with  a -1 index node
	toReturn[398].index = -1;

	fftw_destroy_plan(plan);
	return toReturn;
}

void Transformer::readValues(Path inputFolderPath) {
	closeStreams();
	this->currentFileNo = 0;
	updateFileNamesForCsv(inputFolderPath);
	vector<double> matrixLine;
	double currentValue;
	while (exists(currentFilePath)) {
		//skip first 4 values
		readingStream.ignore(10, ' ');
		readingStream.ignore(10, '\n');
		//read values from each file and find the minimum and maximum
		// I considered 200 points -> 400 coordinates, but the first 4 are skipped
		matrixLine.clear();
		for (int i = 0; i < 398; i++) {
			readingStream >> currentValue;
			matrixLine.push_back(currentValue);
		}
		this->features.push_back(Instance(this->instanceClass, matrixLine));
		incrementCurrentFileNumber();
		updateFileNamesForCsv(inputFolderPath);
	}
}

// call this method after all values are read and stored in the features vector
void Transformer::writeToCsv(Path destinationPath, bool uncomputedRanges) {
	this->writingStream.open(destinationPath, ios_base::app);
	if (uncomputedRanges) {
		findRanges(this->features);
	}
	for (int i = 0; i < features.size(); i++) {
		this->features.at(i) = scale(features.at(i));
		writingStream << features.at(i).instanceClass << " ";
		for (int j = 0; j < features.at(i).instanceVector.size(); j++) {
			writingStream << j + 1 << ":" << features.at(i).instanceVector.at(j) << " ";
		}
		writingStream << "\n";
	}
	this->features.clear();
	this->closeStreams();
}

void Transformer::updateStreams() {
	closeStreams();
	this->readingStream.open(this->currentFilePath);
	this->writingStream.open(this->currentFileToWrite);
}

void Transformer::closeStreams(){
	if (this->readingStream.is_open()) {
		this->readingStream.close();
	}
	if (writingStream.is_open()) {
		this->writingStream.close();
	}
}

void Transformer::updateFileNames() {
	this->currentFilePath = this->pathToFolder.string() + "/sample/sample" + to_string(currentFileNo) + ".txt";
	this->currentFileToWrite = this->pathToFolder.string() + "/dft/dft" + to_string(currentFileNo) + ".txt";
	this->readingStream.open(this->currentFilePath);
	this->writingStream.open(this->currentFileToWrite);
}

void Transformer::updateFileNamesForCsv(Path folderPath) {
	this->currentFilePath = folderPath.string() + "/dft" +to_string(currentFileNo) + ".txt";
	if (this->readingStream.is_open()) {
		this->readingStream.close();
	}
	this->readingStream.open(this->currentFilePath);
}

Transformer::Instance Transformer::scale(Instance feature) {
	vector<double> scaledValues;
	double newMax = 1.0;
	double newMin = -1.0;
	double newVal = 0.0;
	// if the ranges are not in already in memory load them from the file system
	if (featureValueRanges.empty()) {
		this->closeStreams();
		this->readingStream.open(FILE_RANGES_PATH);
		int noOfFeatures = 0;
		readingStream >> noOfFeatures;
		double min, max;
		for (int i = 0; i < noOfFeatures; i++) {
			readingStream >> min;
			readingStream >> max;
			featureValueRanges.push_back(Range(min, max));
		}
	}
	for (int i = 0; i < feature.instanceVector.size(); i++) {
		/*
			newVal = (oldVal - oldMin) / (oldMax - oldMin)  * (newMax -newMin) + newMin
		*/
		newVal = ((feature.instanceVector.at(i) - this->featureValueRanges.at(i).min) / (this->featureValueRanges.at(i).max - this->featureValueRanges.at(i).min))
			* (newMax - newMin) + newMin;
		scaledValues.push_back(newVal);
	}
	return Instance(feature.instanceClass, scaledValues);
}

void Transformer::findRanges(vector<Transformer::Instance> features) {
	int featuresNo = features.size();
	int featureVectorSize = features.at(0).instanceVector.size();
	for (int i = 0; i < featureVectorSize; i++) {
		double min = DBL_MAX;
		double max = -DBL_MAX;
		for (int j = 0; j < featuresNo; j++) {
			if (features.at(j).instanceVector.at(i) > max) {
				max = features.at(j).instanceVector.at(i);
			}
			if (features.at(j).instanceVector.at(i) < min) {
				min = features.at(j).instanceVector.at(i);
			}
		}
		this->featureValueRanges.push_back(Range(min, max));
	}
}

void Transformer::incrementCurrentFileNumber() {
	this->currentFileNo++;
}

void Transformer::setInstanceClass(int instanceClass) {
	this->instanceClass = instanceClass;
}

void Transformer::saveRangesToFile(Path rangesFile){
	closeStreams();
	this->writingStream.open(rangesFile);
	this->writingStream << featureValueRanges.size() << endl;
	for (int i = 0; i < this->featureValueRanges.size(); i++) {
		this->writingStream << this->featureValueRanges.at(i).min << " " << this->featureValueRanges.at(i).max << endl;
	}
	closeStreams();
}

void Transformer::loadRanges(Path rangesFile){
	closeStreams();
	this->readingStream.open(rangesFile);
	int rangesNo;
	double min, max;
	this->readingStream >> rangesNo;
	for (int i = 0; i < rangesNo; i++) {
		this->readingStream >> min;
		this->readingStream >> max;
		this->featureValueRanges.push_back(Range(min, max));
	}
	closeStreams();
}

Transformer::~Transformer(){
}
