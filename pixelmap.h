// pixelmap.h

#ifndef PIXELMAP_H
#define PIXELMAP_H

// #include "grlog.h"
#include "scanner.h"

#ifdef WAFERTESTER
#include "protocol.h"
#endif

// #include "parser.h"

struct CParser;

/*
  bits
    0..3    unmasked pixel readout count
	4..7    masked pixel readout count
	8       trimbit 0 defect (LSB)
	9       trimbit 1 defect
	10      trimbit 2 defect
	11      trimbit 3 defect (MSB)
	12      wrong column address code
	13      wrong pixel address code

*/

// #include "protocol.h"

#define ROCNUMROWS  80
#define ROCNUMCOLS  52
 

class CPixelMap
{
public:
	bool mapExist;
	bool pulseHeightExist;
	bool pulseHeight1Exist;
	bool pulseHeight2Exist;
	bool levelExist;
private:
	unsigned int map[ROCNUMCOLS][ROCNUMROWS];
	short pulseHeight[ROCNUMCOLS][ROCNUMROWS];
	short pulseHeight1[ROCNUMCOLS][ROCNUMROWS];
	short pulseHeight2[ROCNUMCOLS][ROCNUMROWS];
	unsigned char refLevel[ROCNUMCOLS][ROCNUMROWS];
	unsigned char level[ROCNUMCOLS][ROCNUMROWS][4];

	bool IsInRange(unsigned int x, unsigned int y)
	{ return x<ROCNUMCOLS && y<ROCNUMROWS; }
	bool Hex(char **s, unsigned int &value);
	bool Dec(char **s, unsigned char &value);
public:
	CPixelMap() { Init(); }
	void Init();

	// data set methods
	void SetMaskedCount(unsigned int x, unsigned int y, unsigned int count);
	void SetUnmaskedCount(unsigned int x, unsigned int y, unsigned int count);
	void SetDefectTrimBit(unsigned int x, unsigned int y,
		unsigned int bit, bool defect);
	void SetDefectColCode(unsigned int x, unsigned int y, bool defect);
	void SetDefectRowCode(unsigned int x, unsigned int y, bool defect);

	void SetPulseHeight(unsigned int x, unsigned int y, short value);
	void SetPulseHeight1(unsigned int x, unsigned int y, short value);
	void SetPulseHeight2(unsigned int x, unsigned int y, short value);

	void SetRefLevel(unsigned int x, unsigned int y, unsigned char value);
	void SetLevel(unsigned int x, unsigned int y,
		unsigned char bit, unsigned char value);

	// data get methods
	unsigned int GetMaskedCount(unsigned int x, unsigned int y);
	unsigned int GetUnmaskedCount(unsigned int x, unsigned int y);
	bool GetDefectReadoutCnts(unsigned int x, unsigned int y);
	bool GetDefectTrimBit(unsigned int x, unsigned int y, unsigned int bit);
	bool GetDefectTrimBit(unsigned int x, unsigned int y);
	bool GetDefectColCode(unsigned int x, unsigned int y);
	bool GetDefectRowCode(unsigned int x, unsigned int y);
	bool GetDefectAddrCode(unsigned int x, unsigned int y);

	short GetPulseHeight(unsigned int x, unsigned int y);
	short GetPulseHeight1(unsigned int x, unsigned int y);
	short GetPulseHeight2(unsigned int x, unsigned int y);

	unsigned char GetRefLevel(unsigned int x, unsigned int y)
	{ return refLevel[x][y]; }
	unsigned char GetLevel(unsigned int x, unsigned int y, unsigned int bit)
	{ return level[x][y][bit]; }

	void UpdateTrimDefects();

	bool IsDefect(unsigned int x, unsigned int y);
	unsigned int DefectPixelCount();

	bool ReadPixMap(CScanner &Log);
	bool ReadPulseHeight(CScanner &Log);
	bool ReadPulseHeight1(CScanner &Log);
	bool ReadPulseHeight2(CScanner &Log);

	bool ReadRefLevel(CScanner &Log);
	bool ReadLevel(CScanner &Log, unsigned int trimbit);

//	void Histo(CHistogram &h);
//	void HistoDiff(unsigned int trimbit, CHistogram &h);

#ifdef WAFERTESTER
	void Print(CProtocol &prot);
	void PrintPulseHeight(CProtocol &prot);
	void PrintPulseHeight1(CProtocol &prot);
	void PrintPulseHeight2(CProtocol &prot);
	void PrintRefLevel(CProtocol &prot);
	void PrintLevel(unsigned int trimbit, CProtocol &prot);
#endif
};


#endif

