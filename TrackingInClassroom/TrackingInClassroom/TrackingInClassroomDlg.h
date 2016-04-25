
// TrackingInClassroomDlg.h : 头文件
//

#pragma once
#include "CameraDrive.h"
#include "mscomm.h"
#include "CameraManager.h"
#include "VideoProcessor.h"
#include "CameraDrive.h"

// CTrackingInClassroomDlg 对话框
class CTrackingInClassroomDlg : public CDialogEx
{
// 构造
public:
	/*TrackingCameraManager m_trackingmanager;
	StaticCameraManager m_staticmanager;
	StaticVideoProcessor Svp;
	TrackingVideoProcessor Tvp;
	CCameraDrive	m_CameraDrive;*/
	CMSComm	m_commPanTilt;
	CTrackingInClassroomDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TRACKINGINCLASSROOM_DIALOG };
	//StaticCameraManager Scm;
	//TrackingCameraManager Tcm;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	TrackingCameraManager m_trackingmanager;
	StaticCameraManager m_staticmanager;
	StaticVideoProcessor Svp;
	TrackingVideoProcessor Tvp;
	CCameraDrive	m_CameraDrive;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtontest();
};
