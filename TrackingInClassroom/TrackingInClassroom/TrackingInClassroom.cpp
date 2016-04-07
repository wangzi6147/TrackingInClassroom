// TrackingInClassroom.cpp : 定义控制台应用程序的入口点。
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
	//主程序入口
	//CMSComm mycomm;
	//HWND my_hWnd;

	//CCameraDrive mycamera;
	//mycamera.SetFlagofInfo(1);
	//mycamera.Init(&mycomm);//这句会有问题！！！！！！！！！！！！！！！！！！！
	//mycamera.DownLeft(120, 10);


	//videoprocess部分测试
	const char * path = "../res/test.avi";
	testVideoProcessor(path);

	return 0;
}

