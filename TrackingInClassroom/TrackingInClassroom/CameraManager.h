#include "stdafx.h"

class BaseManager
{
public:
	BaseManager();
	~BaseManager();
	//����ȫ�����������ͷ�ĳ�ʼ��
	virtual int init();
	//��ȡһ֡����
	virtual cv::Mat getOneFrame();
	//���������ļ�·��
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
	//��������ͷ���еķ��� ����ָ���������ͷ
	int sendMessage(const char * message);
private:

};