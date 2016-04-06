// CameraDrive.cpp: implementation of the CCameraDrive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "CommCTLPANTILT.h"
#include "CameraDrive.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/* 
	CCameraDrive，摄像机云台控制类，2007年4月，MEI Shuqi

	基于MSComm控件的云台控制，区别于以前做的基于API的串口通讯控制

  1、初始化与关闭：
	BOOL Init(CMSComm* comm);
	void Close();

  2、摄像机运动命令函数：
	void GetOrient(double &dbOrient);
	void GetStateofCommand(CString &msg);
	void RelativeRightTo(WORD pos, BYTE speed);
	void RelativeLeftTo(WORD pos, BYTE speed);
	void DownTo(WORD pos, BYTE speed);
	void DownRight(BYTE panspeed, BYTE tiltspeed);
	void DownLeft(BYTE panspeed, BYTE tiltspeed);
	void UpRight(BYTE panspeed, BYTE tiltspeed);
	void UpLeft(BYTE panspeed, BYTE tiltspeed);
	void Left(BYTE speed);
	void Right(BYTE speed);
	void ToRight(WORD pos, BYTE speed);
	void LeftTo(WORD pos, BYTE speed);
	void Stop();
	void Stop_PanTilt();
	void Stop_Zoom();
	void Stop_Focus();

  3、查询摄像机姿态角：	void QueryPos();

	设置查询命令类型：	void SetFlagofInfo(int flag);
	如果flag为：“1”，则查询摄像机运动指令是否完成；
				“2”，则查询摄像机的姿态角。

  4、此函数专用于被串口事件响应函数调用：
	int  CallMeOnComm(CMSComm* comm);
	它解析串口接收的数据的含义。


////////////////////////////////////////////////////////////////////////////////////////
///                          chai xiaojie          2008.7.27                      //////
控制字解析   81 01 06 03 FF 00  0F 07 02 05 0F 0E 07 00 FF
以下按位置解析  
*1*  81为10000001，第一位是标志，紧跟的三位是发送端编号，主机的编号为0，第四位也是
标志位,0.紧跟的后三位是接受端编号，只有一个摄像头时编号为1，十六进制表示为0x81.
*2*			01 for command ，09 for inquiry
*3*			06 for pan/tilter (00 for interface, 04 for camera)
*4*			03 for (pan/tilter control)relative position   (01 for free control, 02 for absoluter position, 04 for home, 05 for reset)
*5*			for pan speed 01 to 18
*6*			for tilt speed 01 to 17
*78910*		for pan position F725 to 08DB(center 0000),right is positive(0000 to 08DB, 十六进制表示，0.075度/1)
*11121314*  for tilt position FE70 to 04B0(center 0000),up is positive
*15*        FF for end


*/
const int	datasize	= 512;
const DWORD pos_rlim	= 0x08DB;	// 摄像机右极大位置
const DWORD pos_llim	= 0xF725;	// 摄像机左极大位置
const BYTE	maxspeed	= 0x18;		// 24
const BYTE	minspeed	= 0x01;		// 1

CCameraDrive::CCameraDrive()
{
	m_Comm		= NULL;

	m_iCountofData	= 0;
	m_iFlag			= 2;
	m_dbZoomPos		= 0;
	m_dbFoucsPos	= 0;
	m_dbOrient		= 0;
	m_dbtilt		= 0; 
	m_MoveSpeed		= 10;
	m_QueryFlag = 0;
	m_queryFlag_array.SetSize(5,1);
}

CCameraDrive::~CCameraDrive()
{
}

// 类初始化，实际是初始化串口
BOOL CCameraDrive::Init(CMSComm* comm)
{
	// 串口初始化
	if(comm->GetPortOpen())
		comm->SetPortOpen(FALSE);
	comm->SetCommPort(1);								// COM1
	comm->SetInBufferSize(1024);
	comm->SetOutBufferSize(1024);
	comm->SetInputLen(0);								// 设为0，表示全部读取
	comm->SetInputMode(1);								// 二进制方式读写数据
	comm->SetRThreshold(1);								// 当接收缓冲区中有n个以上字符时
														// 引发OnComm事件
	comm->SetSettings("9600, N, 8, 1");					// 波特率9600，无校验，8数据位，1停止位

	m_Comm = comm;
	if(!comm->GetPortOpen() )  
	{
		comm->SetPortOpen(TRUE);// 打开串口
        power(true);            // 打开电源
		// 摄像机Reset
	     Home();				// Home command

		 Auto_Focus();			//  Auto Focus

		return TRUE;
	}
	else
	{
		AfxMessageBox("cannot open serial port");
		return FALSE;
	}
}

void CCameraDrive::Close()
{
	if (m_Comm)
	{
		m_Comm->SetPortOpen(FALSE);
	}
}


void CCameraDrive::Stop_PanTilt()
{
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0x10,						// command[4] for pan-speed 01-18
		0x10,                       //for tilt speed 01-17
		0x03, 0x03,	0xFF};          
	m_stateofCommand = "stop pan/tilt!";	
	SendData(command, sizeof(command));
}


void CCameraDrive::QueryPos()
{
	m_QueryFlag = 0;
	m_queryFlag_array.Add(m_QueryFlag);
	BYTE command[5] = {0x81, 0x09, 0x06, 0x12, 0xFF};

//	m_stateofCommand = "Inquirie pan_tilt pos!";
	SendData(command, sizeof(command));
}

void CCameraDrive::SendData(BYTE *data, int count)
{
	if (m_Comm)
	{
		OutBuf.RemoveAll();
		OutBuf.SetSize(count);
		for (int i=0; i<count; i++)
		{
			OutBuf.SetAt(i, data[i]);
		}

		m_Comm->SetOutput(COleVariant(OutBuf));
	}
}


// 解析串口所传回的摄像机状态命令字，在CMSComm控件的事件响应OnComm函数中直接调用
// m_dbOrient:	转换后的角度，负数表示摄像机目前是左转了orentation度角，
//				角度范围在-170~170
// 
// return:		如果是摄像机执行命令完成信号，则返回1；如果是状态信号，返回2；
// 				如果什么都不是，返回－1。
BOOL CCameraDrive::CallMeOnComm(CMSComm* comm)
{
	VARIANT			variant;
    COleSafeArray	arraydata;
    LONG			len, k;
    BYTE			data[50]; 
	BOOL			flag;
	
	if(comm->GetCommEvent() == 2)					//事件值为2表示接收缓冲区内有字符
	{
		variant		= comm->GetInput();				//读缓冲区
        arraydata	= variant;						//VARIANT型变量转换为ColeSafeArray型变量
        len			= arraydata.GetOneDimSize();	//得到有效数据长度
        for(k=0; k<len; k++)
		{
			arraydata.GetElement(&k, data+k);	
			TRACE(_T("%d "),data[k]);
			if(k == len-1)
			{
				TRACE(_T("\n"));
			}
		}
				//转换为BYTE型数组

		if ((data[0] & 0x90) == 0x90)
		{
			m_iCountofData = 0;
		}
		int offset = m_iCountofData;
		m_iCountofData += (int)len;
		memcpy(m_byteInputBuf+offset, data, (int)len);
		BytestoCompleteSign(m_byteInputBuf, m_iCountofData);
		if(m_queryFlag_array.GetSize() > 0)
		{
			m_QueryFlag = m_queryFlag_array.GetAt(0);
			if(m_QueryFlag ==0)
				flag = BytestoPos(m_byteInputBuf, m_iCountofData, m_dbOrient,m_dbtilt);
			if(m_QueryFlag ==1)
				flag = BytestoFocusPos(m_byteInputBuf, m_iCountofData,m_dbFoucsPos);
			if(m_QueryFlag ==2)
				flag =BytestoZoomPos(m_byteInputBuf, m_iCountofData,m_dbZoomPos);
			m_queryFlag_array.RemoveAt(0);
		}
	}
	
	return flag;
}



// 解析串口所传回的摄像机状态命令字
// command:		命令字
// num:			命令字的个数
// 
// return:		如果是摄像机执行命令完成信号，则返回1；否则返回－1。
BOOL CCameraDrive::BytestoCompleteSign(BYTE *command, int num)
{
	if (num < 3)
	{
		return -1;
	}

	const int length_completion	= 3;
	BYTE Bytes[length_completion];
	int i;
	int iStart	= 0;

	// 定位0xFF
	for (i=0; i<num; i++)
	{
		if (command[i] == 0xFF)
		{
			if ((i-iStart+1) == length_completion)
			{
				memcpy(Bytes, command+iStart, length_completion);
				if (Bytes[0] == 0x90 && Bytes[1] == 0x51 && Bytes[2] == 0xFF)
				{
					m_stateofCommand = "Command Completion!";
					return 1;
				}
			}
			iStart = i+1;
		}
	}

	return -1;
}

void CCameraDrive::GetStateofCommand(CString &msg)
{
	msg = m_stateofCommand;
}

void CCameraDrive::GetOrient(double &dbOrient,double &dbtilt)
{
	dbOrient = m_dbOrient;
	dbtilt = m_dbtilt;
}

// 如果m_iFlag=1，则查询摄像机的运动是否结束
// 如果m_iFlag=2，则查询摄像机的角度状态
void CCameraDrive::SetFlagofInfo(int flag)
{
	m_iFlag = flag;
}

BOOL CCameraDrive::BytestoPos(BYTE *command, int num, double &orientation, double &tilt)
{
	int i, j;
int iStart	= 0;
	
	// 检查命令字中是否含有位姿信息，如果命令字包含摄像机状态，则更新变量
	if (num >= 11)
	{
		const int length_state		= 11;
		const int posMask_left	= 0xFFFFF000;	// 左转掩模
		const int posMask_right	= 0;			// 右转掩模
		const int posMask_down	= 0xFFFFF000;	// 左转掩模
		const int posMask_up	= 0;			// 右转掩模
		int pan_pos;								// 姿态信息，即摄像机左转或右转的角度
		int tilt_pos;								// 姿态信息，即摄像机上偏或下偏的角度
		BYTE stateBytes[length_state];
		
		// 定位0xFF
		for (i=0; i<num; i++)
		{
			if (command[i] == 0xFF)
			{
				if ((i-iStart+1) == length_state)
				{
					memcpy(stateBytes, command+iStart, length_state);
					if (stateBytes[0] == 0x90 && stateBytes[1] == 0x50)
					{
						if (stateBytes[2] == 0x0F)		// 左转角度
						{
							pan_pos = posMask_left;
						}
						else
						{
							pan_pos = posMask_right;
						}
						if (stateBytes[6] == 0x0F)		// 下偏角度
						{
							tilt_pos = posMask_down;
						}
						else
						{
							tilt_pos = posMask_up;
						}
						for(j=3; j<=5; j++)
						{
							pan_pos |= stateBytes[j] << (4*(5-j));
						}
						for(j=7; j<=9; j++)
						{
							tilt_pos |= stateBytes[j] << (4*(9-j));
						}
						// 如果命令字包含摄像机状态，则更新变量
						//orientation = pan_pos*170.0/2267.0;
						//tilt = tilt_pos*170.0/2267.0;
						orientation = pan_pos/14.4;
						tilt = tilt_pos/14.4;
						return TRUE;					
					}
				}
				iStart = i+1;
			}
		}
	}
	else
		return FALSE;
}

void CCameraDrive::Zoom(int level)
{
	BYTE command[6] = {0x81,0x01,0x04,0x07,
		0xFF,                         //BYTE[4] 前四位为2 for tele，3 for wide.后四位 for level(0-7)
		0xFF};
	int temp;
	if(level>0)
		temp = 32+level;
	else
		temp = 48-level;
	BYTE B_level = (BYTE)temp;
	command[4] =B_level;
	m_stateofCommand = "Zoom changing ";
	SendData(command, sizeof(command));

}

void CCameraDrive::Focus(int level)
{
	BYTE command[6] = {0x81,0x01,0x04,0x08,
		0xFF,                         //BYTE[4] 前四位为2 for far，3 for near.后四位 for level(0-7)
		0xFF};
	int temp;
	if(level>0)
		temp = 32+level;
	else
		temp = 48-level;
	BYTE B_level = (BYTE)temp;
	command[4] =B_level;
	m_stateofCommand = "Focus changing ";
	SendData(command, sizeof(command));
	
}
void CCameraDrive::Stop_Zoom()
{
	BYTE command[6]={0x81,0x01,0x04,0x07,0x00,0xFF};
    m_stateofCommand = "stop zooming ";
	SendData(command, sizeof(command));
}
void CCameraDrive::Stop_Focus()
{
	BYTE command[6]={0x81,0x01,0x04,0x08,0x00,0xFF};
    m_stateofCommand = "stop focusing ";
	SendData(command, sizeof(command));
}
void CCameraDrive::Stop()
{
	Stop_Zoom();
	Stop_Focus();
	Stop_PanTilt();
    m_stateofCommand = "stop all ";
}

void CCameraDrive::Home()
{
	BYTE command[5]	= {0x81, 0x01, 0x06, 0x04, 0xFF};	// Home command
	SendData(command, sizeof(command));
	m_stateofCommand = "home ";

}

void CCameraDrive::Auto()
{
	Auto_Focus();
	Auto_Zoom_wide();
	m_stateofCommand = "Auto setting";
}

void CCameraDrive::Auto_Focus()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x38, 0x02,0xFF};	// manual command
	SendData(command, sizeof(command));
	m_stateofCommand = "Auto focus ";
}

void CCameraDrive::Auto_Zoom_tele()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x07, 0x02,0xFF};	// tele(standard) command
	SendData(command, sizeof(command));
	m_stateofCommand = "zoom   tele(standard) ";
}

void CCameraDrive::Auto_Zoom_wide()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x07, 0x03,0xFF};	// wide(standard) command
	SendData(command, sizeof(command));
	m_stateofCommand = "zoom   wide(standard) ";
}

void CCameraDrive::Backlight_compensation(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x33, 0xFF,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
}

void CCameraDrive::SetFocusPos(int Pos)
{
	BYTE command[9] = {0x81, 0x01,0x04,0x48,0x00,0x00,0x00,0x00,0xFF};
	int q = Pos%10;
	int p = Pos%100;
	if(p == Pos)
		p = (Pos -q)/10;
	else{
		if(q<10)
			p = 'A';
		else{
				if(q<20)
					p = 'B';
				else
					p = 'C';
		}
	}
	command[4] = (BYTE)p;
	command[5] = (BYTE)q;
	m_stateofCommand = "Set FocusPosition ";
	SendData(command,sizeof(command));

}

void CCameraDrive::SetZoomPos(int Pos)
{
	BYTE command[9] = {0x81, 0x01,0x04,0x47,0x00,0x00,0x00,0x00,0xFF};
	int q = Pos%16;
	int p = 0;
	/*if(q == Pos)
		p = 0;
	else
		p = (Pos-q)/16;*/
	p=Pos/16%16;
	int r = Pos/16/16%16;
	command[4] = (BYTE)p;
	command[5] = (BYTE)q;
	command[6] = (BYTE)r;
	m_stateofCommand = "Set ZoomPosition ";
	SendData(command,sizeof(command));

}

void CCameraDrive::RelativeLeftTo(double posd, int speed)
{
	//int pos = (DWORD) (posd*2267/170);
	int pos = (DWORD) (posd*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x03, 
		0xFF,						// command[4] for pan-speed
		0x00, 
 		0x0F, 0x0F, 0x0F, 0x0F,		// command[6-9] for the pan position F725-08DB
		0x00, 0x00, 0x00, 0x00,     // for the tilt position FE70-04B0
		0xFF};

	command[4] = speed;				// speed

	//pos = ~pos + 1;					// 取pos的补码
	for (int i=9; i>=6; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}

	m_stateofCommand = "Relative left movement";
	SendData(command, sizeof(command));
}

void CCameraDrive::RelativeRightTo(double posd, int speed)
{
	//int pos = (DWORD) (posd*2267/170);
	int pos = (DWORD) (posd*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x03, 
		0xFF,						// command[4] for pan-speed
		0x00, 
 		0x0F, 0x0F, 0x0F, 0x0F,		// command[6-9] for the pan position
		0x00, 0x00, 0x00, 0x00,
		0xFF};

	command[4] = speed;				// speed

	for (int i=9; i>=6; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	
	m_stateofCommand = "Relative right movement";
	SendData(command, sizeof(command));
}
void CCameraDrive::RelativeUpTo(int pos, int speed)
{
	//pos = (DWORD) (pos*2267/170);
	pos = (DWORD) (pos*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x03, 
		0x00,						
		0xFF,                       // command[5] for tilt-speed 
		0x00, 0x00, 0x00, 0x00,
		0x0F, 0x0F, 0x0F, 0x0F,	     // command[10-14] for the tilt position
		0xFF};

	command[5] = speed;				// speed

	for (int i=13; i>=10; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	
	m_stateofCommand = "Relative Up movement";
	SendData(command, sizeof(command));
}
void CCameraDrive::RelativeDownTo(int pos, int speed)
{
	//pos = (DWORD) (pos*2267/170);
	pos = (DWORD) (pos*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x03, 
		0x00,						
		0xFF,                       // command[5] for tilt-speed 
		0x00, 0x00, 0x00, 0x00,
		0x0F, 0x0F, 0x0F, 0x0F,	     // command[10-14] for the tilt position
		0xFF};

	command[5] = speed;				// speed
	//pos = ~pos + 1;					// 取pos的补码
	for (int i=13; i>=10; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	
	m_stateofCommand = "Relative dwon movement";
	SendData(command, sizeof(command));
}
// 一般来说，用户只区分左右，而不区分正负，因此此处pos参数应该为绝对值
// 在函数内部将之转化为机器可识别码，注意：WORD为两个字节无符号整型
void CCameraDrive::LeftTo(int pos, int speed)
{
	//pos = (DWORD) (pos*2267/170);
	pos = (DWORD) (pos*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x02, 
		0xFF,						// command[4] for pan-speed
		0x00, 
 		0x0F, 0x0F, 0x0F, 0x0F,		// command[6-9] for the pan position
		0x00, 0x00, 0x00, 0x00,
		0xFF};

	command[4] = speed;				// speed

	//pos = ~pos + 1;					// 取pos的补码
	for (int i=9; i>=6; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	m_stateofCommand = "left to";
	SendData(command, sizeof(command));
}

// 一般来说，用户只区分左右，而不区分正负，因此此处pos参数应该为绝对值
// 在函数内部将之转化为机器可识别码，注意：WORD为两个字节无符号整型
void CCameraDrive::RightTo(int pos, int speed)
{
	//pos = (DWORD) (pos*2267/170);
	pos = (DWORD) (pos*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x02, 
		0xFF,						// command[4] for pan-speed
		0x00, 
 		0x0F, 0x0F, 0x0F, 0x0F,		// command[6-9] for the pan position
	
		0x00, 0x00, 0x00, 0x00,
		0xFF};

	command[4] = speed;				// speed

	for (int i=9; i>=6; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	m_stateofCommand = "right to";	
	SendData(command, sizeof(command));
}

void CCameraDrive::Left(int speed)
{
	speed = (BYTE) speed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0xFF,						// command[4] for pan-speed
		0x17, 
		0x01, 0x03,	0xFF};

	command[4] = speed;				// speed
	m_stateofCommand = "left turnning";	
	SendData(command, sizeof(command));
}

void CCameraDrive::Right(int speed)
{
	speed = (BYTE) speed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0xFF,						// command[4] for pan-speed
		0x00, 
		0x02, 0x03,	0xFF};

	command[4] = speed;				// speed
	m_stateofCommand = "right turnning";	
	SendData(command, sizeof(command));
}
void CCameraDrive::Up(int speed)
{
	speed = (BYTE) speed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0x00,						
		0xFF,                      // command[5] for tilt-speed
		0x03, 0x01,	0xFF};

	command[5] = speed;				// speed
	m_stateofCommand = "up  turnning";	
	SendData(command, sizeof(command));
}
void CCameraDrive::Down(int speed)
{
	speed = (BYTE) speed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0x00,						
		0xFF,                      // command[5] for tilt-speed
		0x03, 0x02,	0xFF};

	command[5] = speed;				// speed
	m_stateofCommand = "down turnning";	
	SendData(command, sizeof(command));
}
void CCameraDrive::UpLeft(int panspeed, int tiltspeed)
{
	panspeed = (DWORD) panspeed;
	tiltspeed = (BYTE) tiltspeed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0xFF,						// command[4] for pan-speed
		0xFF,						// command[5] for tilt-speed
		0x01, 0x01,	0xFF};

	command[4] = panspeed;
	command[5] = tiltspeed;
	m_stateofCommand = "UpLeft turnning";	
	SendData(command, sizeof(command));
}

void CCameraDrive::UpRight(int panspeed, int tiltspeed)
{
	panspeed = (DWORD) panspeed;
	tiltspeed = (BYTE) tiltspeed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0xFF,						// command[4] for pan-speed
		0xFF,						// command[5] for tilt-speed
		0x02, 0x01,	0xFF};

	command[4] = panspeed;
	command[5] = tiltspeed;
	m_stateofCommand = "UpRight turnning";	
	SendData(command, sizeof(command));
}

void CCameraDrive::DownLeft(int panspeed, int tiltspeed)
{
	panspeed = (DWORD) panspeed;
	tiltspeed = (BYTE) tiltspeed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0xFF,						// command[4] for pan-speed
		0xFF,						// command[5] for tilt-speed
		0x01, 0x02,	0xFF};

	command[4] = panspeed;
	command[5] = tiltspeed;
	m_stateofCommand = "DownLeft turnning";	
	SendData(command, sizeof(command));
}

void CCameraDrive::DownRight(int panspeed, int  tiltspeed)
{
	panspeed = (DWORD) panspeed;
	tiltspeed = (BYTE) tiltspeed;
	BYTE command[9] = {
		0x81, 0x01, 0x06, 0x01, 
		0xFF,						// command[4] for pan-speed
		0xFF,						// command[5] for tilt-speed
		0x02, 0x02,	0xFF};

	command[4] = panspeed;
	command[5] = tiltspeed;
	m_stateofCommand = "DownRight turnning";	
	SendData(command, sizeof(command));
}
void CCameraDrive::UpTo(int pos, int speed)
{
	//pos = (DWORD) (pos*2267/170);
	pos = (DWORD) (pos*334);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x02, 
		0x00,						
		0xFF,                       // command[5] for tilt-speed
 		0x00, 0x00, 0x00, 0x00,		
		0x0F, 0x0F, 0x0F, 0x0F,		// command[10-13] for the tilt position
		0xFF};

	command[5] = speed;				// speed

	for (int i=13; i>=10; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	m_stateofCommand = "Up to";
	SendData(command, sizeof(command));
}


void CCameraDrive::DownTo(int pos, int speed)
{
	//pos = (DWORD) (pos*2267/170);
	pos = (DWORD) (pos*334);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x02, 
		0x00,						
		0xFF,						// command[4] for tilt-speed
 		0x00, 0x00, 0x00, 0x00,		
		0x0F, 0x0F, 0x0F, 0x0F,		// command[10-13] for the tilt position
		0xFF};

	command[5] = speed;				// speed

	//pos = ~pos + 1;					// 取pos的补码
	for (int i=13; i>=10; i--)		// 给命令字赋值
	{
		command[i] &= pos;
		pos = pos >> 4;
	}
	m_stateofCommand = "down to";
	SendData(command, sizeof(command));
}

void CCameraDrive::QueryFocusPos()
{
	m_QueryFlag = 1;
	m_queryFlag_array.Add(m_QueryFlag);
	BYTE command[5] = {0x81, 0x09, 0x04, 0x48, 0xFF};
	
//	m_stateofCommand = "Inquirie FocusPos!";
	SendData(command, sizeof(command));
}

void CCameraDrive::QueryZoomPos()
{
	m_QueryFlag = 2;
	m_queryFlag_array.Add(m_QueryFlag);
	BYTE command[5] = {0x81, 0x09, 0x04, 0x47, 0xFF};
//	m_stateofCommand = "Inquirie ZoomPos!";
//	//TRACE("Inquirie FocusPos!");
	SendData(command, sizeof(command));

}

BOOL CCameraDrive::BytestoFocusPos(BYTE *command, int num, double &FocusPos)
{
	int i, j;
	int iStart	= 0;
	// 检查命令字中是否含有Focus信息，如果命令字包含Focus状态，则更新变量
	if (num >= 7)
	{
//		//TRACE("%d\n",num);
		const int length_state		= 7;
		int iFocusPos = 0;
		BYTE stateBytes[length_state];
		// 定位0xFF
		for (i=0; i<num; i++)
		{
			if (command[i] == 0xFF)
			{
				if ((i-iStart+1) == length_state)
				{
					memcpy(stateBytes, command+iStart, length_state);
					if (stateBytes[0] == 0x90 && stateBytes[1] == 0x50)
					{
						//只取16位，后面16位为0
						for(j=2; j<=3; j++)
							iFocusPos |= stateBytes[j] << (4*(3-j));
						// 更新变量
						//////////////////////////////////////////////////////////////////////////
						//非线性函数，分段考虑
						int temp = iFocusPos/16;
						////TRACE("temp =%d\n",temp);
						int temp2 = iFocusPos%16;
						switch (temp)
						{
						case 1:
							FocusPos = 100;
							break;
						case 2:
							FocusPos = 8.0-(8.0-3.5)/16*temp2;
							break;
						case 3:
							FocusPos = 3.5-+(3.5-2.0)/16*temp2;
							break;
						case 4:
							FocusPos = 2.0-(2.0-1.4)/16*temp2;
							break;
						case 5:
							FocusPos = 1.4-(1.4-1.0)/16*temp2;
							break;
						case 6:
							FocusPos = 1.0-(1.0-0.8)/16*temp2;
							break;
						case 7:
							FocusPos = 0.8-(0.8-0.29)/16*temp2;
							break;
						case 8:
							FocusPos = 0.29-(0.29-0.1)/16*temp2;
							break;
						case 9:
							FocusPos = 0.1-(0.1-0.047)/16*temp2;
							break;
						case 10:
							FocusPos = 0.047-(0.047-0.023)/16*temp2;
							break;
						case 11:
							FocusPos = 0.023-(0.023-0.01)/16*temp2;
							break;
						default:
							FocusPos = -1;
							break;
						return TRUE;
						}	
					}
				}
				iStart = i+1;
			}
		}
	}
	else
		return FALSE;

}

BOOL CCameraDrive::BytestoZoomPos(BYTE *command, int num, double &ZoomPos)
{
	int i, j;
	int iStart	= 0;
	// 检查命令字中是否含有Focus信息，如果命令字包含Focus状态，则更新变量
	if (num >= 7)
	{
		//		////TRACE("%d\n",num);
		const int length_state		= 7;
		int iZoomPos = 0;
		BYTE stateBytes[length_state];
		// 定位0xFF
		for (i=0; i<num; i++)
		{
			if (command[i] == 0xFF)
			{
				if ((i-iStart+1) == length_state)
				{
					memcpy(stateBytes, command+iStart, length_state);
					if (stateBytes[0] == 0x90 && stateBytes[1] == 0x50)
					{
						//只取16位，后面16位为0
						for(j=2; j<=5; j++)
							iZoomPos |= stateBytes[j] << (4*(5-j));
						// 更新变量
						if(iZoomPos <0x1606)
							ZoomPos = 1;
						if((iZoomPos > 0x1606)&&(iZoomPos < 0x2151))
							ZoomPos = 2;
						if((iZoomPos > 0x2151)&&(iZoomPos < 0x2860))
							ZoomPos = 3;
						if((iZoomPos > 0x2860)&&(iZoomPos < 0x2CB5))
							ZoomPos = 4;
						if((iZoomPos > 0x2CB5)&&(iZoomPos < 0x3060))
							ZoomPos = 5;
						if((iZoomPos > 0x3060)&&(iZoomPos < 0x32D3))
							ZoomPos = 6;
						if((iZoomPos > 0x32D3)&&(iZoomPos < 0x3545))
							ZoomPos = 7;
						if((iZoomPos > 0x3545)&&(iZoomPos < 0x3727))
							ZoomPos = 8;
						if((iZoomPos > 0x3727)&&(iZoomPos < 0x38A9))
							ZoomPos = 9;
						if((iZoomPos > 0x38A9)&&(iZoomPos < 0x3A42))
							ZoomPos = 10;
						if((iZoomPos > 0x3A42)&&(iZoomPos < 0x3B4B))
							ZoomPos = 11;
						if((iZoomPos > 0x3B4B)&&(iZoomPos < 0x3C85))
							ZoomPos = 12;
						if((iZoomPos > 0x3C85)&&(iZoomPos < 0x3D75))
							ZoomPos = 13;
						if((iZoomPos > 0x3D75)&&(iZoomPos < 0x3E4E))
							ZoomPos = 14;
						if((iZoomPos > 0x3E4E)&&(iZoomPos < 0x3EF7))
							ZoomPos = 15;
						if((iZoomPos > 0x3EF7)&&(iZoomPos < 0x3FA0))
							ZoomPos = 16;
						if((iZoomPos > 0x3FA0)&&(iZoomPos < 0x4000))
							ZoomPos = 17;
						if(iZoomPos >0x4000)
							ZoomPos = 18;
						////TRACE("%d\n",ZoomPos);
						ZoomPos *= 4.1;//4.1mm为一倍的焦距
						return TRUE;
					}
				}
				iStart = i+1;
			}

		}
	}
	else
		return FALSE;
}

void CCameraDrive::GetFocusPos(double &dbFocusPos)
{
	dbFocusPos = m_dbFoucsPos;
}

void CCameraDrive::GetZoomPos(double &dbZoomPos)
{
	dbZoomPos = m_dbZoomPos;
}

void CCameraDrive::SetMoveSpeed(int speed)
{	
	m_MoveSpeed = speed;
}

void CCameraDrive::Maunal_Focus()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x38, 0x03,0xFF};	// manual command
	SendData(command, sizeof(command));
	m_stateofCommand = "manual focus ";
}

void CCameraDrive::Auto_mannal_Focus()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x38, 0x10,0xFF};	// Auto/manual command
	SendData(command, sizeof(command));
	m_stateofCommand = "Auto/manual focus ";
}

void CCameraDrive::power(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x00, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = "power on/off ";

}

void CCameraDrive::DirectTo(int pan_pos, int tilt_pos, int speed)
{
	//pan_pos = (DWORD) (pan_pos*2267/170);
	//tilt_pos = (DWORD) (tilt_pos*2267/170);
	pan_pos = (DWORD) (pan_pos*14.4);
	tilt_pos = (DWORD) (tilt_pos*14.4);
	speed = (BYTE) speed;
	BYTE command[15] = {
		0x81, 0x01, 0x06, 0x02, 
		0xFF,						// command[4] for pan-speed
		0xFF,						// command[5] for tilt-speed
 		0x0F, 0x0F, 0x0F, 0x0F,		// command[6-9] for the pan position
		0x0F, 0x0F, 0x0F, 0x0F,		// command[10-13] for the tilt position
		0xFF};

	command[4] = speed;
	command[5] = speed;				// speed

	pan_pos = ~pan_pos + 1;					// 取pos的补码
	for (int i=9; i>=6; i--)		// 给命令字赋值
	{
		command[i] &= pan_pos;
		pan_pos = pan_pos >> 4;
	}

	tilt_pos = ~tilt_pos + 1;					// 取pos的补码
	for (int i=13; i>=10; i--)		// 给命令字赋值
	{
		command[i] &= tilt_pos;
		tilt_pos = tilt_pos >> 4;
	}
	m_stateofCommand = "Direct to";
	SendData(command, sizeof(command));
}

void CCameraDrive::ICR(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x01, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = "ICR(红外) on/off ";
}

void CCameraDrive::Auto_ICR(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x51, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = "AUTO_ICR(红外) on/off ";
}
//flag = 0,无效果。flag = 1,NEG.art效果。flag = 2，黑白效果
void CCameraDrive::PictureEffect(int flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x63, 0x00,0xFF};
	if(flag = 1)
		command[4] = 0x02;
	if(flag = 2)
		command[4] = 0x04;
	SendData(command, sizeof(command));
	m_stateofCommand = "Picture Effect ";

}

void CCameraDrive::Mute(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x75, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = "Mute on/off ";
}
//调节光圈大小，true为up，false为down
void CCameraDrive::ApertureChange(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x02, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = "Aperture Change up/down";

}

void CCameraDrive::ImageFreeze(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x62, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = "Still image on/off ";

}
//锁定摄像机
void CCameraDrive::lock(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x17, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x00;
	SendData(command, sizeof(command));
	m_stateofCommand = " Camera Control Enable/Disable";
}
//遥控enable/disable
void CCameraDrive::IR_Control_Lock(bool flag)
{
	BYTE command[6]	= {0x81, 0x01, 0x06, 0x08, 0x00,0xFF};
	if(flag)
		command[4] = 0x02;
	else
		command[4] = 0x03;
	SendData(command, sizeof(command));
	m_stateofCommand = " IR Control Enable/Disable";
}
//通过遥控控制摄像机后，是否通过串口返回信息
void CCameraDrive::IR_Control_ReceiveReturn(bool flag)
{
	BYTE command[8]	= {0x81, 0x01, 0x7d, 0x01, 0x00, 0x00, 0x00, 0xFF};
	if(flag)
		command[4] = 0x03;
	else
		command[4] = 0x13;
	SendData(command, sizeof(command));
	m_stateofCommand = " IR Control  messageReturn Enable/Disable";
}
//重置光圈
void CCameraDrive::ApertureReset()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x02, 0x00,0xFF};
	SendData(command, sizeof(command));
	m_stateofCommand = " Aperture Reset";
}
//自检，检查镜头性能和CCD是否溢出，移动平台是否正常
void CCameraDrive::check()
{
	BYTE command[6]	= {0x81, 0x01, 0x04, 0x19, 0x01,0xFF};
	SendData(command, sizeof(command));//check lens
	command[2]	= 0x02; 
	SendData(command, sizeof(command));//check CCD 
	BYTE command2[5] = {0x81, 0x01, 0x06, 0x05, 0xFF};
	SendData(command2, sizeof(command2));//check movement
	m_stateofCommand = " check lens,CCD blemishes，movement";
}
//取消命令
void CCameraDrive::CommandCancel()
{
	BYTE command[3]	= {0x81, 0x21, 0xFF};
	SendData(command, sizeof(command));
    
	m_stateofCommand = " Command Cancel";
}
//设置自动关机时间
void CCameraDrive::AutoPowerOff(int min)
{
	min = (DWORD) (min);
	BYTE command[9] = {
		0x81, 0x01, 0x04, 0x40,		
		0x0F, 0x0F, 0x0F, 0x0F,		// command[4-7] for the minute to power off
		0xFF};

	min = ~min + 1;					// 取pos的补码
	for (int i=7; i>=4; i--)		// 给命令字赋值
	{
		command[i] &= min;
		min = min >> 4;
	}
	SendData(command, sizeof(command));
	m_stateofCommand = "set time to power off";
}
