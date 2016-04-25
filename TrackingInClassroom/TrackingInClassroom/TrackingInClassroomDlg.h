
// TrackingInClassroomDlg.h : ͷ�ļ�
//

#pragma once
#include "CameraDrive.h"
#include "mscomm.h"
#include "CameraManager.h"
#include "VideoProcessor.h"
#include "CameraDrive.h"

// CTrackingInClassroomDlg �Ի���
class CTrackingInClassroomDlg : public CDialogEx
{
// ����
public:
	/*TrackingCameraManager m_trackingmanager;
	StaticCameraManager m_staticmanager;
	StaticVideoProcessor Svp;
	TrackingVideoProcessor Tvp;
	CCameraDrive	m_CameraDrive;*/
	CMSComm	m_commPanTilt;
	CTrackingInClassroomDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TRACKINGINCLASSROOM_DIALOG };
	//StaticCameraManager Scm;
	//TrackingCameraManager Tcm;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	TrackingCameraManager m_trackingmanager;
	StaticCameraManager m_staticmanager;
	StaticVideoProcessor Svp;
	TrackingVideoProcessor Tvp;
	CCameraDrive	m_CameraDrive;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtontest();
};
