#include"data_collection.h"



bool data_collection::InitWSA(WORD &wVersion, WSADATA *wsadata)
{
	int Ret = 0;
	if ((Ret = WSAStartup(wVersion, wsadata)) != 0)
	{
		cout << "WSAStartup failed , error" << Ret << endl;
		return false;
	}
	return true;
};

void data_collection::cleanWSA()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		cout << "WSACleanup failed, error" << WSAGetLastError() << endl;

	}
};

bool data_collection::getLocalIP(char* ip)
{

	char hostname[256];
	int ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.转化为char*并拷贝返回
	strcpy_s(ip, 20, inet_ntoa(*(in_addr*)*host->h_addr_list));
	cout << "the local IP address is:" << ip << endl;
	return true;
};

void data_collection::InitSockAddr(SOCKADDR_IN *pSockAddr , char* address , int port)
{
	pSockAddr->sin_family = AF_INET;
	pSockAddr->sin_port = htons(port);
	pSockAddr->sin_addr.s_addr = inet_addr(address);
};

SOCKET data_collection::InitSocket()
{
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "socket creat failed" << WSAGetLastError() << endl;
	}
	return clientSocket;
}

bool data_collection::bindAddr(SOCKADDR_IN *pSockAddr)
{

	int bindResult = bind(clientSocket, (sockaddr *)(pSockAddr), sizeof(*pSockAddr));



	if (bindResult == SOCKET_ERROR)

	{
		cout << "bind error :" << WSAGetLastError() << endl;

		return false;

	}

	return true;

};

bool data_collection::setListener(SOCKET *pSocket, int backlog)
{
	int nResult = listen(*pSocket, backlog);
	if (nResult == SOCKET_ERROR)
	{
		cout << "listen error:" << WSAGetLastError() << endl;
		return false;
	}
	return true;
};

bool data_collection::setConnect(SOCKADDR_IN *serverAddr, int address_size)
{
	int result = connect(clientSocket, (sockaddr *)serverAddr, address_size);
	if (result == SOCKET_ERROR)
	{
		cout << "connect error:" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}

void data_collection::closeSocket()
{
	closesocket(clientSocket);
}