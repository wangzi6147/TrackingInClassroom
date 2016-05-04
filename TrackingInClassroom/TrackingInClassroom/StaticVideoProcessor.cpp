#include "stdafx.h"
#include "VideoProcessor.h"
#include <iostream>
#include <sstream>
#include <fstream>

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/mat.hpp>

using namespace std;
using namespace cv;


static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
//const char* cascade_name2 = "./haarcascade_frontalface_alt.xml";


Ptr<BackgroundSubtractorMOG2> pMOG2; //MOG2 Background subtractor

StaticVideoProcessor::StaticVideoProcessor(){
	//读取敏感区域
	stringstream recall1;
	stringstream recall2;
	stringstream recall3;
	stringstream recall4;
	char temprec[10];
	vector<string> rec;
	ifstream ifile;
	ifile.open("configure.txt");     //打开矩形坐标文件
	Rect ROI;
	if (!ifile.is_open())
	{
		cout << "Error opening ifile"; exit(1);
	}
	while (!ifile.eof())
	{
		for (int i = 0; i < 4; i++)
		{
			ifile>>temprec;
			//cout << temprec << endl;
			rec.push_back(temprec);
		}
		recall1 << rec[0];
		recall1 >> ROI.x; recall1.clear();
		recall2 << rec[1];
		recall2 >> ROI.y; recall2.clear();
		recall3 << rec[2];
		recall3 >> ROI.width; recall3.clear();
		recall4 << rec[3];
		recall4 >> ROI.height; recall4.clear();
		ignore_area.push_back(ROI);
		rec.clear();
	}
	ifile.close();
}

StaticVideoProcessor::~StaticVideoProcessor(){
	
}

int StaticVideoProcessor::init(){
	return 0;
}

int StaticVideoProcessor::setParams(const char * path){
	return 0;
}

bool overlap_judge(CvRect rect1, CvRect rect2)
{
	if (rect1.x > rect2.x + rect2.width)
		return false;
	if (rect1.y > rect2.y + rect2.height)
		return false;
	if (rect2.x > rect1.x + rect1.width)
		return false;
	if (rect2.y > rect1.y + rect1.height)
		return false;
	int col_over = min(rect1.x + rect1.width, rect2.x + rect2.width) - max(rect1.x, rect2.x);
	int row_over = min(rect1.y + rect1.height, rect2.y + rect2.height) - max(rect1.y, rect2.y);
	int area_overlap = col_over*row_over;
	int area1 = rect1.width*rect1.height;
	int area2 = rect2.width*rect2.height;
	float overlap_ratio = (float)area_overlap / (float)(area1 + area2 - area_overlap);
	if (overlap_ratio > 0.3)
		return 1;
}

vector<Rect> StaticVideoProcessor::getTargets(Mat frame){
	vector<Rect> v;
	bool ignore_flag = false;
	//global variables
	Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
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
	if (contours.size() > 0){
		contours.clear();
	}
	if (hierarchy.size() > 0){
		hierarchy.clear();
	}
	int keyboard;
	int width = frame.cols;
	int height = frame.rows;
	double maxarea = width*height*0.2;
	double minarea = width*height*0.1;
		//用mog2处理
		//cv::namedWindow("FG Mask MOG 2");
	pMOG2->operator()(frame, fgMaskMOG2, -0.1);

	//腐蚀膨胀运算
	int dilation_size = 5;
	int erosion_size = 2;
	cv::Mat dilate_element = getStructuringElement(cv::MORPH_RECT,  // dilation_type = MORPH_RECT / MORPH_CROSS / MORPH_ELLIPSE
		cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
		cv::Point(dilation_size, dilation_size));
	cv::Mat erosion_element = getStructuringElement(cv::MORPH_RECT,  // dilation_type = MORPH_RECT / MORPH_CROSS / MORPH_ELLIPSE
		cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		cv::Point(erosion_size, erosion_size));
	erode(fgMaskMOG2, fgMaskMOG2, cv::Mat());

	/*提取轮廓*/
	CvMemStorage* mem_storage = cvCreateMemStorage(0);
	//CvSeq *first_contour = NULL, *c = NULL;
	imwrite("contour.jpg", fgMaskMOG2);
	cvNamedWindow("contour_test");
	cvNamedWindow("contour_raw");
	IplImage* img = cvLoadImage("contour.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	cvShowImage("contour_raw", img);
	cvThreshold(img, img, 128, 255, CV_THRESH_BINARY);
	IplImage* img_temp = cvCloneImage(img);
	//vector<vector<Point>> contours;
	Rect rect;
	Mat ROI;
	vector<Rect> regions;
	findContours(fgMaskMOG2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//cvFindContours(img_temp, mem_storage, &first_contour, sizeof(CvContour), CV_RETR_CCOMP);
	for (int i = 0; i < contours.size(); i++){
		if (contourArea(contours[i]) < frame.rows*frame.cols / 40 || contourArea(contours[i]) > frame.rows*frame.cols*0.8){
			continue;
		}
		rect = boundingRect(contours[i]);
		int temp = rect.y>rect.height / 3 ? rect.y - rect.height / 3 : 0;
		rect = Rect(cvPoint(rect.x, temp), cvPoint(rect.x + rect.width, rect.y + rect.height));
		ROI = frame(rect);
		regions.clear();
		
		if (ROI.cols >= 32 && ROI.rows >= 32)
		{
			IplImage* img = cvCreateImage(cvSize(fgMaskMOG2.cols, fgMaskMOG2.rows), IPL_DEPTH_8U, 3);
			*img = ROI;
		//	humanDetect(img, regions);
			for (int k = 0; k < ignore_area.size(); k++)
			{
				if (overlap_judge(rect, ignore_area[k]))
				{
					ignore_flag = true;
					break;
				}
			}
			if (!ignore_flag)
			{
				rectangle(frame, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), cv::Scalar(255, 255, 255), 2);
				v.push_back(rect);
			}
			ignore_flag = false;
		}
	}
	//return regions;
	cvReleaseImage(&img_temp);
	cvReleaseImage(&img);
	cvReleaseMemStorage(&mem_storage);
	return v;
}
	
void StaticVideoProcessor::humanDetect(IplImage* img, vector<Rect>& regions)
{
	double scale = 1;
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
	IplImage* gray = cvCreateImage(cvSize(img->width, img->height), 8, 1);
	IplImage* small_img = cvCreateImage(cvSize(cvRound(img->width / scale),cvRound(img->height / scale)),8, 1);
	int i;

	cvCvtColor(img, gray, CV_BGR2GRAY);
	cvResize(gray, small_img, CV_INTER_LINEAR);
	cvEqualizeHist(small_img, small_img);
	//cvClearMemStorage(storage);
	storage = cvCreateMemStorage(0);
	cascade = (CvHaarClassifierCascade*)cvLoad(cascade_name, 0, 0, 0);
	if (cascade)
	{
		double t = (double)cvGetTickCount();
		CvSeq* faces = cvHaarDetectObjects(small_img, cascade, storage,1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,cvSize(50, 50), cvSize(200, 200));
		t = (double)cvGetTickCount() - t;
		//printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
		for (i = 0; i < (faces ? faces->total : 0); i++)
		{
			CvRect* r = (CvRect*)cvGetSeqElem(faces, i);
			//CvPoint center;
			if (r->x < 0 || r->x + r->width>img->width){
				continue;
			}
			regions.push_back(cv::Rect(r->x, r->y, r->width, 2 * r->height));
			//int radius;
			//center.x = cvRound((r->x + r->width*0.5)*scale);
			//center.y = cvRound((r->y + r->height*0.5)*scale);
			//radius = cvRound((r->width + r->height)*0.25*scale);
			//cvCircle(img, center, radius, colors[i % 8], 3, 8, 0);
            cvRectangle(img, cvPoint(r->x, r->y), cvPoint(r->x + r->width, r->y + r->height), colors[0]);
		}
	}
	cvReleaseImage(&gray);
	cvReleaseImage(&small_img);
	cvReleaseMemStorage(&storage);
	cvReleaseHaarClassifierCascade(&cascade);
}