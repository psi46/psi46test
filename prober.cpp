/* -------------------------------------------------------------
 *
 *  file:        prober.cpp
 *
 *  description: connection to the Suess prober via RS232
 *
 *  author:      Beat Meier
 *  modified:    13.6.2016
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "rs232.h"
#include "prober.h"

const char defAnswer[] = " no response";

CProber::CProber()
{
	rs232 = -1;
	readback[0] = 0;
	result = 0;
	message = defAnswer;
}

bool CProber::Open (int portNr)
{
	if (IsOpen()) return false;
	rs232 = rs232_open(portNr, 9600, 'N', 8, 1, 0);
	return rs232 >= 0;
}


void CProber::Close ()
{
	if (!IsOpen()) return;
	rs232_exit(rs232);
	rs232 = -1;
	return;
}


void CProber::Clear()
{
	if (!IsOpen()) return;
	rs232_clearRx(rs232);
	readback[0] = 0;
}


void CProber::Read(int ms)
{
	int cnt;

	result = -1;
	message = defAnswer;
	
	cnt = rs232_gets(rs232, readback, sizeof(readback)-1, "\n", ms);

	if (cnt>=2) readback[cnt-2] = 0; else { readback[0] = 0; return; }

	message = strchr(readback, ':');
	if (message == 0) { message = defAnswer; result = -1; return; }
	message++;
	while (*message == ' ') message++;

	if (sscanf(readback, "%i", &result) != 1)
	{ message = defAnswer; result = -1; return; }
}


int CProber::SendCmd(const char *fmt, ...)
{
	if (!IsOpen()) return -2;

	va_list ap;
	char cmd[256];

	Clear();

	va_start(ap,fmt);
#ifdef _WIN32
	_vsnprintf(cmd, 255, fmt, ap);
#else
	vsnprintf(cmd, 255, fmt, ap);
#endif
	va_end(ap);

	rs232_puts(rs232,cmd);
	rs232_puts(rs232,"\r\n");

	Read();
	if (result != 0) PrintErrorMsg();

	return GetRsp();
}


void CProber::PrintErrorMsg()
{
	if (result < 0)
		printf(" RSP no response\n");
	else
		printf(" RSP %s\n", readback);
}
