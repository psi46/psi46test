// usb.h

#ifndef USB_H
#define USB_H

#ifdef _WIN32
#include "windows.h"
#endif
#include "FTD2XX.h"
#include "..\rpc_io.h"


#define USBWRITEBUFFERSIZE  1024
#define USBREADBUFFERSIZE   1024


class CUSB : public CRpcIo
{
	bool isUSB_open;
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;

	unsigned int enumPos, enumCount;

	unsigned long m_posW;
	unsigned char m_bufferW[USBWRITEBUFFERSIZE];

	unsigned long m_posR, m_sizeR;
	unsigned char m_bufferR[USBREADBUFFERSIZE];

	bool FillBuffer(unsigned long minBytesToRead);

public:
	CUSB()
	{
		m_posR = m_sizeR = m_posW = 0;
		isUSB_open = false;
		ftHandle = 0; ftStatus = 0;
		enumPos = enumCount = 0;
	}
	~CUSB() { Close(); }
	int GetLastError() { return ftStatus; }
	static const char* GetErrorMsg(int error);
	bool EnumFirst(unsigned int &nDevices);
	bool EnumNext(char name[]);
	bool Open(char serialNumber[]);
	void Close();
	bool Connected() { return isUSB_open; };


	void Write(const void *buffer, unsigned int size);
	void Flush();
	void Clear();
	void Read(void *buffer, unsigned int size);
};

#endif
