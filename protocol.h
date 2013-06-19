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


class CProtocol
{
	FILE *f;
public:
	CProtocol() { f = NULL; }
	~CProtocol() { close(); }
	bool open(const char filename[]);
	bool append(const char filename[]);
	void close();
	void timestamp(const char s[]);
	void section(const char s[], bool crlf = true);
	void section(const char s[], const char par[]);
	void puts(const char s[]);
	void printf(const char *fmt, ...);
	void flush();
	FILE* File() { return f; }
};


#endif
