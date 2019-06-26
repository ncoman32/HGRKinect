#ifndef OPENNISENSOR_H
#define OPENNISENSOR_H

#include "RGBDSensor.h"
#include "global.h"
#include "OpenNI.h"
#include "svm.h"
#include "Transformer.h"
#include "Sampler.h"

using namespace cv;

class OpenNISensor : public RGBDSensor {
public:
	OpenNISensor();
	~OpenNISensor();
	bool init();
	void scan();

private:
	openni::Device			m_device;
	openni::VideoStream		m_depthStream;
	openni::VideoFrameRef	m_depthFrame;
	svm_model* classifier;
	void classify(vector<Point> contourPoints);
	bool m_flagInitSuccessful;
	bool m_flagShowImage;

	Transformer* transformer = new Transformer(200);
	Sampler* sampler = new Sampler(200);
 };

	
#endif