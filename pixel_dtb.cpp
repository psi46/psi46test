// psi46_tb.cpp

#include "pixel_dtb.h"
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#endif


bool CTestboard::Open(char name[], bool init)
{
	if (!usb.Open(name)) return false;

	if (init) Init();
	return true;
}


void CTestboard::Close()
{
	usb.Close();
}


void CTestboard::mDelay(unsigned short ms)
{
	Flush();
#ifdef _WIN32
	Sleep(ms);			// Windows
#else
	usleep(ms*1000);	// Linux
#endif
}
