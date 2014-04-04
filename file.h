// file.h

#pragma once

#include <stdio.h>

#define ERROR_FILE_END_OF_FILE 0
#define ERROR_FILE_NOT_OPEN 1
#define ERROR_FILE_READ_FAILED 2


class CFileBuffer
{
	FILE *f;
	char lastChar;
	unsigned int pos, n;
	char *buffer;
	char FillBuffer();
public:
	CFileBuffer() : f(0), lastChar(0), pos(0), n(0) {}
	~CFileBuffer() { Close(); }
	void Open(const char *fileName);
	bool IsOpen() { return f != 0; }
	void Close();
	char Get() { return lastChar; }
	char GetNext();
};
