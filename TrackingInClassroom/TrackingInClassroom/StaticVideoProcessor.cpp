#include "stdafx.h"
#include "VideoProcessor.h"

using namespace std;
using namespace cv;

StaticVideoProcessor::StaticVideoProcessor(){
	pMOG2 = new BackgroundSubtractorMOG2();
}

StaticVideoProcessor::~StaticVideoProcessor(){
	delete pMOG2;
	if (cascade != NULL){
		cvReleaseHaarClassifierCascade(&cascade);
	}
	if (storage != NULL){
		cvReleaseMemStorage(&storage);
	}
}

int StaticVideoProcessor::init(){
	//init HOG
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());  // 采用已经训练好的行人检测分类器
	//init Haar
	const char* cascade_name = "F:/haarcascade_mcs_upperbody.xml";
	cascade = (CvHaarClassifierCascade*)cvLoad(cascade_name, 0, 0, 0);
	storage = cvCreateMemStorage(0);
	return 0;
}

int StaticVideoProcessor::setParams(const char * path){
	return 0;
}

void StaticVideoProcessor::detect_and_draw(IplImage* img)
{
	static CvScalar colors[] =
	{
		{ { 0, 0, 255 } },
		{ { 0, 128, 255 } },
		{ { 0, 255, 255 } },
		{ { 0, 255, 0 } },
		{ { 255, 128, 0 } },
		{ { 255, 255, 0 } },
		{ { 255, 0, 0 } },
		{ { 255, 0, 255 } }
	};

	double scale = 1;
	IplImage* gray = cvCreateImage(cvSize(img->width, img->height), 8, 1);
	IplImage* small_img = cvCreateImage(cvSize(cvRound(img->width / scale),
		cvRound(img->height / scale)),
		8, 1);
	int i;

	cvCvtColor(img, gray, CV_BGR2GRAY);
	cvResize(gray, small_img, CV_INTER_LINEAR);
	cvEqualizeHist(small_img, small_img);
	cvClearMemStorage(storage);

	if (cascade)
	{
		double t = (double)cvGetTickCount();
		CvSeq* faces = cvHaarDetectObjects(small_img, cascade, storage,
			1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
			cvSize(50, 50));
		t = (double)cvGetTickCount() - t;
		//printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
		for (i = 0; i < (faces->total>0 ? 1 : 0); i++)
		{
			CvRect* r = (CvRect*)cvGetSeqElem(faces, i);
			CvPoint center;
			int radius;
			center.x = cvRound((r->x + r->width*0.5)*scale);
			center.y = cvRound((r->y + r->height*0.5)*scale);
			radius = cvRound((r->width + r->height)*0.25*scale);
			//cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
			cvRectangle(img, cvPoint(r->x, r->y), cvPoint(r->x + r->width, r->y + r->height), colors[0]);
		}
	}
	cvNamedWindow("Video", 0);
	cvShowImage("Video", img);
	cvReleaseImage(&gray);
	cvReleaseImage(&small_img);
}

vector<Rect> StaticVideoProcessor::getTargets(Mat frame){
	vector<Rect> v;
	vector<Rect> regions;
	pMOG2->operator()(frame, fgMaskMOG2, 0.001);
	erode(fgMaskMOG2, fgMaskMOG2, getStructuringElement(0, Size(2 * centerP + 1, 2 * centerP + 1), Point(centerP, centerP)));
	dilate(fgMaskMOG2, fgMaskMOG2, getStructuringElement(0, Size(2 * centerP + 1, 2 * centerP + 1), Point(centerP, centerP)));
	dilate(fgMaskMOG2, fgMaskMOG2, getStructuringElement(0, Size(2 * centerP + 1, 2 * centerP + 1), Point(centerP, centerP)));
	if (contours.size() > 0){
		contours.clear();
	}
	if (hierarchy.size() > 0){
		hierarchy.clear();
	}
	Mat forContours;
	fgMaskMOG2.copyTo(forContours);
	findContours(forContours, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	for (int i = 0; i < contours.size(); i++){
		if (contourArea(contours[i]) < frame.rows*frame.cols *0.02 || contourArea(contours[i]) > frame.rows*frame.cols*0.9){
			continue;
		}
		r = boundingRect(contours[i]);
		int temp = r.y>r.height / 3 ? r.y - r.height / 3 : 0;
		r = Rect(cvPoint(r.x, temp), cvPoint(r.x + r.width, r.y + r.height));
		ROI = frame(r);
		regions.clear();
		if (ROI.cols >= 32 && ROI.rows >= 32)
		{
			/*hog.detectMultiScale(ROI, regions, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 1);
			for (int j = 0; j < regions.size(); j++){
				v.push_back(Rect(Point(r.x + regions[j].x, r.y + regions[j].y), Point(r.x + regions[j].x + regions[j].width, r.y + regions[j].y + regions[j].height)));
			}*/
			IplImage img;
			img = IplImage(ROI);
			detect_and_draw(&img);
			rectangle(fgMaskMOG2, cv::Point(r.x, r.y), cv::Point(r.x + r.width, r.y + r.height), cv::Scalar(255, 255, 255), 1);
		}
	}
	imshow("FG Mask MOG 2", fgMaskMOG2);
	cvWaitKey(33);
	return v;
}