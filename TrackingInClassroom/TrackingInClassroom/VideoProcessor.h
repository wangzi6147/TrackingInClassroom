#include "stdafx.h"
#include "CameraDrive.h"

using namespace cv;

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
	//CCameraDrive&	m_CameraDrive;
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
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	//bool detect_uperbody(Mat frame);
	void humanDetect(IplImage* img, vector<Rect>& regions);
	const char* cascade_name = "./haarcascade_frontalface_alt.xml";
	vector<Rect> ignore_area;
private:
	//���»��� ΪGMM��ģ�ͷ��� ���ڲ�����
	int updateOneFrame(cv::Mat);

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
	double getrlangle(Mat frame, Rect r);
	double getudangle(Mat frame, Rect r);
	bool detect_upperbody(IplImage* frame);
	vector<Rect> detect_and_draw(IplImage* img);
	String area_judge(IplImage* frame,Rect v);
	const char* cascade_name = "./haarcascade_mcs_upperbody.xml";
	//const char* cascade_name = "./haarcascade_frontalface_alt.xml";
private:
	int rect_center_x = 0, rect_center_y = 0, rect_x = 0, rect_y = 0,center_frame_width = 0, center_frame_height = 0;
	double tilt_angle_ud = 0, tilt_angle_rl = 0;
	double n = tan(0.167*CV_PI);
	
};