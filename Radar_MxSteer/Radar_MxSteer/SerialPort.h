//#pragma once
// SerialPort.h: interface for the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////
#include<afxwin.h>
#include "afxtempl.h"
#define _AFXDLL
#define WM_COMM_BREAK_DETECTED		WM_USER+1	// A break was detected on input.
#define WM_COMM_CTS_DETECTED		WM_USER+2	// The CTS (clear-to-send) signal changed state. 
#define WM_COMM_DSR_DETECTED		WM_USER+3	// The DSR (data-set-ready) signal changed state. 
#define WM_COMM_ERR_DETECTED		WM_USER+4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define WM_COMM_RING_DETECTED		WM_USER+5	// A ring indicator was detected. 
#define WM_COMM_RLSD_DETECTED		WM_USER+6	// The RLSD (receive-line-signal-detect) signal changed state. 
//#define WM_COMM_RXCHAR				WM_USER+7	// A character was received and placed in the input buffer.
#define WM_COMM_RXFLAG_DETECTED		WM_USER+8	// The event character was received and placed in the input buffer.  
#define WM_COMM_TXEMPTY_DETECTED	WM_USER+9	// The last character in the output buffer was sent.  

#define WM_COMM_RXCHAR_MX			WM_USER+101   //MX-106 Steer
#define WM_COMM_RXCHAR_RADAR		WM_USER+102   //UTM-30LX Radar

//#define PORT_NO_MX			4 //¶Ë¿ÚºÅ£ºMX-106
//#define PORT_NO_RADAR		3 //¶Ë¿ÚºÅ£ºUTM-30LX Radar
#if !defined _SERIALPORT_NUM
#define _SERIALPORT_NUM
#endif


class CSerialPort
{														 
public:
	// owner window
	CWnd*				m_pOwner;

	unsigned int m_nWriteSize; 
	void ClosePort();

	// contruction and destruction
	CSerialPort();
	virtual		~CSerialPort();

	// port initialisation
	BOOL         InitPort(CWnd* pPortOwner, UINT portnr = 2, UINT baud = 115200, char parity = 'N', UINT databits = 8, UINT stopbits = 1, DWORD CommEvents = 0x0001, UINT writebuffersize = 1024);
	HANDLE	     m_hComm;

	// start/stop comm watching
	BOOL		StartMonitoring();
	BOOL		RestartMonitoring();
	BOOL		StopMonitoring();

	DWORD		GetWriteBufferSize();
	DWORD		GetCommEvents();
	DCB			GetDCB();

	void		WriteToPort(char* string);
	void		WriteToPort(char* string,int n);
	void		WriteToPort(LPCTSTR string);
	void		WriteToPort(LPCTSTR string,int n);

protected:
	// protected memberfunctions
	void		ProcessErrorMessage(char* ErrorText);
	static UINT	CommThread(LPVOID pParam);
	static void	ReceiveChar(CSerialPort* port, COMSTAT comstat);
	static void	WriteChar(CSerialPort* port);

	// thread
	CWinThread*			m_Thread;

	// synchronisation objects
	CRITICAL_SECTION	m_csCommunicationSync;
	BOOL				m_bThreadAlive;

	// handles
	HANDLE				m_hWriteEvent;
	HANDLE				m_hShutdownEvent;

	// Event array. 
	// One element is used for each event. There are two event handles for each port.
	// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
	// There is a general shutdown when the port is closed. 
	HANDLE				m_hEventArray[3];

	// structures
	OVERLAPPED			m_ov;
	COMMTIMEOUTS		m_CommTimeouts;
	DCB					m_dcb;

	// misc
	UINT				m_nPortNr;
	char*				m_szWriteBuffer;
	DWORD				m_dwCommEvents;
	DWORD				m_nWriteBufferSize;
};
