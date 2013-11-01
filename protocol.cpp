/* -------------------------------------------------------------
 *
 *  file:        protocol.cpp
 *
 *  description: log file for PSI46V1 Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    24.1.2004
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "protocol.h"
#include "psi46test.h"


bool CProtocol::open(const char filename[])
{
	f = fopen(filename, "wt");
	if (f != NULL)
	{
		timestamp("OPEN");
		section("VERSION", false);
		fputs(VERSIONINFO "\n", f);
		return true;
	}
	return false;
}


bool CProtocol::append(const char filename[])
{
	f = fopen(filename, "at");
	if (f != NULL)
	{
		timestamp("OPEN");
		section("VERSION", false);
		fputs(VERSIONINFO "\n", f);
		return true;
	}
	return false;
}


void CProtocol::close()
{
	if (f == NULL) return;
	timestamp("CLOSE");
	fclose(f);
	f = NULL;
}


void CProtocol::timestamp(const char s[])
{
	if (f == NULL) return;
	time_t t;
	struct tm *dt;
	time(&t);
	dt = localtime(&t);
	fprintf(f, "[%s] %s", s, asctime(dt));
}


void CProtocol::section(const char s[], bool crlf)
{
	if (f == NULL) return;
	if (crlf) fprintf(f, "[%s]\n", s);
	else      fprintf(f, "[%s] ",   s);
}


void CProtocol::section(const char s[], const char par[])
{
	if (f == NULL) return;
	fprintf(f, "[%s] %s\n", s, par);
}


void CProtocol::puts(const char s[])
{
	if (f == NULL) return;
	fputs(s, f);
}

void CProtocol::puts(const std::string s)
{
	if (f == NULL) return;
	fputs(s.c_str(), f);
}


void CProtocol::printf(const char *fmt, ...)
{
	va_list ap;
	if (f == NULL) return;
	va_start(ap,fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
}


void CProtocol::flush()
{
	if (f == NULL) return;
	fflush(f);
}
