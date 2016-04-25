#include "stdafx.h"
#include "CameraManager.h"

using namespace cv;

StaticCameraManager::StaticCameraManager(){

}

StaticCameraManager::~StaticCameraManager(){

}

int StaticCameraManager::init(){
	return 0;
}

int StaticCameraManager::setParams(const char * path){
	return 0;
}

cv::Mat StaticCameraManager::getOneFrame(cv::VideoCapture& capture){
	cv::namedWindow("Frame");
	//clock_t start = clock();
	//clock_t stop;
	//cv::VideoCapture capture("F:/00015_h.avi");   //create the capture object
	//if (!capture.isOpened()){
	//	//error in opening the video input
	//	std::cerr << "Unable to open video file: " << "F:/00015_h.avi" << std::endl;
	//	exit(EXIT_FAILURE);
	//}
	//read input data. ESC or 'q' for quitting

	//char* s = new char;
	//while ((char)keyboard != 'q' && (char)keyboard != 27){
		//read the current frame
		if (!capture.read(frame)) {
			std::cerr << "Unable to read next frame." << std::endl;
			std::cerr << "Exiting..." << std::endl;
			//stop = clock();
			//double runtime = double(stop - start);//就是运行时间,单位ms
			//std::cout << runtime << std::endl;
			exit(EXIT_FAILURE);
		}
		//capture.release();
		//delete s;
		//return frame;
	//}
}