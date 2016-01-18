// settings.cpp

#include "settings.h"
#include <stdio.h>


CSettings::CSettings()
{
	dtbId = -1;
	scriptPath = "script";

	proberPort = -1;

	rocType = 1;
	sensor = false;

	// cable length:
	deser160_clkDelay = 4;
	deser160_tinDelay = 4;

	adc_tinDelay = 14;
	adc_toutDelay = 10;
	adc_clkDelay = 16;
	cableLength = 50; 

	errorRep = 1;
}

void CSettings::SkipWhitespace()
{
	while (IsWhitespace(f.Get())) f.GetNext();
}


void CSettings::SkipComment()
{
	char ch = f.GetNext();
	if (ch == '*')
	{ // skip to string end
		do { while (f.GetNext() != '*'); } while(f.GetNext() != '/');
		f.GetNext();
	}
	else if (ch == '/')
	{ // skip to line end
		f.GetNext();
		while (f.Get() != '\n' && f.Get() != '\r') f.GetNext(); 
	}
}


void CSettings::SkipToTag()
{
	while (f.Get() != '[')
	{
		if (f.Get() == '/') SkipComment();
		else f.GetNext();
	}
	f.GetNext();
}


void CSettings::ReadTag(std::string &tag)
{
	tag.clear();
	SkipToTag();
	for (int i=0; i<64; i++)
	{
		if (IsAlphaNum(f.Get())) tag.push_back(f.Get());
		else if (f.Get() == ']') { f.GetNext();  return; }
		else throw int(101);
		f.GetNext();
	}
	tag.clear();
	throw int(102);
}


bool CSettings::ReadBool()
{
	std::string s;
	ReadString(s);
	if (s == "true") return true;
	if (s == "false") return false;
	throw int(103);
	return false;
}


int  CSettings::ReadInt(int min, int max)
{
	int x;
	bool negative = false;
	
	SkipWhitespace();

	if (f.Get() == '+') f.GetNext();
	else if (f.Get() == '-') { negative = true; f.GetNext(); }

	if (IsNumber(f.Get())) { x = f.Get() - '0'; f.GetNext(); }
	else throw int(104);

	while (IsNumber(f.Get())) { x = x*10 + f.Get() - '0'; f.GetNext(); }
	if (negative) x = -x;

	if (x < min || x > max) throw int(105);
	return x;
}


void CSettings::ReadString(std::string &s)
{
	s.clear();
	int i = 0;
	SkipWhitespace();
	while (IsAlphaNum(f.Get()))
	{
		s.push_back(f.Get());
		i++;
		if (i > 256) throw int(106);
		f.GetNext();
	}
}




bool CSettings::Read(const char filename[])
{
	bool ok = true;
	try
	{
		f.Open(filename);
		while (true)
		{
			std::string s;
			ReadTag(s);
			if      (s == "DTB_ID")             dtbId = ReadInt(-1, 1000);
			else if (s == "SCRIPT_PATH")        ReadString(scriptPath);
			else if (s == "PROBER_PORT")        proberPort = ReadInt(-1, 99);
			else if (s == "ROC_TYPE")           rocType = ReadInt(0, 2);
			else if (s == "SENSOR")             sensor = ReadBool();
			else if (s == "TESTREP")            errorRep = ReadInt(0, 10);
			else if (s == "CABLE_LENGTH")       cableLength = ReadInt(0, 10000);
			else if (s == "DESER160_CLK_DELAY") deser160_clkDelay = ReadInt(0, 63);
			else if (s == "DESER160_TIN_DELAY") deser160_tinDelay = ReadInt(0, 63);
			else if (s == "ADC_TIN_DELAY")      adc_tinDelay  = ReadInt(0, 63);
			else if (s == "ADC_TOUT_DELAY")     adc_toutDelay = ReadInt(0, 63);
			else if (s == "ADC_CLK_DELAY")      adc_clkDelay = ReadInt(0, 320);
			else if (s == "WAFERLIST")          ReadString(waferList);
		}
	}
	catch (int e)
	{
		if (e != ERROR_FILE_END_OF_FILE) ok = false;
	}

	return ok;
}
