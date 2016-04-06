// CameraDrive.h: interface for the CCameraDrive class.
//
//////////////////////////////////////////////////////////////
//////////////CommControl  class for sony D70 camer//////////
/////////////////////designed by  ≤Òœ˛Ω‹///////////////////// 
//////////////////////////2008.10///////////////////////////   
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERADRIVE_H__6C9E6658_57A6_4F8F_8B7A_135CD54342AE__INCLUDED_)
#define AFX_CAMERADRIVE_H__6C9E6658_57A6_4F8F_8B7A_135CD54342AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mscomm.h"

class CCameraDrive  
{
public:
	CCameraDrive();
	virtual ~CCameraDrive();
	
public:
	void AutoPowerOff(int min);
	void CommandCancel();
	void check();
	void ApertureReset();
	void IR_Control_ReceiveReturn(bool flag);
	void IR_Control_Lock(bool flag);
	void lock(bool flag);
	void ImageFreeze(bool flag);
	void ApertureChange(bool flag);
	void Mute(bool flag);
	void PictureEffect(int flag);
	void Auto_ICR(bool flag);
	void ICR(bool flag);
	void power(bool flag);
	void Auto_mannal_Focus();
	void Maunal_Focus();
	void SetMoveSpeed(int speed);
	void GetZoomPos(double &dbZoomPos);
	void GetFocusPos(double &dbFocusPos);
	void QueryPos();
	void QueryZoomPos();
	void QueryFocusPos();
	void DirectTo(int pan_pos,int tilt_pos,int speed = 10);
	void Right(int speed = 10);
	void Left(int speed =10);
	void Up(int speed =10);
	void Down(int speed = 10);
	void RelativerRightTo(int pos,int speed =10 );
	void RelativeRightTo(double pos, int speed =10);
	void RelativeLeftTo(double pos, int speed =10);
	void RelativeUpTo(int pos, int speed =10);
	void RelativeDownTo(int pos, int speed =10);
	void DownRight(int panspeed, int tiltspeed =10);
	void DownLeft(int panspeed, int tiltspeed =10);
	void UpRight(int panspeed, int tiltspeed =10);
	void UpLeft(int panspeed, int tiltspeed =10);
	void RightTo(int pos, int speed =10);
	void LeftTo(int pos, int speed =10);
	void UpTo(int pos, int speed =10);
	void DownTo(int pos, int speed =10);
	void SetZoomPos(int Pos);
	void SetFocusPos(int Pos);
	void Backlight_compensation(bool flag);
	void Auto_Zoom_wide();
	void Auto_Zoom_tele();
	void Auto_Focus();
	void Auto();
	void Home();
	void Focus(int level);
	void Zoom(int level);
	void SetFlagofInfo(int flag);
	void GetOrient(double &dbOrient,double &dbtilt);
	void GetStateofCommand(CString &msg);
	BOOL CallMeOnComm(CMSComm* comm);
	void Stop();
	void Stop_PanTilt();
	void Stop_Zoom();
	void Stop_Focus();
	void Close();
	BOOL Init(CMSComm* comm);

private:

	BOOL BytestoZoomPos(BYTE * command,int num,double &ZoomPos);
	BOOL BytestoFocusPos(BYTE *command, int num, double &FocusPos);
	BOOL BytestoPos(BYTE *command, int num, double &orientation, double &tilt);
	BOOL BytestoCompleteSign(BYTE *command, int num);
	void SendData(BYTE *data, int count);

	CByteArray			OutBuf; 
	CMSComm				*m_Comm;
	double				m_dbOrient;				// …„œÒª˙∑ΩŒªΩ«
	double				m_dbtilt;				// …„œÒª˙«„–±Ω«
	double				m_dbFoucsPos;           // ∂‘ΩπŒª÷√£¨e£®œÒæ‡£©
	double              m_dbZoomPos;			// Ωπæ‡f
 	CString				 m_stateofCommand;//√¸¡Ó”Ôæ‰
	int					m_MoveSpeed;
	BYTE				m_byteInputBuf[100];
	int					m_iCountofData;
	int					m_iFlag;
    CArray<int,int>		m_queryFlag_array;
	int					m_QueryFlag;			// ≤È—Ø±Í÷æŒª£¨0(pan-tilt),1(Focus position),2(Zoom position)			

};

#endif // !defined(AFX_CAMERADRIVE_H__6C9E6658_57A6_4F8F_8B7A_135CD54342AE__INCLUDED_)
