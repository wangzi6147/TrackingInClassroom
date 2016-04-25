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

static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;

void detect_and_draw( IplImage* image );
//��������

const char* cascade_name = "d:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
//const char* cascade_name = "F:/����/dt.xml";
//��Ҫ�����ļ���OpenCV\data\haarcascades�ļ����п�������Ŀ�ļ�����

int main(int argc, char ** argv)
{
	videoInput vi;//������Ƶ�������
	vi.setupDevice(0);//�����豸
	int width=vi.getWidth(0);
	int height=vi.getHeight(0);
	IplImage *frame=cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	vi.showSettingsWindow(0);//����������ʾ��Ƶ���ô��ڣ�����ȥ��
	vi.setIdealFramerate(0, 60);

	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	 //��������������õķ�����
	if( !cascade )
	{
		fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
		fprintf( stderr,
			"Usage: facedetect --cascade=\"<cascade_path>\" [filename|camera_index]\n" );
		return -1;
	}
	storage = cvCreateMemStorage(0);
	//��̬�洢�ṹ�������洢������ͼ���е�λ��
	fprintf( stderr, "����ESC�˳�\n����s��ͼ��������D��\n" );
	while(1)
	{
		if(vi.isFrameNew(0))
		{
			vi.getPixels(0, (unsigned char *)frame->imageData, false, true);		//��ȡһ֡
			if( frame )
			{
				detect_and_draw( frame );						
			}
			char c=cvWaitKey(1);
			if(c == 27) 
			{
				break;
			}//��ESC�˳�
			if (c==115)
			{
				cvSaveImage("D:/my.jpg", frame);
			}
			cvNamedWindow("Video", CV_WINDOW_AUTOSIZE);
			cvShowImage("Video", frame);
		}
	}
	cvReleaseImage( &frame );
	return 0;
}

void detect_and_draw( IplImage* img )
{
	static CvScalar colors[] = 
	{
		{{0,0,255}},
		{{0,128,255}},
		{{0,255,255}},
		{{0,255,0}},
		{{255,128,0}},
		{{255,255,0}},
		{{255,0,0}},
		{{255,0,255}}
	};

	double scale = 1;
	IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
	IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
		cvRound (img->height/scale)),
		8, 1 );
	int i;

	cvCvtColor( img, gray, CV_BGR2GRAY );
	cvResize( gray, small_img, CV_INTER_LINEAR );
	cvEqualizeHist( small_img, small_img );
	cvClearMemStorage( storage );

	if( cascade )
	{
		double t = (double)cvGetTickCount();
		CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
			1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
			cvSize(30, 30) );
		t = (double)cvGetTickCount() - t;
		//printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
		for( i = 0; i < (faces ? faces->total : 0); i++ )
		{
			CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
			CvPoint center;
			int radius;
			center.x = cvRound((r->x + r->width*0.5)*scale);
			center.y = cvRound((r->y + r->height*0.5)*scale);
			radius = cvRound((r->width + r->height)*0.25*scale);
			cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
			cvRectangle(img,cvPoint(r->x,r->y),cvPoint(r->x+r->width,r->y+r->height),colors[0]);
		}
	}
	cvNamedWindow("Video", 0);
	//cvResizeWindow("Video", WIDTH, HEIGHT);
	cvShowImage( "Video", img );
	cvReleaseImage( &gray );
	cvReleaseImage( &small_img );
}