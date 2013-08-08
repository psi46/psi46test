/* -------------------------------------------------------------
 *
 *  file:        prober.cpp
 *
 *  description: connection to the Suess prober via RS232
 *
 *  author:      Beat Meier
 *  modified:    24.1.2004
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


bool CProber::open (int portNr)
{
	if (isOpen()) return false;
	rs232 = rs232_open(portNr, 9600, 'N', 8, 1, 0);
	return rs232 >= 0;
}


void CProber::close ()
{
	if (!isOpen()) return;
	rs232_exit(rs232);
	rs232 = -1;
	return;
}


void CProber::clear()
{
	if (!isOpen()) return;
	rs232_clearRx(rs232);
	readback[0] = 0;
}


char* CProber::read(int ms)
{
	int cnt;
	cnt = rs232_gets(rs232, readback, sizeof(readback)-1, "\n", ms);

	if (cnt>=2) readback[cnt-2] = 0;
	else readback[0] = 0;

	return readback;
}


char* CProber::printf(const char *fmt, ...)
{
	if (!isOpen()) return readback;

	va_list ap;
	char cmd[256];

	clear();

	va_start(ap,fmt);
#ifdef _WIN32
	_vsnprintf(cmd, 255, fmt, ap);
#else
	vsnprintf(cmd, 255, fmt, ap);
#endif
	va_end(ap);

	rs232_puts(rs232,cmd);
	rs232_puts(rs232,"\r\n");

	return read();
}

