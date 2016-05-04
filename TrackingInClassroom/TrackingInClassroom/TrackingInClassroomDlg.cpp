
// TrackingInClassroomDlg.cpp : 实现文件
//

#pragma comment(linker,"/NODEFAULTLIB:atlthunk.lib")

#include "stdafx.h"
#include "stdio.h"
#include "TrackingInClassroom.h"
#include "TrackingInClassroomDlg.h"
#include "afxdialogex.h"
#include "videoInput.h"

using namespace std;
using namespace cv;
#ifdef _EiC
#define WIN32
#endif

bool zoom_flag=true;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define SPEED 6

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

extern Ptr<BackgroundSubtractorMOG2> pMOG2;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTrackingInClassroomDlg 对话框



CTrackingInClassroomDlg::CTrackingInClassroomDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTrackingInClassroomDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTrackingInClassroomDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MSCOMM, m_commPanTilt);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTrackingInClassroomDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_test, &CTrackingInClassroomDlg::OnBnClickedButtontest)
END_MESSAGE_MAP()


// CTrackingInClassroomDlg 消息处理程序

Rect rect_combine(vector<Rect> vec)
{
	Rect rec;
	int minx = INT_MAX, miny = INT_MAX, maxw = 0, maxh = 0;
	if (vec.size()>0)
	{
		for (int i = 0; i<vec.size(); i++)
		{
			if ((vec[i]).x<minx)
			{
				minx = (vec[i]).x;
			}
			if ((vec[i]).y<miny)
			{
				miny = (vec[i]).y;
			}
			if (((vec[i]).x+(vec[i]).width)>maxw)
			{
				maxw = ((vec[i]).x + (vec[i]).width);
			}
			if (((vec[i]).y + (vec[i]).height)>maxh)
			{
				maxh = ((vec[i]).y + (vec[i]).height);
			}
		}
		rec.x=minx;
		rec.y = miny;
		rec.width = maxw-minx;
		rec.height = maxh-miny;
	}
	return rec;
}




BOOL CTrackingInClassroomDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	UpdateData(TRUE);
	m_CameraDrive.SetFlagofInfo(1);
	if (!m_CameraDrive.Init(&m_commPanTilt))
	{
		AfxMessageBox(L"Error!");
	}
	//从摄像头读取数据
    VideoCapture cap(1);
	cv::Mat frm;
	if (!cap.isOpened())
    {
		cout << "file open failed, exit with returning 1." << endl;
		return 1;
	}
	cap >> frm;
	//读入视频文件
	/*cv::VideoCapture capture("F:/00015_h.avi");   //create the capture object
	if (!capture.isOpened()){
		//error in opening the video input
		std::cerr << "Unable to open video file: " << "F:/00015_h.avi" << std::endl;
		exit(EXIT_FAILURE);
	}*/
	pMOG2 = new BackgroundSubtractorMOG2(20,10,false);  //MOG2 approach;
	videoInput vi;//创建视频捕获对象
	vi.setupDevice(0);//配置设备
	int width = vi.getWidth(0);
	int height = vi.getHeight(0);
	IplImage *frame1 = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	//vi.showSettingsWindow(0);//该语句可以显示视频设置窗口，可以去掉
	vi.setIdealFramerate(0, 60);
	Sleep(4000);
	//上半身检测
	bool tracking = false;
	while (1){
		vector<Rect>  sv_vec, tv_vec;
		int sv_size=0, tv_size=0;
		double  angle_rl, angle_ud;
		Rect sv_vec1, tv_vec1;
		String str;
		if (cap.read(frm)){
			 sv_vec = Svp.getTargets(frm);
			 sv_size = sv_vec.size();
            //屏蔽区域显示
				m_CameraDrive.Zoom(-5);
				for (int k = 0; k < Svp.ignore_area.size(); k++)
				{
					rectangle(frm, cv::Point(Svp.ignore_area[k].x, Svp.ignore_area[k].y), cv::Point(Svp.ignore_area[k].x + Svp.ignore_area[k].width, Svp.ignore_area[k].y + Svp.ignore_area[k].height), cv::Scalar(255, 0, 0), 2);
				}

				//计算特写摄像头转动角度
				if (!sv_vec.empty())
				{
					Rect sv_vec1 = rect_combine(sv_vec);
					angle_rl = Tvp.getrlangle(frm, sv_vec1);
					angle_ud = Tvp.getudangle(frm, sv_vec1);
					//rectangle(frm, sv_vec1,Scalar(0, 0, 255), 2);
					rectangle(frm, cv::Point(sv_vec1.x, sv_vec1.y), cv::Point(sv_vec1.x + sv_vec1.width, sv_vec1.y + sv_vec1.height), cv::Scalar(0, 0, 255), 2);
				}
				imshow("Hua", frm);
			}
		if (vi.isFrameNew(0)){
			vi.getPixels(0, (unsigned char*)frame1->imageData, false, true);
			if (frame1)
			{
				 tv_vec = Tvp.detect_and_draw(frame1);
				 tv_size = tv_vec.size();

				if (!tv_vec.empty())
				{
					Rect tv_vec1 = rect_combine(tv_vec);
					str = Tvp.area_judge(frame1, tv_vec1);
				}
			}
		}
		if (tv_size<sv_size)
		{
			m_CameraDrive.Absoluteto(angle_rl, angle_ud, SPEED);
		}
		else if (tv_size > sv_size)
		{
			if (tv_size == 1 && sv_size == 0)
			{
				double s = tv_vec1.width*tv_vec1.height;
				double zoom_in = (*frame1).width*(*frame1).height*0.2;
				double zoom_out = (*frame1).width*(*frame1).height*0.6;
				//cout << str << endl;
				if (str.compare("DownRight") == 0){
					m_CameraDrive.DownRight(SPEED);
				}
				else if (str.compare("Right") == 0)
				{
					m_CameraDrive.Right(SPEED);

				}
				else if (str.compare("UpRight") == 0)
				{
					m_CameraDrive.UpRight(SPEED);
				}
				else if (str.compare("Down") == 0)
				{
					m_CameraDrive.Down(SPEED);
				}
				else if (str.compare("Up") == 0)
				{
					m_CameraDrive.Up(SPEED);
				}
				else if (str.compare("DownLeft") == 0)
				{
					m_CameraDrive.DownLeft(SPEED);
				}
				else if (str.compare("Left") == 0)
				{
					m_CameraDrive.Left(SPEED);
				}
				else if (str.compare("UpLeft") == 0)
				{
					m_CameraDrive.UpLeft(SPEED);
				}
				else
				{
					m_CameraDrive.Stop();
					if (zoom_flag&&s < zoom_in)
					{
						m_CameraDrive.Zoom(2);
					}
					else if (zoom_flag&&s > zoom_out)
					{
						m_CameraDrive.Zoom(-2);
					}
					else
					{
						m_CameraDrive.Stop_Zoom();
					}
				}
			}
			else if (tv_size>1)
			{
				if (sv_size>0)
				{
					m_CameraDrive.Absoluteto(angle_rl, angle_ud, SPEED);
				}
				
			}
		}
		else if (tv_size==sv_size) 
		{
			double s = tv_vec1.width*tv_vec1.height;
			double zoom_in = (*frame1).width*(*frame1).height*0.2;
			double zoom_out = (*frame1).width*(*frame1).height*0.6;
			//cout << str << endl;
			if (str.compare("DownRight") == 0){
				m_CameraDrive.DownRight(SPEED);
			}
			else if (str.compare("Right") == 0)
			{
				m_CameraDrive.Right(SPEED);

			}
			else if (str.compare("UpRight") == 0)
			{
				m_CameraDrive.UpRight(SPEED);
			}
			else if (str.compare("Down") == 0)
			{
				m_CameraDrive.Down(SPEED);
			}
			else if (str.compare("Up") == 0)
			{
				m_CameraDrive.Up(SPEED);
			}
			else if (str.compare("DownLeft") == 0)
			{
				m_CameraDrive.DownLeft(SPEED);
			}
			else if (str.compare("Left") == 0)
			{
				m_CameraDrive.Left(SPEED);
			}
			else if (str.compare("UpLeft") == 0)
			{
				m_CameraDrive.UpLeft(SPEED);
			}
			else
			{
				m_CameraDrive.Stop();
				if (zoom_flag&&s < zoom_in)
				{
					m_CameraDrive.Zoom(2);
				}
				else if (zoom_flag&&s > zoom_out)
				{
					m_CameraDrive.Zoom(-2);
				}
				else
				{
					m_CameraDrive.Stop_Zoom();
				}
			}
		}
				
				char c = cvWaitKey(1);
				if (c == 27) break;//按ESC退出
				cvNamedWindow("hei", CV_WINDOW_AUTOSIZE);
				cvShowImage("hei", frame1);
			}
	cvReleaseImage(&frame1);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTrackingInClassroomDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTrackingInClassroomDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTrackingInClassroomDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTrackingInClassroomDlg::OnBnClickedButtontest()
{
	// TODO:  在此添加控件通知处理程序代码
	//m_CameraDrive.Left(5);
}
