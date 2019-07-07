#include "OpenNISensor.h"
#include <queue>
#include "svm.h"
#include "Transformer.h"
#include "Sampler.h"

OpenNISensor::OpenNISensor() {
	m_flagInitSuccessful = m_flagShowImage = true;
	m_frameNum = m_frameIdx = 0;
	m_sensorType = 0;
	classifier = svm_load_model("./dataset/train.csv.model");
	transformer = new Transformer(200);
	sampler = new Sampler(200);
	init();
}

OpenNISensor::~OpenNISensor() {
	m_depthStream.stop();
	m_depthStream.destroy();
	m_device.close();
	openni::OpenNI::shutdown();
}
// objects used for the classifying task
	
svm_node* instanceToClassify = (svm_node*)malloc(398 * sizeof(svm_node));


//mat variables used to process and store data fed by kinect
Mat depthRoiWithCross;
Mat depthRoi = Mat(200, 200, CV_8UC1);
Mat depthRoiToSave = Mat(200, 200, CV_8UC1);

//mat variables used to store data in filesystem
Mat handMat = Mat(200,200,CV_8UC1);
Mat contourMat;
vector<Point> handBorder;

bool OpenNISensor::init() {
	openni::Status rc = openni::STATUS_OK;
	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK) {
		cerr << "OpenNI Initial Error: " << openni::OpenNI::getExtendedError() << endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}

	const char* deviceURI = openni::ANY_DEVICE;
	rc = m_device.open(deviceURI);
	if (rc != openni::STATUS_OK) {
		cerr << "ERROR: Can't Open Device: " << openni::OpenNI::getExtendedError() << endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}

	rc = m_depthStream.create(m_device, openni::SENSOR_DEPTH);

	if (rc == openni::STATUS_OK) {
		rc = m_depthStream.start();

		if (rc != openni::STATUS_OK) {
			cerr << "ERROR: Can't start depth stream on device: " << openni::OpenNI::getExtendedError() << endl;
			m_depthStream.destroy();
			m_flagInitSuccessful = false;
			return m_flagInitSuccessful;
		}
	}
	else {
		cerr << "ERROR: This device does not have depth sensor" << endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}

	if (!m_depthStream.isValid()) {
		cerr << "SimpleViewer: No valid streams. Exiting" << endl;
		m_flagInitSuccessful = false;
		openni::OpenNI::shutdown();
		return m_flagInitSuccessful;
	}

	openni::VideoMode depthVideoMode = m_depthStream.getVideoMode();
	m_depthWidth = depthVideoMode.getResolutionX();
	m_depthHeight = depthVideoMode.getResolutionY();
	cout << "Depth = (" << m_depthWidth << "," << m_depthHeight << ")" << endl;
	
	// Set exposure if needed
	m_flagInitSuccessful = true;
	// load features ranges used for scaling
	transformer->loadRanges(Path("D:/Uni/PRS/RGBDCapture/RGBDCapture/dataset/ranges.txt"));
	return m_flagInitSuccessful;
}

void writeContour(int m_frameIdx, string m_strRGBDFolder) {
	char fileName[50];
	int numberOfPoints = handBorder.size();
	sprintf_s(fileName, "contour%d.txt", m_frameIdx);
	Path p = m_strRGBDFolder + "/contour/" + fileName;
	Ofstream boundaryFile(p);
	// The first and the last point are the same, so it is written only once
	boundaryFile << handBorder.size() - 1 << endl;
	// The Fourier Descriptors method requires the points to be ordered in  clockwise order
	// The border tracing algorithm obtains the contour points in an anti-clockwise order, that's why the points are saved in reversed order
	for (int i = numberOfPoints - 1; i >= 1 ; i--) {
		boundaryFile << handBorder.at(i).x << " " << handBorder.at(i).y << endl;
	}
	cout << "SAVED: " + p.string() + "\n"; ;
}

// use region growing 
// BFS used for region growing
Mat detectHand(Mat srcMat, int delta) {
	//8 neighbors
	int di[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dj[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	//flag used to mark visited pixels
	int visited = 1;

	Mat toReturn = Mat::zeros(srcMat.rows, srcMat.cols, CV_8UC1);
	Mat labels = Mat::zeros(srcMat.rows, srcMat.cols, CV_8UC1);

	// part of hand will always be in the middle of the image
	//scan the bottom row of the image and get the minimum value which is different than 0 ( 0 is noise in raw acquired data)
	int min = 256;

	Point seed;
	for (int i = 0; i < srcMat.cols; i++) {
		if (srcMat.at<uchar>(srcMat.rows - 1, i) < min && srcMat.at<uchar>(srcMat.rows - 1, i) > 0) {
			min = srcMat.at<uchar>(srcMat.rows - 1, i);
			seed.x = i;
			seed.y = srcMat.rows -1;
		}
	}
	int referenceValue = min;

	//copy seedPoint to result mat and start bfs
	queue<Point> queue;
	queue.push(seed);
	toReturn.at<uchar>(seed.y, seed.x) = srcMat.at<uchar>(seed.y, seed.x);
	labels.at<uchar>(seed.y, seed.x) = visited;
	Point currentPoint;
	while (!queue.empty()) {
		currentPoint = queue.front();
		queue.pop();
		for (int k = 0; k < 8; k++) {
			//make sure that work is done withing image bounds
			if (currentPoint.x + di[k] > 0 && currentPoint.y + dj[k] > 0
				&& currentPoint.x + di[k] < srcMat.rows && currentPoint.y + dj[k] < srcMat.cols) {

				//compare pixel values and start building result image
				if (abs(referenceValue - srcMat.at<uchar>(currentPoint.y + dj[k], currentPoint.x + di[k])) < delta) {
					toReturn.at<uchar>(currentPoint.y, currentPoint.x) = 255;

					//if neighbors not visited, mark them as visited and process point
					if (labels.at<uchar>(currentPoint.y + dj[k], currentPoint.x + di[k]) == 0) {
						labels.at<uchar>(currentPoint.y + dj[k], currentPoint.x + di[k]) = visited;
						queue.push({ currentPoint.x + di[k], currentPoint.y + dj[k] });
					}
				}
			}
		}
	}
	return toReturn;
}

// used to extract contour from a grayscale image
// extracts points in anti-clockwise orientation
void getBorderPoints(Mat src) {

	Point pointZero, pointOne, pointN, pointNMinusOne;
	vector<Point> border;
	int di[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	int dj[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	contourMat = Mat(src.rows, src.cols, CV_8UC1, Scalar(0));
	int dir = 7;
	int currentX = -1;
	int currentY = -1;
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) == 255) {
				pointZero = Point(j, i);
				currentX = j;
				currentY = i;
				i = src.rows;
				j = src.cols;
			}
		}
	}

	if (currentX == -1 && currentY == -1) {
		contourMat = Mat::zeros(contourMat.size(),CV_8UC1);
		return;
	}

	int n = 0;
	do {
		if (dir % 2 == 1) {
			dir = (dir + 6) % 8;
		}
		else {
			dir = (dir + 7) % 8;
		}
		 
			while (src.at<uchar>(currentY + di[dir], currentX + dj[dir]) != 255) {
				dir = (dir + 1) % 8;
			}

			n++;
			if (n == 1) {
				pointOne = Point(currentX + dj[dir], currentY + di[dir]);
			}

			pointNMinusOne = Point(currentX, currentY);
			pointN = Point(currentX + dj[dir], currentY + di[dir]);

			border.push_back(pointN);
			contourMat.at<uchar>(currentY + di[dir], currentX + dj[dir]) = 255;

			currentY = currentY + di[dir];
			currentX = currentX + dj[dir];
	} while (!(n > 1 && pointN == pointOne && pointNMinusOne == pointZero));

	handBorder = border;
}

// used to extract the pixels on which the circle overlays
Mat cropCircle(Mat imageToBeCropped, bool colorImage) {
	
	if (!colorImage) {
		Mat toReturn = Mat(imageToBeCropped.rows, imageToBeCropped.cols, CV_8UC1, Scalar(0));
		circle(toReturn, Point(toReturn.rows / 2, toReturn.cols / 2), 60, Scalar(255), -1);
		for (int i = 0; i < imageToBeCropped.rows; i++) {
			for (int j = 0; j < imageToBeCropped.cols; j++) {
				if (toReturn.at<uchar>(i, j) == 255) {
					toReturn.at<uchar>(i, j) = imageToBeCropped.at<uchar>(i, j);
				}
			}
		}
		return toReturn;
	}
	else {
		Mat toReturn = Mat(imageToBeCropped.rows, imageToBeCropped.cols, CV_8UC3, Scalar(0, 0, 0));
		circle(toReturn, Point(toReturn.rows / 2, toReturn.cols / 2), 55, Scalar(255, 255, 255), -1);
		for (int i = 0; i < imageToBeCropped.rows; i++) {
			for (int j = 0; j < imageToBeCropped.cols; j++) {
				if (toReturn.at<Vec3b>(i, j) == Vec3b(255, 255, 255)) {
					toReturn.at<Vec3b>(i, j) = imageToBeCropped.at<Vec3b>(i, j);
				}
			}
		}
		return toReturn;
	}
}

void putText(Mat mat, string text) {
	putText(mat, text, Point(10, 20), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255));
}
 
void OpenNISensor::classify(vector<Point> contourPoints) {
	// in order to apply dft, the points must be in clock-wise order, so the vector must be
	// also the first and the last pixel are the same, so delete the last element
	if (contourPoints.size() > 200) {
		reverse(contourPoints.begin(), contourPoints.end());
		contourPoints.erase(contourPoints.end() - 1);
		vector<Point> sampled = sampler->sampleContour(contourPoints);
		instanceToClassify = transformer->getNodeForPrediction(sampled);
		double predicted = svm_predict(classifier, instanceToClassify);
		switch ((int)predicted) {
		case 0:; putText(contourMat, "a");  break;
		case 1:  putText(contourMat, "b"); break;
		case 2:  putText(contourMat, "c"); break;
		case 3:  putText(contourMat, "d"); break;
		case 4:  putText(contourMat, "f"); break;
		case 5:  putText(contourMat, "g"); break;
		case 6:  putText(contourMat, "h"); break;
		case 7:  putText(contourMat, "5"); break;
		case 8:  putText(contourMat, "i"); break;
		case 9:  putText(contourMat, "k"); break;
		case 10: putText(contourMat, "L"); break;
		case 11: putText(contourMat, "p"); break;
		case 12: putText(contourMat, "q"); break;
		case 13: putText(contourMat, "r"); break;
		case 14: putText(contourMat, "s"); break;
		case 15: putText(contourMat, "t"); break;
		case 16: putText(contourMat, "u"); break;
		case 17: putText(contourMat, "v"); break;
		case 18: putText(contourMat, "w"); break;
		case 19: putText(contourMat, "x"); break;
		case 20: putText(contourMat, "y"); break;
		default: break;
		}
	} else {
		cout << endl << "The contour of the hand did not have 200 points. Try putting the hand closer to the camera!" << endl;
	}
}

void OpenNISensor::scan() {
	if (!m_flagInitSuccessful) {
		cout << "WARNING: initialize the device at first!" << endl;
		return;
	}

	//create folred in which captures will be saved
	createRGBDFolders();

	if (m_device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_OFF)) {
		if (0 == m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF)) {
			cout << "No registration! ";
		}
	}

	while (true) {
		m_depthStream.readFrame(&m_depthFrame);
		if (m_depthFrame.isValid()) {
			//convert the image fed by kinect to a 8UC1 so it will be supported by imshow
			Mat mImageDepth(m_depthHeight, m_depthWidth, CV_16UC1, (void*)m_depthFrame.getData());

			Mat depthMap = Mat(mImageDepth.rows, mImageDepth.cols, CV_8UC1);
			mImageDepth.convertTo(depthMap, CV_8UC1, 0.1);

			//extract a region of iterest
			depthRoi = depthMap(Rect(200, 100, 200, 200));
			Mat depthRoiWithCircle = depthRoi.clone();

			//draw crosshair and circle over displayed image
			circle(depthRoiWithCircle, Point(depthRoiWithCircle.rows / 2, depthRoiWithCircle.cols / 2), 60, Scalar(0, 0, 255));

			//show the image with the chrosshair
			imshow("ROI", depthRoiWithCircle);

		}
		else {
			cerr << "ERROR: Cannot read depth frame from depth stream. Quitting..." << endl;
			return;
		}

		// when C pressed
		// binarize hand out of image, crop circle around the hand, extract contour points
		// sample points, apply dft, scale dft results, feed to classifier, output result
		if (waitKey(1) == 99) {
			handMat = detectHand(depthRoi, 10);
			handMat = cropCircle(handMat, false);
			getBorderPoints(handMat);
			classify(handBorder);
			imshow("contour", contourMat);
		}

		// S - take a snap
		if (waitKey(1) == 115) {
			handMat = detectHand(depthRoi, 10);
			imshow("contour", handMat);
			depthRoiToSave = depthRoi.clone();
			handMat = cropCircle(handMat, false);
			getBorderPoints(handMat);
			vector<cv::Point> sampled = sampler->sampleContour(handBorder);
			cv::Mat sampledMat = cv::Mat::zeros(handMat.size(), CV_8UC1);
			for (int i = 0; i < sampled.size(); i++) {
				sampledMat.at<uchar>(sampled.at(i).y, sampled.at(i).x) = 255;
			}
			imshow("contour", contourMat);
			imshow("sampled", sampledMat);
		}

		// if K pressed save contour along with corresponding frames
		if (cvWaitKey(5) == 107) {
				Thread contourThread(writeContour, m_frameIdx, savingPath);
				m_frameIdx++;
		}

		// exit when ESC pressed
		if (waitKey(5) == 27) {
			break;
		}

	}
}