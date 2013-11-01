// rs232.cpp
// dummy


#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>


int rs232_open(int portNr, int baud, char parity,
	int data_bit, int stop_bit, int flow_control)
{
	if (portNr < 0 && 99 < portNr) return -1;

	HANDLE hComm;
	char comport[8];
	sprintf(comport,"COM%i",portNr);
	hComm = CreateFile(comport, GENERIC_READ | GENERIC_WRITE,
				0,					// exclusive-access
				NULL,				// no security attributes
				OPEN_EXISTING,		// comm devices must use OPEN_EXISTING
				0,					// not overlapped I/O
				NULL);	// hTemplate must be NULL for comm devices
	if (hComm == INVALID_HANDLE_VALUE) return -1;

	DCB commDCB;
	if (!GetCommState(hComm,&commDCB))
	{ CloseHandle(hComm); return -1; }

	const struct { int speed, code; } baud_table[] =
	{
		{  300,CBR_300  },
		{  600,CBR_600  },
		{ 1200,CBR_1200 },
		{ 2400,CBR_2400 },
		{ 4800,CBR_4800 },
		{ 9600,CBR_9600 },
		{19200,CBR_19200},
		{38400,CBR_38400},
		{0,0}
	};
	int i=0;
	int baudrate = CBR_9600;
	while (baud_table[i].speed)
	{
		if (baud_table[i].speed == baud) { baudrate = baud_table[i].code; break; }
		i++;
	}

	commDCB.BaudRate = baudrate;	// current baud rate
	commDCB.fBinary = true;			// binary mode, no EOF check
	commDCB.fParity = false;		// enable parity checking
	commDCB.fOutxCtsFlow = false;	// CTS output flow control
	commDCB.fOutxDsrFlow = false;	// DSR output flow control
	commDCB.fDtrControl = DTR_CONTROL_DISABLE;	// DTR flow control type
	commDCB.fDsrSensitivity = false;			// DSR sensitivity
	commDCB.fTXContinueOnXoff = false;			// XOFF continues Tx
	commDCB.fOutX = false;			// XON/XOFF out flow control
	commDCB.fInX = false;			// XON/XOFF in flow control
	commDCB.fErrorChar = false;		// enable error replacement
	commDCB.fNull = false;			// enable null stripping
	commDCB.fRtsControl = RTS_CONTROL_DISABLE;	// RTS flow control
	commDCB.fAbortOnError = false;	// abort on error
	commDCB.XonLim = 0;				// transmit XON threshold
	commDCB.XoffLim = 0;			// transmit XOFF threshold
	commDCB.ByteSize = 8;			// number of bits/byte, 4-8
	commDCB.Parity = NOPARITY;		// 0-4=no,odd,even,mark,space
	commDCB.StopBits = ONESTOPBIT;	// 0,1,2 = 1, 1.5, 2

	if (!SetCommState(hComm,&commDCB))
	{
		DWORD le = GetLastError();
		CloseHandle(hComm);
		return -1;
	}

	SetupComm(hComm,600,600);

	COMMTIMEOUTS timeout;
	timeout.ReadIntervalTimeout         = 100; // ms
	timeout.ReadTotalTimeoutMultiplier  = 0;
	timeout.ReadTotalTimeoutConstant    = 100;
	timeout.WriteTotalTimeoutMultiplier = 0;
	timeout.WriteTotalTimeoutConstant   = 0;
	if (!SetCommTimeouts(hComm,&timeout))
	{ CloseHandle(hComm); return -1; }

	return (int)hComm;
}


int rs232_exit(int fd)
{
	CloseHandle((HANDLE)fd);
	return 0;
}


int rs232_write(int fd, const char *data, int size)
{
	if (fd == -1) return 0;
	unsigned long bytesWritten;
	WriteFile((HANDLE)fd, data, size, &bytesWritten, NULL);

	return (int)bytesWritten;
}


int rs232_puts(int fd, const char *str)
{
	return rs232_write(fd,str,strlen(str));
}


int rs232_gets(int fd, char *str, int size, const char *pattern, int timeout)
{
	int l;
	unsigned long i;
	if (fd == -1) return 0;

	struct _timeb start, now;
	double fstart, fnow;

	_ftime(&start);
	fstart = start.time+start.millitm/1000.0;


	memset(str, 0, size);
	for (l=0; l<size-1;)
	{
		ReadFile((HANDLE)fd, str+l, 1, &i, NULL);
		if (i>0)
		{
			l += i;
			if (pattern && pattern[0])
				if (strstr(str, pattern) != NULL) break;
		}

		_ftime(&now);
		fnow = now.time+now.millitm/1000.0;

		if ((fnow - fstart) >= timeout/1000.0)
		{
			if (pattern && pattern[0]) return 0;
			break;
		}
	}
	return l;
}


void rs232_clearRx(int fd)
{
	if (fd == -1) return;
	PurgeComm((HANDLE)fd, PURGE_RXABORT|PURGE_RXCLEAR);
}


void rs232_clearTx(int fd)
{
	if (fd == -1) return;
	PurgeComm((HANDLE)fd, PURGE_TXABORT|PURGE_TXCLEAR);
}


void rs232_clear(int fd)
{
	if (fd == -1) return;
	PurgeComm((HANDLE)fd, PURGE_TXABORT|PURGE_RXABORT|
				  PURGE_TXCLEAR|PURGE_RXCLEAR);
}


void rs232_setStatus(int fd, bool rts, bool cts)
{
	if (fd == -1) return;
	DCB commDCB;
	if (!GetCommState((HANDLE)fd,&commDCB)) return;
	commDCB.fRtsControl = (rts ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE);
	commDCB.fDtrControl = (cts ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE);
	SetCommState((HANDLE)fd,&commDCB);
}

