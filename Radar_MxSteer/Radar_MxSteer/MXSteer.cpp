// MXSteer.cpp: implementation of the CMXSteer class.
//
//////////////////////////////////////////////////////////////////////
//#include "stdafx.h"
#include "MXSteer.h"
#include<iostream>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMXSteer::CMXSteer()
{
	memset(ms_SendBuf,0,sizeof(unsigned char)*STEER_SENDBUF_LEN);
	memset(mr_Data,   0,sizeof(unsigned char)*STEER_RECVBUF_LEN); 
	//ID			= 0x01;
	ms_ID		= 0x01;
	//LedStaut	= false;
	mr_bRecvFlag = false;
	mr_startFlag[0] = 0;
	mr_startFlag[1] = 0;
	mr_DataLen = 0;
}

CMXSteer::~CMXSteer()
{
	DisConnect();
}

void CMXSteer::SetSteerStatusLED(unsigned char ID ,bool led)
{
	switch (ID)
	{
	case 0x01:
		m_SteerRadar.led = led;
		break;
	case 0x02:
		m_SteerCam1.led = led;
		break;
	case 0x03:
		m_SteerCam2.led = led;
		break;
	default:
		break;
	}
}
void CMXSteer::SetSteerStatusSpeed(unsigned char ID ,int speed)
{
	switch (ID)
	{
	case 0x01:
		m_SteerRadar.speed = speed;
		break;
	case 0x02:
		m_SteerCam1.speed = speed;
		break;
	case 0x03:
		m_SteerCam2.speed = speed;
		break;
	default:
		break;
	}
}
void CMXSteer::SetSteerStatusCurPos(unsigned char ID ,int curPos)
{
	switch (ID)
	{
	case 0x01:
		m_SteerRadar.curPos = curPos;
		break;
	case 0x02:
		m_SteerCam1.curPos = curPos;
		break;
	case 0x03:
		m_SteerCam2.curPos = curPos;
		break;
	default:
		break;
	}
}


BOOL CMXSteer::Connect( char* portnr, UINT baud)
{
	//端口初始化
	if (!m_SerialPort.com_connect(portnr, baud))
	{
		return	FALSE;
	}
}

void CMXSteer::DisConnect()
{
	m_SerialPort.com_disconnect();
}



void CMXSteer::SetSpeed(unsigned char ID,int speed)
{
	unsigned int setSpeed	 = speed;
	if(setSpeed>=0x3ff)   setSpeed =0x3ff;   //速度值不能超过0x3ff 400
	ms_Instruction = 0x03;	//Write data
	ms_Param[0] = 0x20;		//对应舵机RAM的地址
	ms_Param[1] = (setSpeed%256);	//目标速度低字节
	ms_Param[2] = (setSpeed/256);	//目标速度高字节
	SendInstruction(ID,ms_Instruction,ms_Param,3);
	SetSteerStatusSpeed(ID,setSpeed);
	Sleep(1000);
}

//运动到指定位置[0,4095].
void CMXSteer::MoveToPosition(unsigned char ID,int position)
{//[0,4095]，对应十六进制[0x00,0xFFF]
	unsigned int Position = position;
	if (ID==1)
	{
		if(Position>MX_MAX) Position =MX_MAX; 
		if(Position<MX_MIN) Position =MX_MIN; 
	}
	if(Position>=0xFFF) Position =0xFFF;   //位置值不能超过0xFFF,对应4095
	ms_Instruction = 0x03;	//Write data
	ms_Param[0] = 0x1e;		//对应舵机RAM的地址
	ms_Param[1] = (Position%256);	//目标位置低字节
	ms_Param[2] = (Position/256);	//目标位置高字节
	SendInstruction(ID,ms_Instruction,ms_Param,3);
	SetSteerStatusCurPos(ID,Position);
	Sleep(50);
}

//运动到指定位置[0,360]deg.
void CMXSteer::MoveToPosition_Deg(unsigned char ID,int position_deg)
{//[0,360]deg，对应十进制[0,4095]，对应十六进制[0x00,0xFFF]
	unsigned int Position = (int)(position_deg/360.0*4096);	
	if(Position>=0xFFF) Position =0xFFF;   //位置值不能超过0xFFF,对应4095
	ms_Instruction = 0x03;	//Write data
	ms_Param[0] = 0x1e;		//对应舵机RAM的地址
	ms_Param[1] = (Position%256);	//目标位置低字节
	ms_Param[2] = (Position/256);	//目标位置高字节
	SendInstruction(ID,ms_Instruction,ms_Param,3);
	SetSteerStatusCurPos(ID,Position);
}
void CMXSteer::GetPosition(unsigned char ID)
{
	QueryInfo_Position(ID);
}
void CMXSteer::GetSpeed(unsigned char ID)
{
	QueryInfo_Speed(ID);
}
void CMXSteer::GetTEMP(unsigned char ID)
{
	QueryInfo_TEMP(ID);
}

//查询当前位置
void CMXSteer::QueryInfo_Position(unsigned char ID)
{
	LastSendInstruction[0] = 0x02;
	LastSendInstruction[1] = 0x24;

	ms_Instruction = 0x02;
	ms_Param[0] = 0x24;	//对应舵机RAM的地址
	ms_Param[1] = 0x02;
	SendInstruction(ID,ms_Instruction,ms_Param,2);
}
//查询当前速度
void CMXSteer::QueryInfo_Speed(unsigned char ID)
{
	LastSendInstruction[0] = 0x02;
	LastSendInstruction[1] = 0x26;

	ms_Instruction = 0x02;
	ms_Param[0] = 0x26;	//对应舵机RAM的地址
	ms_Param[1] = 0x02;
	SendInstruction(ID,ms_Instruction,ms_Param,2);
}
//查询当前温度
void CMXSteer::QueryInfo_TEMP(unsigned char ID)
{
	LastSendInstruction[0] = 0x02;
	LastSendInstruction[1] = 0x2B;

	ms_Instruction = 0x02;
	ms_Param[0] = 0x2B;	//对应舵机RAM的地址
	ms_Param[1] = 0x01;
	SendInstruction(ID,ms_Instruction,ms_Param,2);
}

void CMXSteer::SendInstruction(unsigned char ID,unsigned char Instruction,unsigned char Param[],int paramLength)
{
	unsigned int Check_Sum = 0;
	unsigned int length = paramLength + 2;
	int i = 0;
	ms_bufLen=0;
	ms_SendBuf[ms_bufLen++]=0xff;
	ms_SendBuf[ms_bufLen++]=0xff;			//发送起始码
	ms_SendBuf[ms_bufLen++]=0x01;			//ID
	ms_SendBuf[ms_bufLen++]=length;			//LENGTH
	ms_SendBuf[ms_bufLen++]=Instruction;	//指令代码
	for (i=0;i<paramLength;i++)
	{
		ms_SendBuf[ms_bufLen++]=Param[i];
		Check_Sum +=Param[i];
	}
	Check_Sum=~(ID+length+Instruction+Check_Sum);
	if(Check_Sum>0xff) Check_Sum=Check_Sum%256; 
	ms_SendBuf[ms_bufLen++]=Check_Sum;		//校验码
	send(ms_SendBuf,ms_bufLen);
}


//接收的串口数据帧解析
void CMXSteer::FrameAnalysis()
{
	TRACE("\nReceiced One Frame MXSTEER Data, Run into FrameAnalysis.\n");
	unsigned int Check_Sum = 0;
	memset(mr_Param, 0, sizeof(unsigned char)*(STEER_RECVBUF_LEN-4)); 
	mr_ID		= mr_Data[0];
	mr_Length	= mr_Data[1];
	mr_Error	= mr_Data[2];
	for (int i=0; (3+i)<mr_DataLen-1; i++)
	{
		mr_Param[i] = mr_Data[3+i];
		Check_Sum += mr_Data[3+i];
	}
	mr_CheckSum = mr_Data[mr_DataLen-1];
	//Checksum校验和检验
	Check_Sum = ~(mr_ID+mr_Length+mr_Error+Check_Sum);
	if(Check_Sum>0xff) Check_Sum=Check_Sum%256; 
	if (Check_Sum != mr_CheckSum)
	{
		TRACE("Check sum err!");
		AfxMessageBox(_T("Check sum err!"));
		return;
	}
	//Error 分析
	//
	//
	int target_Position	= 0;	//目标位置
	int target_Speed	= 0;	//运动速度
	int cur_Position	= 0;	//当前位置
	int cur_Speed		= 0;	//当前速度
	int TEMP			= 0;	//温度
//	if (LastSendInstruction[0] == 0x01)//Ping
//	{
//	}
	if (LastSendInstruction[0] == 0x02)//Read data
	{
		switch(LastSendInstruction[1])
		{
		case 0X1E://目标位置 低
			break;
		case 0X20://运动速度 低
			break;
		case 0X24://当前位置 低
			cur_Position = mr_Param[0] + (mr_Param[1]<<8);
		//	UpLoadData(ID_CUR_POSITION,cur_Position);
		    break;
		case 0X26://当前速度 低
			cur_Speed = mr_Param[0] + (mr_Param[1]<<8);
		//	UpLoadData(ID_CUR_SPEED,cur_Speed);
		    break;
		case 0X2B://当前温度
			TEMP = mr_Param[0];
		//	UpLoadData(ID_TEMP,TEMP);
			break;
		default:
		    break;
		}
	}
//	if (LastSendInstruction[0] == 0x03)//Write data
//	{
//	}
	
}
bool CMXSteer::GetDataReceive()
{
	if (m_SerialPort.com_recv(dataReceive, 4096, 100))
	   return true;
	else
		return false;

}
void CMXSteer::DataCout()
{
	std::cout << dataReceive << std::endl;

}