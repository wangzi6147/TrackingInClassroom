#include "stdafx.h"
#include "CameraDrive.h"

using namespace cv;

class BaseProcessor{
public:
	BaseProcessor(){}
	~BaseProcessor(){}
	//初始化
	virtual int init() = 0;
	//设置配置文件路径
	virtual int setParams(const char * path) = 0;
	//在画面中捕捉目标（人形）
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
	//在画面中捕捉目标（人形）
	std::vector<cv::Rect> getTargets(cv::Mat frame);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	//bool detect_uperbody(Mat frame);
	void humanDetect(IplImage* img, vector<Rect>& regions);
	const char* cascade_name = "./haarcascade_frontalface_alt.xml";
	vector<Rect> ignore_area;
private:
	//更新画面 为GMM等模型服务 类内部调用
	int updateOneFrame(cv::Mat);

};

class TrackingVideoProcessor:virtual public BaseProcessor
{
public:
	TrackingVideoProcessor();
	~TrackingVideoProcessor();
	int init();
	int setParams(const char * path);
	//在画面中捕捉目标（人形）
	std::vector<cv::Rect> getTargets(cv::Mat frame);

	//TODO 这里考虑是否加入其它检测/追踪目标的方式
	//例如将StaticCamera的目标图像传递给TrackingCamera去做匹配
	//或者加入运动方向的匹配 还有光流法等追踪方法
	//目的是为了增加追踪的准确性
	//我们之前的思路是StaticCamera去修正TrackingCamera
	//现在考虑是否反过来 用TrackingCamera修正StaticCamera的结果
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