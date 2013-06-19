//
// Author: Beat Meier
//
// Class provides basic functionalities to use the USB interface
// 
// Modifications by Peter Trüb
//
//  - changed the file name to USBInterface
//  - increased the sizes of the write and read buffers
//  - increased the timeouts for reading and writing to get better stability under linux
//  - removed some code not needed under linux
//

#ifndef USB_H
#define USB_H

#include "ftd2xx.h"


#define USBWRITEBUFFERSIZE  150000
#define USBREADBUFFERSIZE   150000


class CUSB
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
	bool Write(unsigned long bytesToWrite, const void *buffer);
	bool Flush();
	bool Read(unsigned long bytesToRead, void *buffer, unsigned long &bytesRead);
	bool _Read(void *buffer, unsigned long bytesToRead)
	{
		unsigned long bytesRead;
		if (!Read(bytesToRead, (unsigned char *)buffer, bytesRead)) return false;
		return bytesRead == bytesToRead;
	}
	bool _Write(const void *buffer, unsigned long bytesToWrite)
	{ return Write(bytesToWrite, buffer); }

	bool Clear();


	// read methods

	bool Read_CHAR(char &x) { return _Read(&x, sizeof(char)); }

	bool Read_CHARS(char *x, unsigned short count)
	{ return _Read(x, count*sizeof(char)); }

	bool Read_UCHAR(unsigned char &x)	{ return _Read(&x, sizeof(char)); }

	bool Read_UCHARS(unsigned char *x, unsigned int count)
	{ return _Read(x, count*sizeof(char)); }

	bool Read_SHORT(short &x)
	{ return _Read((unsigned char *)(&x), sizeof(short)); }

	bool Read_SHORTS(short *x, unsigned short count)
  	{ return _Read(x, count*sizeof(short)); }

	bool Read_USHORT(unsigned short &x)
	{ return _Read((unsigned char *)(&x), sizeof(short)); }

	bool Read_USHORTS(unsigned short *x, unsigned short count)
  	{ return _Read(x, count*sizeof(short)); }

	bool Read_LONG(long &x)
	{ return _Read((unsigned char *)(&x), sizeof(long)); }

	bool Read_LONGS(long *x, unsigned short count)
  	{ return _Read(x, count*sizeof(long)); }

	bool Read_ULONG(unsigned long &x)
	{ return _Read((unsigned char *)(&x), sizeof(long)); }

	bool Read_ULONGS(unsigned long *x, unsigned short count)
  	{ return _Read(x, count*sizeof(long)); }

	bool Read_String(char *s, unsigned short maxlength);


	// -- write methods

	bool Write_CHAR(char x) { return _Write(&x, sizeof(char)); }

	bool Write_CHARS(const char *x, unsigned short count)
	{ return _Write(x, count*sizeof(char)); }

	bool Write_UCHAR(const unsigned char x) { return _Write(&x, sizeof(char)); }

	bool Write_UCHARS(const unsigned char *x, unsigned int count)
	{ return _Write(x, count*sizeof(char)); }

	bool Write_SHORT(const short x) { return _Write(&x, sizeof(short)); }

	bool Write_SHORTS(const short *x, unsigned short count)
  	{ return _Write(x, count*sizeof(short)); }

	bool Write_USHORT(const unsigned short x)
	{ return _Write(&x, sizeof(short)); }

	bool Write_USHORTS(const unsigned short *x, unsigned short count)
  	{ return _Write(x, count*sizeof(short)); }

	bool Write_LONG(const long x) { return _Write(&x, sizeof(long)); }

	bool Write_LONGS(const long *x, unsigned short count)
  	{ return _Write(x, count*sizeof(long)); }

	bool Write_ULONG(const unsigned long x) { return _Write(&x, sizeof(long)); }

	bool Write_ULONGS(const unsigned long *x, unsigned short count)
  	{ return _Write(x, count*sizeof(long)); }

	bool Write_String(const char *s);
};

#endif
