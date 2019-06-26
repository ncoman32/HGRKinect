#pragma once

#include "global.h"
#include "fftw3.h"
#include <string>
#include <opencv2/opencv.hpp>
#include "svm.h"
#define REAL 0
#define IMAG 1

class Transformer{

private:
	struct Instance {
		int instanceClass;
		std::vector<double> instanceVector;
		Instance(int clasz, std::vector<double> vector) {
			instanceClass = clasz;
			instanceVector = vector;
		}
	};

	struct Range {
		double min;
		double max;
		Range(double mini, double maxi) {
			min = mini;
			max = maxi;
		}
	};
	int instanceClass = NULL;
	std::vector<Transformer::Instance> features;
	std::vector<Transformer::Range> featureValueRanges;

	fftw_complex *inputData;
	fftw_complex *outputData;
	fftw_plan plan;
	int noOfPoints;
	int currentFileNo = 0;
	Ifstream readingStream;
	Ofstream writingStream;
	Path pathToFolder;
	Path currentFilePath;
	Path currentFileToWrite;
	void updateStreams();
	void closeStreams();
	void updateFileNames();
	void incrementCurrentFileNumber();
	void updateFileNamesForCsv(Path folderPath);
	Instance scale(Instance feature);
	void findRanges(vector<Transformer::Instance> features);
	const string FILE_RANGES_PATH = "./dataset/ranges.txt";

public:
	Transformer(int);
	~Transformer();
	void dftFolder(Path pathToFolder); // applies dft on all contour files from the given folder 
	svm_node* getNodeForPrediction(vector<cv::Point> sampledPoints);
	void readValues(Path inputFolderPath); // read all values from all files in the folder into the feature vectors
	void writeToCsv(Path destinationPath, bool uncomputedRanges); // writes scaled data to a csvFile
	void setInstanceClass(int instanceClass);
	void saveRangesToFile(Path rangesFile);
	void loadRanges(Path rangesFile);
};

