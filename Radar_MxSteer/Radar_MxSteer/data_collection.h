
#include "afxtempl.h"
#include<WinSock2.h>
#include<iostream>
#include<string>

using namespace std;

#pragma comment(lib, "WS2_32") 

/*
��ͷ��data_collection.cpp ʵ�������״�Ŀ��ƺ����ݴ���
*/

#define IPaddress "192.168.0.10" //�״�IP��ַ
#define portNumber 10940 // �״�˿ں�
class data_collection
{
private:
	SOCKET clientSocket;
public:
	bool InitWSA(WORD &wVersion, WSADATA *wsadata);  //����WSA����
	void cleanWSA();   //�ر�WSA����
	bool getLocalIP(char *IP);  
	void InitSockAddr(SOCKADDR_IN *pSockAddr, char *address = IPaddress, int netport = portNumber);    //����ȷ��Э�飬��ŵ�ַ�ķ���
	SOCKET InitSocket();   // ��ʼ��Socket����
	bool bindAddr(SOCKADDR_IN *pSockAddr);   //�󶨷���
	bool setListener(SOCKET * pSocket, int backlog);  //�����������Դ���listener�ķ���
	bool setConnect(SOCKADDR_IN *serverAddr, int address_size);  // �ͻ������Դ������ӵķ���
	void closeSocket();
};



