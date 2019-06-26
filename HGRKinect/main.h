#include "OpenNISensor.h"
#include "global.h"

vector<path> trainPaths;
vector<path> testPaths;
void loadFIlePaths(std::vector<path> &trainPaths, std::vector<path> &testPaths);
void saveToCsv(int noOfClasses, std::vector<path> trainPaths, std::vector<path> testPaths);
void sampleAndTransform(string clasz, bool trainData);

Transformer* transformer = new Transformer(200);
Sampler* sampler = new Sampler(200);
