#include "stdafx.h"
#include "CameraManager.h"

TrackingCameraManager::TrackingCameraManager(){

}

TrackingCameraManager::~TrackingCameraManager(){

}

int TrackingCameraManager::init(){
	return 0;
}

int TrackingCameraManager::setParams(const char * path){
	return 0;
}

cv::Mat TrackingCameraManager::getOneFrame(cv::VideoCapture& capture){
	cv::Mat frame(7, 7, CV_32FC2, Scalar(1, 3));
	return frame;
}