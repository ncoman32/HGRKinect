#pragma once
#include "global.h"

class Sampler{

private:
	Path pathToFolder;
	Ifstream readingStream;
	Ofstream writingStream;
	Path currentFilePath;
	Path currentFileToWrite;
 	int noOfSamplePoints;
	int currentFileNo = 0;
	vector<cv::Point> readPoints;
	vector<cv::Point> pointsToWrite;
	void updateStreams();
	void closeStreams();
	void incrementCurrentFileNumber();
	void updateFileNamesForSampling();
public:
	void sampleFolder(Path pathToFolder);
	vector<cv::Point> sampleContour(vector<cv::Point> contour);
	Sampler(int);
	~Sampler();
};

