#include "stdafx.h"
#include "VideoProcessor.h"

using namespace std;
using namespace cv;

StaticVideoProcessor::StaticVideoProcessor(){
	pMOG2 = new BackgroundSubtractorMOG2();
}

StaticVideoProcessor::~StaticVideoProcessor(){
	
}

int StaticVideoProcessor::init(){
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());  // 采用已经训练好的行人检测分类器
	return 0;
}

int StaticVideoProcessor::setParams(const char * path){
	return 0;
}

vector<Rect> StaticVideoProcessor::getTargets(Mat frame){
	vector<Rect> v;
	vector<Rect> regions;
	pMOG2->operator()(frame, fgMaskMOG2, -0.1);
	erode(fgMaskMOG2, fgMaskMOG2, getStructuringElement(0, Size(2 * centerP + 1, 2 * centerP + 1), Point(centerP, centerP)));
	dilate(fgMaskMOG2, fgMaskMOG2, getStructuringElement(0, Size(2 * centerP + 1, 2 * centerP + 1), Point(centerP, centerP)));
	dilate(fgMaskMOG2, fgMaskMOG2, getStructuringElement(0, Size(2 * centerP + 1, 2 * centerP + 1), Point(centerP, centerP)));
	if (contours.size() > 0){
		contours.clear();
	}
	if (hierarchy.size() > 0){
		hierarchy.clear();
	}
	findContours(fgMaskMOG2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	for (int i = 0; i < contours.size(); i++){
		if (contourArea(contours[i]) < frame.rows*frame.cols / 40 || contourArea(contours[i]) > frame.rows*frame.cols*0.8){
			continue;
		}
		r = boundingRect(contours[i]);
		int temp = r.y>r.height / 3 ? r.y - r.height / 3 : 0;
		r = Rect(cvPoint(r.x, temp), cvPoint(r.x + r.width, r.y + r.height));
		ROI = frame(r);
		regions.clear();
		if (ROI.cols >= 32 && ROI.rows >= 32)
		{
			hog.detectMultiScale(ROI, regions, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 1);
			for (int j = 0; j < regions.size(); j++){
				v.push_back(Rect(Point(r.x + regions[j].x, r.y + regions[j].y), Point(r.x + regions[j].x + regions[j].width, r.y + regions[j].y + regions[j].height)));
			}
			rectangle(fgMaskMOG2, cv::Point(r.x, r.y), cv::Point(r.x + r.width, r.y + r.height), cv::Scalar(255, 255, 255), 1);
		}
	}
	imshow("FG Mask MOG 2", fgMaskMOG2);
	cvWaitKey(33);
	return v;
}