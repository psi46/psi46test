// scanner.cpp

#include <string.h>
#include "scanner.h"


// === CFile =============================================================


bool CFile::open(const char filename[])
{
	init();
	buffer = new char[FILEBUFFERSIZE];
	if (!buffer) return false;
	m_stream = fopen(filename,"r");
	if (m_stream == NULL) return false;
	getNextChar();
	return true;
}

bool CFile::rewind()
{
	if (!m_stream) return false;
	fseek(m_stream, 0, SEEK_SET);
	getNextChar();
	return true;
}

char CFile::loadBuffer()
{
	if (!m_stream) return 0;
	count = fread(buffer,1,FILEBUFFERSIZE,m_stream);
	pos = 0;
	
	if (pos >= count) m_LastChar = 0;
	else m_LastChar = buffer[pos++];
	return m_LastChar;
}


void CFile::close()
{
	if (buffer) { delete[] buffer; buffer=NULL; }
	if (m_stream) { fclose(m_stream); m_stream = NULL; }
}



// === CScanner ==========================================================

bool CScanner::open(const char filename[])
{
	if (m_logf.open(filename)) { getNextSection(); return true; }
	return false;
}


bool CScanner::rewind()
{
	if (m_logf.rewind()) { getNextSection(); return true; }
	return false;
}


void CScanner::close()
{
	m_logf.close();
}


char* CScanner::getNextSection()
{
	m_logf.skipToChar('[');
	int pos = 0;
	char ch = m_logf.getNextChar();
	while (ch && ch!=']')
	{
		if (pos<MAXSECTIONLEN) m_Section[pos++] = ch;
		ch = m_logf.getNextChar();
	}
	m_Section[pos] = 0;
	m_logf.getNextChar();
	return m_Section;
}


bool CScanner::getNextSection(const char name[])
{
	do
	{
		getNextSection();
		if (m_Section[0] == 0) return false;
	}
	while (strcmp(m_Section, name) != 0);

	return true;
}


bool CScanner::getNextSection(const char name[], const char stop[])
{
	if (strcmp(m_Section,stop) == 0) return strcmp(name,stop) == 0;
	do
	{
		getNextSection();
		if (m_Section[0] == 0) return false;
		if (strcmp(m_Section,name) == 0) return true;
	} while(strcmp(m_Section,stop) != 0);
	return false;
}


char* CScanner::getNextLine()
{
	int pos = 0;
	char ch = m_logf.getChar();
	while (ch && ch!='[' && ch!='\r' && ch!='\n')
	{
		if (pos<MAXLINELEN) m_Line[pos++] = ch;
		ch = m_logf.getNextChar();
	}
	m_Line[pos] = 0;
	while (ch=='\r' || ch=='\n') ch = m_logf.getNextChar();
	return m_Line;
}


void CScanner::skipLine()
{
	char ch = m_logf.getChar();
	while (ch && ch!='[' && ch!='\r' && ch!='\n')
		ch = m_logf.getNextChar();
	while (ch=='\r' || ch=='\n') ch = m_logf.getNextChar();
}

