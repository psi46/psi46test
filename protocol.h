/* -------------------------------------------------------------
 *
 *  file:        protocol.h
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <string>


class CProtocol
{
	FILE *f, *fs;
	bool smode;
public:
	CProtocol() : f(0), fs(0), smode(false) {}
	~CProtocol() { close(); }
	bool open(const char filename[], const char *filenameS = 0);
	bool append(const char filename[], const char *filenameS = 0);
	void close();
	void timestamp(const char s[]);
	void section(const char s[], bool crlf = true);
	void section(const char s[], const char par[]);
	void puts(const char s[]);
	void puts(const std::string s);
	void printf(const char *fmt, ...);
	void flush();
	void SummaryMode(bool on)  { smode = on && fs; }
	FILE* File() { return f; }
};


#endif
