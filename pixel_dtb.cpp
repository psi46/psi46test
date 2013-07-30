// psi46_tb.cpp

#include "pixel_dtb.h"
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#endif

bool CTestboard::EnumNext(string &name)
{
	char s[64];
	if (!usb.EnumNext(s)) return false;
	name = s;
	return true;
}


bool CTestboard::Enum(unsigned int pos, string &name)
{
	char s[64];
	if (!usb.Enum(s, pos)) return false;
	name = s;
	return true;
}


bool CTestboard::Open(char name[], bool init)
{
	rpc_Clear();
	if (!usb.Open(name)) return false;

	if (init) Init();
	return true;
}


void CTestboard::Close()
{
//	if (usb.Connected()) Daq_Close();
	usb.Close();
	rpc_Clear();
}


void CTestboard::mDelay(uint16_t ms)
{
	Flush();
#ifdef _WIN32
	Sleep(ms);			// Windows
#else
	usleep(ms*1000);	// Linux
#endif
}
