// file.cpp

#include "file.h"


#define FILE_BUFFER_SIZE 1024


void CFileBuffer::Open(const char *fileName)
{
	Close();
	f = fopen(fileName, "rt");
	if (f == 0) throw int(ERROR_FILE_NOT_OPEN);
	buffer = new char[FILE_BUFFER_SIZE];
	pos = n = 0;
	lastChar = ' ';
}


void CFileBuffer::Close()
{
	if (IsOpen())
	{
		fclose(f);
		f = 0;
		delete[] buffer;
		lastChar = 0;
	}
}


char CFileBuffer::FillBuffer()
{
	if (!IsOpen()) throw int(ERROR_FILE_NOT_OPEN);
	n = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, f); 
	pos = 0;
	if (n == 0)
	{
		if (feof(f)) throw int(ERROR_FILE_END_OF_FILE);
		else throw int(ERROR_FILE_READ_FAILED);
	}
	return buffer[pos++];
}


char CFileBuffer::GetNext()
{
	lastChar = (pos < n) ? buffer[pos++] : FillBuffer();
	return lastChar;
}
