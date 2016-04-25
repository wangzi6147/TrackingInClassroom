#include "videoInput.h"
#include "stdio.h"
#include "cv.h"
#include "highgui.h"

#pragma comment(linker,"/NODEFAULTLIB:atlthunk.lib")

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#ifdef _EiC
#define WIN32
#endif

#include "stdafx.h"
#include "VideoProcessor.h"

using namespace std;
using namespace cv;

static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
//const char* cascade_name = "./haarcascade_frontalface_alt.xml";


TrackingVideoProcessor::TrackingVideoProcessor(){

}

TrackingVideoProcessor::~TrackingVideoProcessor(){

}

int TrackingVideoProcessor::init(){
	return 0;
}

int TrackingVideoProcessor::setParams(const char * path){
	return 0;
}

std::vector<cv::Rect> TrackingVideoProcessor::getTargets(Mat frame){
	vector<Rect> v;
	Rect r = cvRect(0, 0, 0, 0);
	v.push_back(r);
	return v;
}

double TrackingVideoProcessor::getrlangle(Mat frame,vector<Rect> v){
	center_frame_width = frame.cols / 2;
	center_frame_height = frame.rows / 2;
	//vector<Rect>::iterator it;
	//for (it = v.begin(); it != v.end(); it++){
	if (v.size()>0){
		rect_center_x = (v[0]).x + (v[0]).width / 2;
		rect_x = rect_center_x - center_frame_width;
		double camera_d = center_frame_width / n;
		tilt_angle_rl = atan2(rect_x, camera_d)*(180 / CV_PI);
	}
	cout << "rl:"<<tilt_angle_rl <<endl;
	//}
	return tilt_angle_rl;
}

double TrackingVideoProcessor::getudangle(Mat frame,vector<Rect> v){
	center_frame_width = frame.cols / 2;
	center_frame_height = frame.rows / 4;
	if (v.size() > 0){
		rect_center_y = (v[0]).y + (v[0]).height / 2;
		rect_y = center_frame_height - rect_center_y;
		double camera_d = center_frame_width / n;
		tilt_angle_ud = atan2(rect_y, camera_d)*(180 / CV_PI);
		if (tilt_angle_ud < 0){
			tilt_angle_ud = 0;
		}
	}
	//cout <<"ud:"<< tilt_angle_ud << endl;
	return tilt_angle_ud;
}

bool TrackingVideoProcessor::detect_upperbody(IplImage* frame){
	//while (1){
			if (frame)
			{
				detect_and_draw(frame);
			}
			char c = cvWaitKey(1);
	//}
	//cvReleaseImage(&frame);
	return 0;
}

vector<Rect> TrackingVideoProcessor::detect_and_draw(IplImage* img)
{
	vector<Rect> v;
	CvPoint center;
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
	IplImage* small_img = cvCreateImage(cvSize(cvRound(img->width / scale),cvRound(img->height / scale)),8, 1);
	int i;

	cvCvtColor(img, gray, CV_BGR2GRAY);
	cvResize(gray, small_img, CV_INTER_LINEAR);
	cvEqualizeHist(small_img, small_img);
//	cvClearMemStorage(storage);
	storage = cvCreateMemStorage(0);
	cascade = (CvHaarClassifierCascade*)cvLoad(cascade_name, 0, 0, 0);
	//if (!cascade)
	//{
	//	fprintf(stderr, "ERROR: Could not load classifier cascade\n");
	//	fprintf(stderr,
	//		"Usage: facedetect --cascade=\"<cascade_path>\" [filename|camera_index]\n");
	//}
	if (cascade)
	{
		double t = (double)cvGetTickCount();
		CvSeq* upperbodies = cvHaarDetectObjects(small_img, cascade, storage, 1.1, 2, 0, cvSize(200, 200));
		t = (double)cvGetTickCount() - t;
		//printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
		//for (i = 0; i < (upperbodies ? upperbodies->total : 0); i++)
		//{
		if (upperbodies){
			CvRect* r = (CvRect*)cvGetSeqElem(upperbodies,0);
			int radius;
			if (r){
				center.x = cvRound((r->x + r->width*0.5)*scale);
				center.y = cvRound((r->y + r->height*0.5)*scale);
				radius = cvRound((r->width + r->height)*0.25*scale);
				cvCircle(img, center, radius, colors[0 % 8], 3, 8, 0);
				cvRectangle(img, cvPoint(r->x, r->y), cvPoint(r->x + r->width, r->y + r->height), colors[0]);
				v.push_back(*r);
			}
		
		}
			
		//}
	}
	//cvNamedWindow("Video", 0);
	//cvResizeWindow("Video", WIDTH, HEIGHT);
	//cvShowImage("Video", img);
	cvReleaseImage(&gray);
	cvReleaseImage(&small_img);
	return v;
}

String TrackingVideoProcessor::area_judge(IplImage* frame,vector<Rect> v){
	String str;
	Mat mtr;
	mtr = frame;
	int frame_width = mtr.cols;
	int frame_height = mtr.rows;
	int blob_width = frame_width / 3;
	int blob_height = frame_height / 3;
	if (!v.empty())
	{
		int center_x = (v[0]).x + (v[0]).width / 2;
		int center_y = (v[0]).y + (v[0]).height / 2;
		if (v.size() > 0)
		{
			if (0 < center_x && center_x < blob_width)
			{
				if (0 < center_y && center_y < blob_height)
				{
					str = "UpLeft";
				}
				else if (blob_height < center_y && center_y <2 * blob_height)
				{
					str = "Left";
				}
				else if (2 * blob_height < center_y && center_y< frame_height)
				{
					str = "DownLeft";
				}
			}
			else if (blob_width< center_x && center_x < 2 * blob_width)
			{
				if (0 < center_y && center_y< blob_height)
				{
					str = "Up";
				}
				else if (blob_height < center_y && center_y<2 * blob_height)
				{
					str = "stay";
				}
				else if (2 * blob_height < center_y && center_y < frame_height)
				{
					str = "Down";
				}
			}
			else if (2 * blob_width< center_x && center_x< frame_width)
			{
				if (0 < center_y && center_y < blob_height)
				{
					str = "UpRight";
				}
				else if (blob_height < center_y && center_y <2 * blob_height)
				{
					str = "Right";
				}
				else if (2 * blob_height < center_y && center_y < frame_height)
				{
					str = "DownRight";
				}
			}
		}
	}
	return str;
}