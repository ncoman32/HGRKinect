#ifndef GLOBAL_H
#define GLOBAL_H

#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <float.h>
#include <opencv2/opencv.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread/thread.hpp>

using namespace boost::filesystem;
using namespace std;

using Ifstream = boost::filesystem::ifstream;
using Ofstream = boost::filesystem::ofstream;
using Path = boost::filesystem::path;
using Thread = boost::thread;

typedef unsigned short DepthValueType;
const float c_depthScaleFactor = 5.0;


#endif