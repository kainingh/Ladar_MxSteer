#include "data_collection.h"
#include <afxdialogex.h>
#include <iostream>
#include<fstream>
#include<vector>
#include<time.h>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include "MXSteer.h"
using namespace std;
//using namespace cv;
#define InitialPosition 900

//sendBuffInit 为初始化雷达命令的函数
void sendBuffInit(char * ms_SendBuf)
{
	int ms_bufLen = 0;
	//ms_SendBuf 为向雷达发送的命令， 现在由于主要发送采集命令，故为MD-0300-0780-01-000 分别代表命令类型， 起始位置，采集密度
    ms_SendBuf[ms_bufLen++] = 'M';//MD为三字节解码
	ms_SendBuf[ms_bufLen++] = 'D';
	ms_SendBuf[ms_bufLen++] = '0';	//Start Step (4 Bytes)
	ms_SendBuf[ms_bufLen++] = '3';	//3
	ms_SendBuf[ms_bufLen++] = '0';
	ms_SendBuf[ms_bufLen++] = '0';
	ms_SendBuf[ms_bufLen++] = '0';	//End Step (4 Bytes)
	ms_SendBuf[ms_bufLen++] = '7';	//7
	ms_SendBuf[ms_bufLen++] = '8';
	ms_SendBuf[ms_bufLen++] = '0';
	ms_SendBuf[ms_bufLen++] = '0';	//Cluster Count (2bytes) 
	ms_SendBuf[ms_bufLen++] = '1';	//
	ms_SendBuf[ms_bufLen++] = '0';	//Scan Interval (1 byte) 
	ms_SendBuf[ms_bufLen++] = '0';	//Number of Scans (2 bytes)
	ms_SendBuf[ms_bufLen++] = '0';
	//String Characters (max 16-letters)
	ms_SendBuf[ms_bufLen++] = '\n';	//LF
}

CMXSteer steering;


// initSteer 是舵机初始化函数
int initSteer()
{
	int ID = 1;
	if (!steering.Connect("COM3", 57600))  //第一个参数为舵机端口号
	{
		cout << "串口连接失败" << endl;
		return 0;
	}
	int position, speed;
	//cin >>  position
	steering.SetSpeed(1, 25);
	//舵机指令，指示运动到初始位置900
	steering.MoveToPosition(1,InitialPosition);
	
	Sleep(3000);
	//设置速度为10
	steering.SetSpeed(1, 10);
}
void moveOneStep(int currentPosition)
{
	steering.MoveToPosition(1, currentPosition );
}
//用以剔除多余数据
/*
void dataNumCorrect()
{
	ifstream in;
	in.open("valueSave.txt");
	istream_iterator<float> begin(in);
	istream_iterator<float> end;
	vector<float> save(begin, end);
	if (save.size() % 495 == 0)
		return;
	else
	{
		int reminder = save.size() % 495;
		save._Pop_back_n(reminder);
	}
	ofstream out;
	out.open("valueSaveCorrect.txt");
	for (vector<float>::iterator it = save.begin(); it != save.end(); it++)
		out << *it << " ";
}
*/

int main()
{

	char ch;
	char mr_startFlag[3];
	int ms_bufLen = 0;
	int mr_DataLen = 0;
	int ma_DataBufLen = 0;
	int ma_DataDistLen = 0;
	bool mr_bRecvFlag = false;
	bool m_SaveData_P = false;
	bool recv_Finish = false;
	char ma_DataBuf[4096];
	char mr_Data[4096];
	char START_FLAG[3];
	ofstream out;
	out.open("valueSave.txt");
	vector<long>  ma_DataDist;
	START_FLAG[0] = '9';
	START_FLAG[1] = '9';
	START_FLAG[2] = 'b';//数据采集启动校验位
	string filename = "dataSave.txt";
	ofstream dataOut;
	dataOut.open(filename);
	initSteer();
	int currentPosition = InitialPosition;
	//TCP_IP 启动过程 包括初始化WSA,Socket初始化，

	data_collection client;
	WSADATA wsadata;
	WORD word = MAKEWORD(1, 1);
	if (!client.InitWSA(word, &wsadata))
	{
		return 0;
	}
	SOCKADDR_IN serverAddr;
	client.InitSockAddr(&serverAddr);
	SOCKET clientSocket = client.InitSocket();
	int serverAddr_size = (int)(sizeof(serverAddr));
	if (!client.setConnect(&serverAddr, serverAddr_size))
	{
		return 0;
	}


	//通过建立fd句柄 用以监视接口的可读可写性
	fd_set fdSocket, receive;
	FD_ZERO(&fdSocket);
	FD_SET(clientSocket, &fdSocket);
	int count = 0;
	steering.MoveToPosition(1, 100);
	while (true)
	{
		cout << count << endl;
		count++;
		if (count >=202)
			break;
		fd_set fdRead, fdWrite;
	    fdRead = fdSocket;
	    fdWrite = fdSocket;
     	char sendBuff[100] = { 0 };
	    char recvBuff[4096];
	    //初始化雷达指令
	    sendBuffInit(sendBuff);
	    int nResult_select = select(0, NULL, &fdWrite, NULL, NULL);
	    //cout << "可写" << nResult_select << endl;
	    cout << currentPosition << endl;
	    int ret = send(clientSocket, sendBuff, (int)(strlen(sendBuff)), 0);
	    if (ret == SOCKET_ERROR)
	    {
		  cout << "send error" << WSAGetLastError() << endl;
		  //break;
	     }
	    else
	    {
		   cout << "send correct" << endl;
	    }

	    int nRecv = 1;
	    int nResult_select1 = select(0, &fdRead, NULL, NULL, NULL);
	    //if (nResult_select1)
	    //	cout << "可读" << nResult_select1 << endl;

	    struct timeval timeout;
	    timeout.tv_sec = 4;
	    timeout.tv_usec = 0;
   	    //	string wholeName;
	    vector<string> dataSave;


	    if (dataOut.fail())
		   ERROR("file open failed");
	    int count = 0;
		
		while (nRecv = recvfrom(clientSocket, recvBuff, (int)(strlen(recvBuff)), 0, (sockaddr *)&serverAddr, &serverAddr_size)) //receive data until no data come in in 4 seconds
		{

			recvBuff[nRecv] = '\0';
			if (!select(0, &fdRead, 0, 0, &timeout))
				break;
			for (int t = 0; t != nRecv; t++)
			{
				ch = recvBuff[t];
				if (mr_bRecvFlag)//一帧的接收开始
				{
					if (mr_DataLen >= 10000)
					{//接收长度超过可能的最大长度，接收的数据出现错误，抛弃当前数据
						//状态重置
						mr_startFlag[0] = 0;
						mr_startFlag[1] = 0;
						mr_startFlag[2] = 0;
						mr_bRecvFlag = false;
						mr_DataLen = 0;
						ma_DataBufLen = 0;
						ma_DataDistLen = 0;

					}
					else
					{
						//边接收边解析
						//解析时就将数据左右顺序校

						if (mr_DataLen >= 13)//非数据区域共10字节，三字节解码
						{
							ma_DataBuf[ma_DataBufLen++] = ch;//有效数据存入ma_DataBuf
							if (ch == '\r' || ch == '\n')
							{
								ma_DataBufLen -= 2;//去掉sum和LF 
							}
							if (ma_DataBufLen % 3 == 0)
							{

								//数据解码
								const int data_byte = 3;//3字节解码
								long value = 0;
								int i = ma_DataBufLen - 3;
								int j;
								for (j = 0; j < data_byte; ++j) //每3个字节获得一个距离值
								{
									value <<= 6;
									value &= ~0x3f;
									value |= ma_DataBuf[i++] - 0x30;
								}
								//ma_DataDist[ma_DataDistLen++] = value;//原数据左右颠倒
								ma_DataDist.push_back(value);

							}
						}
						////end of 边接收边解析
					}

					mr_Data[mr_DataLen++] = ch;
					//1帧接收完毕标志：连续两个换行符
					if ((ch == '\r' || ch == '\n') && (mr_Data[mr_DataLen - 2] == '\r' || mr_Data[mr_DataLen - 2] == '\n'))
					{
						//TRACE("\n");
						//FrameAnalysis();//解析
						//状态重置
						mr_startFlag[0] = 0;
						mr_startFlag[1] = 0;
						mr_startFlag[2] = 0;
						mr_bRecvFlag = false;
						recv_Finish = true;
						mr_DataLen = 0;
						ma_DataBufLen = 0;
						ma_DataDistLen = 0;
						//out << ma_DataDist.size() << endl;
						cout << ma_DataDist.size() << endl;
						for (vector<long>::iterator iter = ma_DataDist.begin(); iter != ma_DataDist.end(); iter++)
						{

							out << *iter << " ";
						}
						//	out << endl;
						ma_DataDist.clear();
						break;
					}

				}

				//一帧的接收未开始，则寻找帧开始标志 
				//接收判断'99b'or'00p'



				mr_startFlag[0] = mr_startFlag[1];
				mr_startFlag[1] = mr_startFlag[2];
				mr_startFlag[2] = ch;

				//头字节匹配	
				if ((mr_startFlag[0] == START_FLAG[0]) && (mr_startFlag[1] == START_FLAG[1]) && (mr_startFlag[2] == START_FLAG[2]))
				{
					mr_DataLen = 0;
					mr_Data[mr_DataLen++] = START_FLAG[0];
					mr_Data[mr_DataLen++] = START_FLAG[1];
					mr_Data[mr_DataLen++] = START_FLAG[2];
					mr_bRecvFlag = true;
				}


				/*  if (count >= 1000)
					 break;*/

			}
			if (recv_Finish == true)
			{
				recv_Finish = false;
				break;
			}
		}

		if (nRecv == SOCKET_ERROR)
		{
			cout << "receive failed" << endl;
			cout << WSAGetLastError() << endl;
		}

	
    }

	//dataNumCorrect();
	client.cleanWSA();
	client.closeSocket();
	dataOut.close();
	out.close();
	steering.DisConnect();
	
	
}