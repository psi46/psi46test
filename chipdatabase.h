// chipdatabase.h

#ifndef CHIPDATABASE_H
#define CHIPDATABASE_H

#include <string.h>
#include "config.h"
#include "error.h"
#include "pixelmap.h"



// --- parser ------------------------------------------------------------

// --- double columns test results ---------------------------------------
//  B0000'0000 :  i.O.
//  Bxxxx'xxx1 :  keine Reaktion
//  Bxxxx'xx1x :  WBC-Fehler
//  Bxxxx'x1xx :  Fehler im Timestamp Buffer
//  Bxxxx'1xxx :  Fehler im Databuffer
//  Bxxx1'xxxx :  kein funktionierender Pixel gefunden
#define DCOL_DEAD    0x01
#define DCOL_WBCERR  0x02
#define DCOL_TSERR   0x04
#define DCOL_DBERR   0x08
#define DCOL_NOPIX   0x10

class CDcol
{
	bool exist;
	int dcol[26]; // 0=ok, 0x01=wbc-, 0x02=ts-, 0x04=db-error
public:
	void Init() { exist = false; }
	bool IsValid() { return exist; }
	void SetValid(bool set) { exist = set; }
	bool read(CScanner &Log);
	int get(int col) { return dcol[col]; }
	void set(int col, int value) { dcol[col] = value; }
};


struct CAnalogLevel
{
	bool  exist;
	int    n;
	double mean;
	double stdev;
	int    min;
	int    max;
	void Init() { exist = false; }
	bool Read(CScanner &Log);
	void Save(const char name[], FILE *f);
};


struct CProbeCardData
{
	bool  isValid;
	double vd_cap;
	double vd_reg;
	double v_dac;
	double v_tout;
	double v_aout;
	void Init() { isValid = false; }
};


struct CLogFile;
class CWaferDataBase;


class CChip
{
protected:
	CChip *prev, *next;
	int multi;
	static const char monthNames[12][4];
	bool ConvertDate(char *xmlDate);
public:
	typedef enum
	{
		FAIL5_BIN1   =  0,
		FAIL5_BIN2   =  1,
		FAIL5_BIN3   =  2,
		FAIL5_BIN4   =  3,
		FAIL5_DEF    =  4,
		FAIL4_COL    =  5,
		FAIL4_PIXNUM =  6,
		FAIL4_MASK   =  7,
		FAIL4_ADDR   =  8,
		FAIL4_IDON   =  9,
		FAIL4_IAON   = 10,
		FAIL4_IDINIT = 11,
		FAIL4_IAINIT = 12,
		FAIL3_1PC    = 13,
		FAIL3_TMEAN  = 14,
		FAIL3_TSTD   = 15,
		FAIL3_TDIFF  = 16,
		FAIL3_TCOL   = 17, // new
		FAIL3_PHOFFS = 18, // new
		FAIL3_PHGAIN = 19, // new
		FAIL3_ASTEP  = 20,
		FAIL3_BL     = 21,
		FAIL2_1PM    = 22,
		FAIL_NOFAIL  = 23
	} TFailCode;

	int nEntry; // 0=no entry found
//	[WAFER]
	char productId[40];
	char waferId[40];
	char waferNr[10];
//	[CHIP] chip on wafer
	int mapX, mapY, mapPos;
//	[CHIP1] single chip
	char chipId[42];
//	[BEGIN]
	char startTime[28];
//	[END]
	char endTime[28];
//	[FREQ]
	int frequency;  // (<0 = not existing)
//	[PON]
	double IdigOn, IanaOn; // mA (<0 = not existing)
//	[INIT]
	double IdigInit, IanaInit; // mA (<0 = not existing)
//	[VDCAP][VDREG][VDAC][VTOUT][VAOUT]
	CProbeCardData probecard;
//	[VANA]
	double Iana[5]; // mA (Iana[0]<0 = not existing)
//	[ITRIM]
	int InitVana;  // -1 = not existing
	double InitIana;
//	[CALDELSCAN]
	int InitCalDel;
//	[AOUT]
	CAnalogLevel ublack, black, l[6];
	int token; // <0 = not existing
	int i2c; // <0=not existing, 0=error, 1=ok
	int bin;
	int pixtest2; // 256*nFailed * nPassed
	//	[PIXMAP][PUCn]
	CPixelMap pixmap;
//	[DCOL]
	CDcol dcol;
//  [CLASS]
	int logChipClass; // <0=not existing

// --- abgeleitete Groessen ----------------------------------------------
	void SetAoutOffset(double offset);

	void Calculate();
	void Pic();

	int chipClass;
	int pickClass;
	TFailCode failCode;
	int picX;
	int picY;
	int pickGroup;

	double addressStep; // < 0 if not exist
	int n;
	double pm;
	double pm_col_max;
	double pstd;
	int pmin;
	int pmax;

	int nPh;
	double ph1mean, ph21mean;
	double ph1std, ph21std;
	int nPhFail;

	int nPixDefect;
	int nPixNoSignal;
	int nPixNoisy;
	int nPixUnmaskable;
	int nPixAddrDefect;
	int nPixNoTrim;
	int nColDefect;
	int nPixThrOr;

// -----------------------------------------------------------------------

	CChip() { Invalidate(); }
	void Invalidate();
	void Save(FILE *f);
protected:
	bool Read(CScanner &Log);
private:
	void getCurrent(CScanner &Log, double *Idig,double *Iana);
	void PrintFailString(FILE *f);
	friend class CScanner;
	friend struct CLogFile;
	friend class CWaferDataBase;
};


class CWaferDataBase
{
	CChip *first;
	CChip *last;
	void Swap(CChip *entry);
	bool WriteXML_File(char path[], CChip &chip);
public:
	double aoutOffset;

	CWaferDataBase() { first = last = NULL; aoutOffset = 0; }
	~CWaferDataBase() { DeleteAll(); }

	CChip* GetFirstM() { return first; }
	static CChip* GetPrevM(CChip *chip) { return chip ? chip->prev : NULL; }
	static CChip* GetNextM(CChip *chip) { return chip ? chip->next : NULL; }

	CChip* GetFirst();
	static CChip* GetPrev(CChip *chip);
	static CChip* GetNext(CChip *chip);

	bool Add(CChip *chip);
	void DeleteAll();

	double CorrectAoutOffset();
	void Calculate();
	void SortPicOrder();
	void CalculateMulti();
	void SetPicGroups();

	bool GeneratePickFile(char filename[]);
	bool GenerateXML(char path[]);
	bool GenerateErrorReport(char filename[]);
	bool GenerateDataTable(char filename[]);
	bool GenerateStatistics(const char filename[]);
	bool GenerateWaferMap(char filename[], unsigned int mode);
};


struct CLogFile
{
	CScanner Log;
	char logTime[28];
	char logVersion[44];
	char productId[40];
	char waferId[40];
	char waferNr[10];

	void Init();
	bool readHeader();
public:
	CLogFile() { Init(); logTime[0]=logVersion[0]=0; }
	bool open(char logFilename[]);
	bool rewind();
	void close() { Log.close(); }
	bool readChip(CChip &chip);
	bool checkFileEnd();
};


#endif
