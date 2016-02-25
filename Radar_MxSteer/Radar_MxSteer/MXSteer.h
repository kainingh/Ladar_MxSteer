
#include "afxtempl.h"
#include "Serial_Port.h"
#include<afxwin.h>
typedef unsigned int        UINT;
#define WM_MXSteerPosition	(WM_USER+20)
#define WM_MXSteerSpeed		(WM_USER+21)
#define WM_MXSteerTEMP		(WM_USER+22)

#define STEER_SENDBUF_LEN 1024
#define STEER_RECVBUF_LEN	64
//限制ID为1的舵机角度，以624为水平0°，从-30°到+60°，即最小264，最大1344，对应[108,540]
//限制ID为1的舵机角度，从-45°到+225°，对应码盘位置[512,3584]
#define MX_MAX	0x540		
#define MX_MIN	0x108		

extern UINT PORT_NO_MX;
extern UINT PORT_NO_RADAR;
struct STEER_STATUS
{
	unsigned char	ID;		//舵机对应唯一ID
	int				speed;
	int				curPos;
	bool			led;	//LED状态，true为On，false为Off
	STEER_STATUS()
	{
		ID = 0x00;
		speed = 0;
		curPos = 0;
		led = false;
	}
};
class CMXSteer
{
public:
	/*舵机信息状态数据*/
	STEER_STATUS m_SteerRadar;	//控制几个舵机就申明几个状态信息结构体
	STEER_STATUS m_SteerCam1;
	STEER_STATUS m_SteerCam2;

private:
	/*舵机信息状态数据*/
	enum { ID_GOAL_POSITION = 1, ID_MOVING_SPEED, ID_CUR_POSITION, ID_CUR_SPEED, ID_TEMP };//查询信息反馈标志

	/*数据部分，用于串口收发和解析*/
	//串口对象
	SerialPort		m_SerialPort;
	//发送 
	char	ms_SendBuf[STEER_SENDBUF_LEN];	//数据发送缓冲区
	int				ms_bufLen;			//数据发送长度
	unsigned char	ms_ID;				//ID
	//unsigned char	ms_Length;			//Length = 参数个数+2
	unsigned char	ms_Instruction;		//Instruction
	unsigned char	ms_Param[16];		//附加信息
	//unsigned char	ms_CheckSum;		//校验和
	unsigned char	LastSendInstruction[16];//存储最后发送的指令，用于接收数据解析
	//接收
	char dataReceive[4096];
	bool			mr_bRecvFlag;		//接收标志
	int				mr_DataLen;			//接收数据长度，从ID开始算
	unsigned char	mr_Data[STEER_RECVBUF_LEN];//接收数据缓存，从ID开始存
	unsigned char	mr_startFlag[2];	//0XFF 0XFF 这两个值为0XFF的字节表示数据包开始传递标志
	unsigned char	mr_ID;				//ID
	unsigned char	mr_Length;			//Length
	unsigned char	mr_Error;			//Error
	unsigned char	mr_Param[STEER_RECVBUF_LEN - 4];//附加信息
	unsigned char	mr_CheckSum;		//校验和

public:
	CMXSteer();
	virtual ~CMXSteer();
	BOOL Connect(char* portnr, UINT baud);
	void DisConnect();		//断开设备
//	void ProMsgChar(WPARAM ch);
//	void SetLED(unsigned char ID, bool staut);	//设置LED
	void SetSpeed(unsigned char ID, int speed);	//设置速度 
	void MoveToPosition(unsigned char ID, int position);		//运动到指定位置[0,4095].
	void MoveToPosition_Deg(unsigned char ID, int position_deg);//运动到指定位置[0,360]deg.对应十进制[0,4095]，对应十六进制[0x00,0xFFF]
	void GetPosition(unsigned char ID);	//获取当前位置
	void GetSpeed(unsigned char ID);	//获取当前速度
	void GetTEMP(unsigned char ID);	//获取温度
	bool GetDataReceive();
	void DataCout();

private:
	/*状态信息设置*/
	void SetSteerStatusLED(unsigned char ID, bool led);
	void SetSteerStatusSpeed(unsigned char ID, int speed);
	void SetSteerStatusCurPos(unsigned char ID, int curPos);

	/*接口，向舵机发送指令*/
	void SendInstruction(unsigned char ID, unsigned char Instruction, unsigned char Param[], int paramLength);

	/*发送数据至串口*/
	void send(char* data, int data_len){ m_SerialPort.com_send(data, data_len); }

	/*查询舵机信息*/
	void QueryInfo_Position(unsigned char ID);	//当前位置
	void QueryInfo_Speed(unsigned char ID);	//当前速度
	void QueryInfo_TEMP(unsigned char ID);	//温度

	/*接收的串口数据帧解析*/
	void FrameAnalysis();


};
