// scanner.h

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <string.h>
#include "config.h"

	
// --- CFile -------------------------------------------------------------

#define FILEBUFFERSIZE 4096

class CFile
{
	int pos, count;
	char *buffer;
	char m_LastChar;
	FILE *m_stream;
	char loadBuffer();
	void init() { m_stream = NULL; m_LastChar = 0; buffer = NULL;
					pos = 0; count = 0; }
public:
	CFile() { init(); }
	~CFile() { close(); }
	bool open(const char filename[]);
	bool rewind();
	char getNextChar()
	{
		if (pos < count) return m_LastChar = buffer[pos++];
		return loadBuffer();
	}
	char getChar() { return m_LastChar; }
	char skipToChar(char ch)
	{
		while (m_LastChar && m_LastChar != ch) getNextChar();
		return m_LastChar;
	};
	void close();
};


// --- Scanner -----------------------------------------------------------


#define MAXSECTIONLEN  20
#define MAXLINELEN    300


class CScanner
{
	CFile m_logf;
	char m_Section[MAXSECTIONLEN+1];
	char m_Line[MAXLINELEN+1];
public:
	bool open(const char filename[]);
	bool rewind();
	void close();
	~CScanner() { close(); }
	char* getNextSection();
	bool getNextSection(const char name[]);
	bool getNextSection(const char name[], const char stop[]);
	bool isSection(const char name[]) { return strcmp(m_Section,name) == 0; }

	char* getNextLine();
	char* getLine() { return m_Line; }
	void skipLine();
};


#endif
