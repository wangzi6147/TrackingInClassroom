#include "stdafx.h"

class BaseManager
{
public:
	BaseManager();
	~BaseManager();
	//用于全景或跟踪摄像头的初始化
	virtual int init();
	//获取一帧画面
	virtual cv::Mat getOneFrame();
	//设置配置文件路径
	virtual int setParams(const char * path);
private:

};

class StaticCameraManager:BaseManager
{
public:
	StaticCameraManager();
	~StaticCameraManager();
	int init();
	cv::Mat getOneFrame();
	int setParams(const char * path);
private:

};

class TrackingCameraManager :BaseManager
{
public:
	TrackingCameraManager();
	~TrackingCameraManager();
	int init();
	cv::Mat getOneFrame();
	int setParams(const char * path);
	//跟踪摄像头独有的方法 发送指令控制摄像头
	int sendMessage(const char * message);
private:

};