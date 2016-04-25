#include "stdafx.h"
#include "CameraDrive.h"
using namespace cv;

class BaseManager
{
public:
	BaseManager(){}
	~BaseManager(){}
	//����ȫ�����������ͷ�ĳ�ʼ��
	virtual int init() = 0;
	//��ȡһ֡����
	virtual cv::Mat getOneFrame(cv::VideoCapture& capture) = 0;
	//���������ļ�·��
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
	//��������ͷ���еķ��� ����ָ���������ͷ
	//int sendMessage(const char * message);
	//TrackingVideoProcessor Tvp;
private:

};