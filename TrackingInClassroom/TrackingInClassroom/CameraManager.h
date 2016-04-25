#include "stdafx.h"
#include "CameraDrive.h"
using namespace cv;

class BaseManager
{
public:
	BaseManager(){}
	~BaseManager(){}
	//用于全景或跟踪摄像头的初始化
	virtual int init() = 0;
	//获取一帧画面
	virtual cv::Mat getOneFrame(cv::VideoCapture& capture) = 0;
	//设置配置文件路径
	virtual int setParams(const char * path) = 0;
	//CCameraDrive	m_CameraDrive;
private:

};

class StaticCameraManager: public BaseManager
{
public:
	StaticCameraManager();
	~StaticCameraManager();
	int init();
	cv::Mat getOneFrame(cv::VideoCapture& capture);
	int setParams(const char * path);
	//StaticVideoProcessor Svp;
private:
	int keyboard;
	Mat frame;
};

class TrackingCameraManager :virtual public BaseManager
{
public:
	TrackingCameraManager();
	~TrackingCameraManager();
	int init();
	cv::Mat getOneFrame(cv::VideoCapture& capture);
	int setParams(const char * path);
	//跟踪摄像头独有的方法 发送指令控制摄像头
	//int sendMessage(const char * message);
	//TrackingVideoProcessor Tvp;
private:

};