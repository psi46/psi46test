// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>
#include <string>
#include "config.h"
#include "file.h"


#define NUMSETTING 3

class CSettings
{
	CFileBuffer f;
	static bool IsNumber(char ch) { return '0' <= ch && ch <= '9'; }
	static bool IsAlpha(char ch) { return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || (ch == '_') || (ch == '.'); }
	static bool IsAlphaNum(char ch) { return IsAlpha(ch) || IsNumber(ch); }
	static bool IsWhitespace(char ch) { return (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'); }
	void SkipWhitespace();
	void SkipComment();
	void SkipToTag();
	void ReadTag(std::string &tag);

	bool ReadBool();
	int  ReadInt(int min, int max);
	void ReadString(std::string &s);
public:
	CSettings();
	bool Read(const char filename[]);

// --- data --------------------------------------------------------------
	int  dtbId;             // force to open special board (-1 = any connected board)
	std::string scriptPath; // script path

	int  proberPort;	    // prober serial port nr (-1 = no prober)

	int rocType;            // 0 = analog ROC, 1 = digital ROC, 2 = PROC600
	bool sensor; 		    // sensor mounted

	// cable length:           5   48  prober 450 cm  bump bonder
	int deser160_clkDelay;  //  4    0    19    5       16
	int deser160_tinDelay;  //  4    4     5    6        5

	int adc_tinDelay;
	int adc_toutDelay;
	int adc_clkDelay;
	int cableLength;        // adapter cable length in mm

	int  errorRep;          // # test rep if defect chip

	std::string waferList;
	bool IsWaferList() { return waferList.length() != 0; }
};


#endif
