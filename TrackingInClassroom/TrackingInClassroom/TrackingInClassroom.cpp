// TrackingInClassroom.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "VideoProcessor.h"
#include "CameraDrive.h"
#include "mscomm.h"

using namespace cv;
using namespace std;

int testVideoProcessor(const char * path){
	StaticVideoProcessor svp;
	VideoCapture vc;
	vc.open(path);
	Mat frame;
	vector<Rect> v;
	namedWindow("show");
	if (vc.isOpened()){
		svp.init();
		while (vc.read(frame)){
			v = svp.getTargets(frame);
			for (Rect r : v){
				cout << "x:" << r.x << " y:" << r.y << " w:" << r.width << " h:" << r.height << endl;
				rectangle(frame, r, Scalar(255, 0, 0));
			}
			imshow("show", frame);
			int key = cvWaitKey(33);
			if (key == 'q')
				break;
			else if (key == ' '){
				while (cvWaitKey(-1) != ' ')
					continue;
			}
		}
	}
	vc.release();
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	//���������
	//CMSComm mycomm;
	//HWND my_hWnd;

	//CCameraDrive mycamera;
	//mycamera.SetFlagofInfo(1);
	//mycamera.Init(&mycomm);//���������⣡������������������������������������
	//mycamera.DownLeft(120, 10);


	//videoprocess���ֲ���
	const char * path = "../res/test.avi";
	testVideoProcessor(path);

	return 0;
}

