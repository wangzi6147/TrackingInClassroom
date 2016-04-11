#include "stdafx.h"

class BaseProcessor{
public:
	BaseProcessor(){}
	~BaseProcessor(){}
	//��ʼ��
	virtual int init() = 0;
	//���������ļ�·��
	virtual int setParams(const char * path) = 0;
	//�ڻ����в�׽Ŀ�꣨���Σ�
	virtual std::vector<cv::Rect> getTargets(cv::Mat frame) = 0;
private:
};

class StaticVideoProcessor:virtual public BaseProcessor
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
	//Haar���
	void detect_and_draw(IplImage* img);

	//GMM���
	cv::BackgroundSubtractor *pMOG2;
	cv::Mat fgMaskMOG2;
	int centerP = 2;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Rect r;
	//HOG
	cv::Mat ROI;
	cv::HOGDescriptor hog; // ����Ĭ�ϲ���
	//Haar
	CvHaarClassifierCascade* cascade;
	CvMemStorage* storage;
};

class TrackingVideoProcessor:virtual public BaseProcessor
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