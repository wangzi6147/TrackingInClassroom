#include "stdafx.h"

class BaseProcessor{
public:
	BaseProcessor();
	~BaseProcessor();
	//��ʼ��
	virtual int init();
	//���������ļ�·��
	virtual int setParams(const char * path);
	//�ڻ����в�׽Ŀ�꣨���Σ�
	std::vector<cv::Rect> getTargets(cv::Mat frame);
private:
};

class StaticVideoProcessor:BaseProcessor
{
public:
	StaticVideoProcessor();
	~StaticVideoProcessor();
	int init();
	int setParams(const char * path);
	//�ڻ����в�׽Ŀ�꣨���Σ�
	std::vector<cv::Rect> getTargets(cv::Mat frame);

private:
	//���»��� ΪGMM��ģ�ͷ��� ���ڲ�����
	int updateOneFrame(cv::Mat);
};

class TrackingVideoProcessor:BaseProcessor
{
public:
	TrackingVideoProcessor();
	~TrackingVideoProcessor();
	int init();
	int setParams(const char * path);
	//�ڻ����в�׽Ŀ�꣨���Σ�
	std::vector<cv::Rect> getTargets(cv::Mat frame);

	//TODO ���￼���Ƿ�����������/׷��Ŀ��ķ�ʽ
	//���罫StaticCamera��Ŀ��ͼ�񴫵ݸ�TrackingCameraȥ��ƥ��
	//���߼����˶������ƥ�� ���й�������׷�ٷ���
	//Ŀ����Ϊ������׷�ٵ�׼ȷ��
	//����֮ǰ��˼·��StaticCameraȥ����TrackingCamera
	//���ڿ����Ƿ񷴹��� ��TrackingCamera����StaticCamera�Ľ��
private:

};