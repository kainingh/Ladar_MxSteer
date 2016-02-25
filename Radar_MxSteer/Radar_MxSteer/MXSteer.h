
#include "afxtempl.h"
#include "Serial_Port.h"
#include<afxwin.h>
typedef unsigned int        UINT;
#define WM_MXSteerPosition	(WM_USER+20)
#define WM_MXSteerSpeed		(WM_USER+21)
#define WM_MXSteerTEMP		(WM_USER+22)

#define STEER_SENDBUF_LEN 1024
#define STEER_RECVBUF_LEN	64
//����IDΪ1�Ķ���Ƕȣ���624Ϊˮƽ0�㣬��-30�㵽+60�㣬����С264�����1344����Ӧ[108,540]
//����IDΪ1�Ķ���Ƕȣ���-45�㵽+225�㣬��Ӧ����λ��[512,3584]
#define MX_MAX	0x540		
#define MX_MIN	0x108		

extern UINT PORT_NO_MX;
extern UINT PORT_NO_RADAR;
struct STEER_STATUS
{
	unsigned char	ID;		//�����ӦΨһID
	int				speed;
	int				curPos;
	bool			led;	//LED״̬��trueΪOn��falseΪOff
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
	/*�����Ϣ״̬����*/
	STEER_STATUS m_SteerRadar;	//���Ƽ����������������״̬��Ϣ�ṹ��
	STEER_STATUS m_SteerCam1;
	STEER_STATUS m_SteerCam2;

private:
	/*�����Ϣ״̬����*/
	enum { ID_GOAL_POSITION = 1, ID_MOVING_SPEED, ID_CUR_POSITION, ID_CUR_SPEED, ID_TEMP };//��ѯ��Ϣ������־

	/*���ݲ��֣����ڴ����շ��ͽ���*/
	//���ڶ���
	SerialPort		m_SerialPort;
	//���� 
	char	ms_SendBuf[STEER_SENDBUF_LEN];	//���ݷ��ͻ�����
	int				ms_bufLen;			//���ݷ��ͳ���
	unsigned char	ms_ID;				//ID
	//unsigned char	ms_Length;			//Length = ��������+2
	unsigned char	ms_Instruction;		//Instruction
	unsigned char	ms_Param[16];		//������Ϣ
	//unsigned char	ms_CheckSum;		//У���
	unsigned char	LastSendInstruction[16];//�洢����͵�ָ����ڽ������ݽ���
	//����
	char dataReceive[4096];
	bool			mr_bRecvFlag;		//���ձ�־
	int				mr_DataLen;			//�������ݳ��ȣ���ID��ʼ��
	unsigned char	mr_Data[STEER_RECVBUF_LEN];//�������ݻ��棬��ID��ʼ��
	unsigned char	mr_startFlag[2];	//0XFF 0XFF ������ֵΪ0XFF���ֽڱ�ʾ���ݰ���ʼ���ݱ�־
	unsigned char	mr_ID;				//ID
	unsigned char	mr_Length;			//Length
	unsigned char	mr_Error;			//Error
	unsigned char	mr_Param[STEER_RECVBUF_LEN - 4];//������Ϣ
	unsigned char	mr_CheckSum;		//У���

public:
	CMXSteer();
	virtual ~CMXSteer();
	BOOL Connect(char* portnr, UINT baud);
	void DisConnect();		//�Ͽ��豸
//	void ProMsgChar(WPARAM ch);
//	void SetLED(unsigned char ID, bool staut);	//����LED
	void SetSpeed(unsigned char ID, int speed);	//�����ٶ� 
	void MoveToPosition(unsigned char ID, int position);		//�˶���ָ��λ��[0,4095].
	void MoveToPosition_Deg(unsigned char ID, int position_deg);//�˶���ָ��λ��[0,360]deg.��Ӧʮ����[0,4095]����Ӧʮ������[0x00,0xFFF]
	void GetPosition(unsigned char ID);	//��ȡ��ǰλ��
	void GetSpeed(unsigned char ID);	//��ȡ��ǰ�ٶ�
	void GetTEMP(unsigned char ID);	//��ȡ�¶�
	bool GetDataReceive();
	void DataCout();

private:
	/*״̬��Ϣ����*/
	void SetSteerStatusLED(unsigned char ID, bool led);
	void SetSteerStatusSpeed(unsigned char ID, int speed);
	void SetSteerStatusCurPos(unsigned char ID, int curPos);

	/*�ӿڣ���������ָ��*/
	void SendInstruction(unsigned char ID, unsigned char Instruction, unsigned char Param[], int paramLength);

	/*��������������*/
	void send(char* data, int data_len){ m_SerialPort.com_send(data, data_len); }

	/*��ѯ�����Ϣ*/
	void QueryInfo_Position(unsigned char ID);	//��ǰλ��
	void QueryInfo_Speed(unsigned char ID);	//��ǰ�ٶ�
	void QueryInfo_TEMP(unsigned char ID);	//�¶�

	/*���յĴ�������֡����*/
	void FrameAnalysis();


};
