#include "stdafx.h"

class BaseManager
{
public:
	BaseManager(){}
	~BaseManager(){}
	//����ȫ�����������ͷ�ĳ�ʼ��
	virtual int init() = 0;
	//��ȡһ֡����
	virtual cv::Mat getOneFrame() = 0;
	//���������ļ�·��
	virtual int setParams(const char * path) = 0;
private:

};

class StaticCameraManager:virtual public BaseManager
{
public:
	StaticCameraManager();
	~StaticCameraManager();
	int init();
	cv::Mat getOneFrame();
	int setParams(const char * path);
private:

};

class TrackingCameraManager :virtual public BaseManager
{
public:
	TrackingCameraManager();
	~TrackingCameraManager();
	int init();
	cv::Mat getOneFrame();
	int setParams(const char * path);
	//��������ͷ���еķ��� ����ָ���������ͷ
	int sendMessage(const char * message);
private:

};