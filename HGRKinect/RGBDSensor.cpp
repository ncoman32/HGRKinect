#include "RGBDSensor.h"

void RGBDSensor::createRGBDFolders() {
	string folder = "./save";
	int i = 0;
	while (true) {
		string dir = folder + to_string(i);
		if (boost::filesystem::is_directory(dir)) {
			i++;
		}
		else {
			savingPath = dir;
			string dirContour = dir + "/contour/";
			boost::filesystem::create_directory(dir);
			boost::filesystem::create_directory(dirContour);
			break;
		}
	}
}