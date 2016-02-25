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
	//�˿ڳ�ʼ��
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
	if(setSpeed>=0x3ff)   setSpeed =0x3ff;   //�ٶ�ֵ���ܳ���0x3ff 400
	ms_Instruction = 0x03;	//Write data
	ms_Param[0] = 0x20;		//��Ӧ���RAM�ĵ�ַ
	ms_Param[1] = (setSpeed%256);	//Ŀ���ٶȵ��ֽ�
	ms_Param[2] = (setSpeed/256);	//Ŀ���ٶȸ��ֽ�
	SendInstruction(ID,ms_Instruction,ms_Param,3);
	SetSteerStatusSpeed(ID,setSpeed);
	Sleep(1000);
}

//�˶���ָ��λ��[0,4095].
void CMXSteer::MoveToPosition(unsigned char ID,int position)
{//[0,4095]����Ӧʮ������[0x00,0xFFF]
	unsigned int Position = position;
	if (ID==1)
	{
		if(Position>MX_MAX) Position =MX_MAX; 
		if(Position<MX_MIN) Position =MX_MIN; 
	}
	if(Position>=0xFFF) Position =0xFFF;   //λ��ֵ���ܳ���0xFFF,��Ӧ4095
	ms_Instruction = 0x03;	//Write data
	ms_Param[0] = 0x1e;		//��Ӧ���RAM�ĵ�ַ
	ms_Param[1] = (Position%256);	//Ŀ��λ�õ��ֽ�
	ms_Param[2] = (Position/256);	//Ŀ��λ�ø��ֽ�
	SendInstruction(ID,ms_Instruction,ms_Param,3);
	SetSteerStatusCurPos(ID,Position);
	Sleep(50);
}

//�˶���ָ��λ��[0,360]deg.
void CMXSteer::MoveToPosition_Deg(unsigned char ID,int position_deg)
{//[0,360]deg����Ӧʮ����[0,4095]����Ӧʮ������[0x00,0xFFF]
	unsigned int Position = (int)(position_deg/360.0*4096);	
	if(Position>=0xFFF) Position =0xFFF;   //λ��ֵ���ܳ���0xFFF,��Ӧ4095
	ms_Instruction = 0x03;	//Write data
	ms_Param[0] = 0x1e;		//��Ӧ���RAM�ĵ�ַ
	ms_Param[1] = (Position%256);	//Ŀ��λ�õ��ֽ�
	ms_Param[2] = (Position/256);	//Ŀ��λ�ø��ֽ�
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

//��ѯ��ǰλ��
void CMXSteer::QueryInfo_Position(unsigned char ID)
{
	LastSendInstruction[0] = 0x02;
	LastSendInstruction[1] = 0x24;

	ms_Instruction = 0x02;
	ms_Param[0] = 0x24;	//��Ӧ���RAM�ĵ�ַ
	ms_Param[1] = 0x02;
	SendInstruction(ID,ms_Instruction,ms_Param,2);
}
//��ѯ��ǰ�ٶ�
void CMXSteer::QueryInfo_Speed(unsigned char ID)
{
	LastSendInstruction[0] = 0x02;
	LastSendInstruction[1] = 0x26;

	ms_Instruction = 0x02;
	ms_Param[0] = 0x26;	//��Ӧ���RAM�ĵ�ַ
	ms_Param[1] = 0x02;
	SendInstruction(ID,ms_Instruction,ms_Param,2);
}
//��ѯ��ǰ�¶�
void CMXSteer::QueryInfo_TEMP(unsigned char ID)
{
	LastSendInstruction[0] = 0x02;
	LastSendInstruction[1] = 0x2B;

	ms_Instruction = 0x02;
	ms_Param[0] = 0x2B;	//��Ӧ���RAM�ĵ�ַ
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
	ms_SendBuf[ms_bufLen++]=0xff;			//������ʼ��
	ms_SendBuf[ms_bufLen++]=0x01;			//ID
	ms_SendBuf[ms_bufLen++]=length;			//LENGTH
	ms_SendBuf[ms_bufLen++]=Instruction;	//ָ�����
	for (i=0;i<paramLength;i++)
	{
		ms_SendBuf[ms_bufLen++]=Param[i];
		Check_Sum +=Param[i];
	}
	Check_Sum=~(ID+length+Instruction+Check_Sum);
	if(Check_Sum>0xff) Check_Sum=Check_Sum%256; 
	ms_SendBuf[ms_bufLen++]=Check_Sum;		//У����
	send(ms_SendBuf,ms_bufLen);
}


//���յĴ�������֡����
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
	//ChecksumУ��ͼ���
	Check_Sum = ~(mr_ID+mr_Length+mr_Error+Check_Sum);
	if(Check_Sum>0xff) Check_Sum=Check_Sum%256; 
	if (Check_Sum != mr_CheckSum)
	{
		TRACE("Check sum err!");
		AfxMessageBox(_T("Check sum err!"));
		return;
	}
	//Error ����
	//
	//
	int target_Position	= 0;	//Ŀ��λ��
	int target_Speed	= 0;	//�˶��ٶ�
	int cur_Position	= 0;	//��ǰλ��
	int cur_Speed		= 0;	//��ǰ�ٶ�
	int TEMP			= 0;	//�¶�
//	if (LastSendInstruction[0] == 0x01)//Ping
//	{
//	}
	if (LastSendInstruction[0] == 0x02)//Read data
	{
		switch(LastSendInstruction[1])
		{
		case 0X1E://Ŀ��λ�� ��
			break;
		case 0X20://�˶��ٶ� ��
			break;
		case 0X24://��ǰλ�� ��
			cur_Position = mr_Param[0] + (mr_Param[1]<<8);
		//	UpLoadData(ID_CUR_POSITION,cur_Position);
		    break;
		case 0X26://��ǰ�ٶ� ��
			cur_Speed = mr_Param[0] + (mr_Param[1]<<8);
		//	UpLoadData(ID_CUR_SPEED,cur_Speed);
		    break;
		case 0X2B://��ǰ�¶�
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