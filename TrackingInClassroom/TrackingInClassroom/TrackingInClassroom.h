
// TrackingInClassroom.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTrackingInClassroomApp: 
// �йش����ʵ�֣������ TrackingInClassroom.cpp
//

class CTrackingInClassroomApp : public CWinApp
{
public:
	CTrackingInClassroomApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTrackingInClassroomApp theApp;