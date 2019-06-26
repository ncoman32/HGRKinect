#include "Sampler.h"

Sampler::Sampler(int noOfPoints){
	this->noOfSamplePoints = noOfPoints;
	this->pointsToWrite.reserve(noOfPoints);
}

void Sampler::sampleFolder(Path pathToFolder) {
	this->pathToFolder = pathToFolder;
	this->currentFilePath = this->pathToFolder.string() + "/contour/contour" + to_string(currentFileNo) + ".txt";
	this->currentFileToWrite = this->pathToFolder.string() + "/sample/sample" + to_string(currentFileNo) + ".txt";
	while (boost::filesystem::exists(currentFilePath)) {
		updateStreams();
		int noOfPointsToRead;
		int x, y;
		float floatIndex;
		readingStream >> noOfPointsToRead;
		for (int i = 0; i < noOfPointsToRead; i++) {
			readingStream >> x;
			readingStream >> y;
			this->readPoints.push_back(cv::Point(x, y));
		}
		for (float i = 0.0; i < (float)this->noOfSamplePoints; i++) {
			floatIndex = ( i * (float)noOfPointsToRead ) / (float)this->noOfSamplePoints;
			writingStream << readPoints.at(std::round(floatIndex)).x
				<< " " << readPoints.at(std::round(floatIndex)).y << std::endl;
		}
		incrementCurrentFileNumber();
		updateFileNamesForSampling();
		this->readPoints.clear();
	}
	this->closeStreams();
	this->currentFileNo = 0;
}

vector<cv::Point> Sampler::sampleContour(vector<cv::Point> contour){
	vector<cv::Point> toReturn;
	float floatIndex;
	for (float i = 0.0; i < (float)this->noOfSamplePoints; i++) {
		floatIndex = (i * (float)contour.size()) / (float)this->noOfSamplePoints;
		toReturn.push_back(cv::Point(contour.at(std::round(floatIndex)).x, contour.at(std::round(floatIndex)).y));
	}
	return toReturn;
}

void Sampler::incrementCurrentFileNumber() {
	this->currentFileNo++;
}

void Sampler::updateStreams() {
	closeStreams();
	this->readingStream.open(this->currentFilePath);
	this->writingStream.open(this->currentFileToWrite);
}

void Sampler::closeStreams() {
	if (this->readingStream.is_open()) {
		this->readingStream.close();
	}
	if (this->writingStream.is_open()) {
		this->writingStream.close();
	}
}

void Sampler::updateFileNamesForSampling(){
	this->currentFilePath = this->pathToFolder.string() + "/contour/contour" + to_string(currentFileNo) + ".txt";
	this->currentFileToWrite = this->pathToFolder.string() + "/sample/sample" + to_string(currentFileNo) + ".txt";
	this->readingStream.open(this->currentFilePath);
	this->writingStream.open(this->currentFileToWrite);
}

Sampler::~Sampler(){
}
