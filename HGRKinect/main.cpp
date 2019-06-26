#include "main.h"

// samples contour for a class (iterates through all files in "contour" folder and samples them, saves samples in "sample folder")
// iterates through all sample files from sample folder and applies dft on the sampled contour files, saves them in "dft" fodler
void sampleAndTransform(string clasz, bool trainData){
	

	if (trainData) {
	    create_directory("./dataset/train/" + clasz +"/sample");
		create_directory("./dataset/train/" + clasz + "/dft");

		sampler->sampleFolder(path("./dataset/train/" + clasz));
		transformer->dftFolder("./dataset/train/" + clasz);
	}
	else {
		create_directory("./dataset/test/" + clasz + "/sample");
		create_directory("./dataset/test/" + clasz + "/dft");

		sampler->sampleFolder(path("./dataset/test/" + clasz));
		transformer->dftFolder("./dataset/test/" + clasz);
	}
}

// saves processed data to csv files ( train.csv, test.csv)
// the values are scaled according to train data (min,max) values for each feature
void saveToCsv(int noOfClasses, vector<path> trainPaths, vector<path> testPaths) {
 
	for (int i = 0; i < noOfClasses; i++) {
		transformer->setInstanceClass(i);
		transformer->readValues(trainPaths.at(i).string());
	}
	transformer->writeToCsv(path("./dataset/train.csv"), true);
	cout << "Train set built" << endl;
	transformer->saveRangesToFile(path("./dataset/ranges.txt"));
	cout << "Feature ranges saved" << endl;
	
	for (int i = 0; i < noOfClasses; i++) {
		transformer->setInstanceClass(i);
		transformer->readValues(testPaths.at(i).string());
	}
	transformer->writeToCsv(path("./dataset/test.csv"), false);	
	cout << "Test set built" << endl;
}

void loadFIlePaths(vector<path> &trainPaths, vector<path> &testPaths){
	Ifstream testPathsFile;
	Ifstream trainPathsFile;

	trainPathsFile.open(path("./trainFolderPaths.txt"));
	testPathsFile.open(path("./testFolderPaths.txt"));

	string filePath;
	while(getline(trainPathsFile, filePath)) {
		cout << "Loading: " + filePath << endl;
		trainPaths.push_back(path(filePath));
	}

	while (getline(testPathsFile, filePath)) {
		cout << "Loading: " + filePath << endl;
		testPaths.push_back(path(filePath));
	}
	testPathsFile.close();
	trainPathsFile.close();
}

void processLetter(string letter) {
	if (letter.compare("") == 0 ) {
		return;
	}
	sampleAndTransform(letter, true);
	sampleAndTransform(letter, false);
}
//  TODO: add letter U
int main(int argc, char** argv) {
	
	int option;
	cout << "Menu: " << endl;
	cout << "0 - Classification using Kinect" << endl;
	cout << "1 - Process a new class" << endl;
	cout << "2 - Build the dataset" << endl;
	cin >> option;

	switch (option) {
		case 0: {
			RGBDSensor* sensor = new OpenNISensor();
			sensor->scan();
			delete sensor;
			break;
		}
		case 1: {
			string letter("");
			cout << "What is the letter of the class ?" << endl;
			cin >> letter;
			processLetter(letter);	
			break;
		}
		case 2: {
			loadFIlePaths(trainPaths, testPaths);
			saveToCsv(21, trainPaths, testPaths);
			break;
		}
		default: break;
	}
	
	return 0;
}