// datastream.h

#pragma once

#include <stdint.h>
#include <list>
#include <vector>

#include "config.h"
#include "psi46test.h"
#include "datapipe.h"
#include "protocol.h"
#include "histo.h"


using namespace std;


// === Error Messages =======================================================

DATAPIPE_ERROR(DS_no_dtb_access, "No DTB connection")
DATAPIPE_ERROR(DS_buffer_overflow, "Buffer overflow")
DATAPIPE_ERROR(DS_empty, "Buffer empty")


// === Binary Data Record Format ============================================

class CDataRecord
{
	/*
		bit 0 = misaligned start
		bit 1 = no end detected
		bit 2 = overflow
	*/
	vector<uint16_t> data;
	unsigned int flags;
public:
	void SetStartError() { flags |= 1; }
	void SetEndError()   { flags |= 2; }
	void SetOverflow()   { flags |= 4; }
	void ResetStartError() { flags &= ~1; }
	void ResetEndError()   { flags &= ~2; }
	void ResetOverflow()   { flags &= ~4; }
	void Clear() { flags = 0; data.clear(); }
	bool IsStartError() { return (flags & 1) != 0; }
	bool IsEndError()   { return (flags & 2) != 0; }
	bool IsOverflow()   { return (flags & 4) != 0; }

	unsigned int recordNr;
	void Add(uint16_t value) { data.push_back(value); }
	unsigned int GetSize() { return data.size(); }
	uint16_t& operator[](int index) { return data[index]; }
};


class CAnalogLevelDecoder
{
	int level0;
	int level1;
	int levelS;

public:
	void Calibrate(int ublackLevel, int blackLevel);
	static int ExpandSign(uint16_t x) { return (x & 0x0800) ? int(x) - 4096 : int(x); }
	int Translate(uint16_t x);
	int CorrectOffset(uint16_t x) { return ExpandSign(x) - level0; }
};


struct CRocPixel
{
	// error bits:{ ph | x | y | c1 | c0 | r2 | r1 | r0 }
	int error;

	int raw;
	int x;
	int y;
	int ph;
	void DecodeRaw(); // PSI46dig
	void DecodeRawLinear(); // PROC600
	void DecodeAna(CAnalogLevelDecoder &dec, uint16_t *x); // PSI46 analog
};


struct CRocEvent
{
	/* error bits:
		0: pixel error
	*/
	unsigned int error;

	unsigned short header;
	vector<CRocPixel> pixel;
};


struct CEvent
{
	unsigned int recordNr;

	/* error bits:
		 0: pixel error
		 1: missing TBM trailer or ROC header after TBM header
		 2: missing TBM trailer before idle pattern
		 3: code error during event data transmission
		 4: frame error during event data transmission
		 5: TBM trailer error
		 6:
		 7:
		 8: T1 missing
		 9: T0 missing
		10: H1 missing
		11: H0 missing
	*/
	int error;

	/*
		ROCA  PSI46 single ROC
		ROCD  PSI46dig single ROC
		ROCX  PROC600 single ROC
		MODA  PSI46 module (analog)
		MODD  PSI46dig module
		MODX  PROC600 module
	*/
	enum DeviceType { ROCX, ROCD, ROCA, MODX, MODD, MODA } deviceType;
	unsigned short header;
	unsigned short trailer;
	vector<CRocEvent> roc;
};



// === Data Sources =========================================================

// --- PixelDTB

#define DTB_SOURCE_BLOCK_SIZE 65536

class CDtbSource : public CSource<uint16_t>
{
	bool isOpen;
	bool logging;
	unsigned int channel;
	unsigned int dtbFifoSize;
	volatile bool stopAtEmptyData;

	// --- DTB control/state
	CTestboard *tb;
	uint32_t dtbRemainingSize;
	uint8_t  dtbState;

	// --- data buffer
	uint16_t lastSample;
	unsigned int pos;
	vector<uint16_t> buffer;
	uint16_t FillBuffer();

	// --- virtual data access methods
	uint16_t Read() { return (pos < buffer.size()) ? lastSample = buffer[pos++] : FillBuffer(); }
	uint16_t ReadLast() { return lastSample; }

	bool Open(CTestboard &dtb, unsigned int dataChannel,
		bool endless, unsigned int dtbBufferSize);
public:
	CDtbSource() : isOpen(false), logging(false) {}
	~CDtbSource() { Close(); }
	
	bool OpenRocAna(CTestboard &dtb, uint8_t tinDelay, uint8_t toutDelay, uint16_t timeout,
		bool endless = true, unsigned int dtbBufferSize = 5000000);
	bool OpenRocDig(CTestboard &dtb, uint8_t deserAdjust,
		bool endless = true, unsigned int dtbBufferSize = 5000000);
	bool OpenModDig(CTestboard &dtb, unsigned int channel, bool endless = true, unsigned int dtbBufferSize = 5000000);
	bool OpenSimulator(CTestboard &dtb,
		bool endless = true, unsigned int dtbBufferSize = 5000000);

	void Close();
	void Logging(bool on) { logging = on; }
	void Enable();
	void Disable();
	void Clear() { Disable(); Enable(); }

	// --- control and status
	uint8_t  GetState() { return dtbState; }
	uint32_t GetRemainingSize() { return dtbRemainingSize; }
	void Stop() { stopAtEmptyData = true; }
};


// --- File

#define FILE_SOURCE_BLOCK_SIZE 16384

class CBinaryFileSource : public CSource<uint16_t>
{
	FILE *f;
	uint16_t lastSample;

	unsigned int size;
	unsigned int pos;
	vector<uint16_t> buffer;
	uint16_t FillBuffer();

	uint16_t Read() { return (pos < size) ? lastSample = buffer[pos++] : FillBuffer(); }
	uint16_t ReadLast() { return lastSample; }
public:
	CBinaryFileSource() : f(0), lastSample(0), size(0), pos(0) { buffer.reserve(FILE_SOURCE_BLOCK_SIZE); }
	~CBinaryFileSource() { Close(); }
	bool Open(const char *filename) { return (f = fopen(filename, "rb")) != 0; }
	void Close() { if (f) { fclose(f); f = 0; } }
};


// === CStreamDump (uint16_t, uint16_t) ==============

class CStreamDump : public CDataPipe<uint16_t>
{
	FILE *f;
	int row;
	uint16_t x;
	uint16_t Read();
	uint16_t ReadLast() { return x; }
public:
	CStreamDump(const char *filename) { row = 0; f = fopen(filename, "wt"); }
	~CStreamDump() { fclose(f); }
};


// === CStreamErrorDump (uint16_t, uint16_t) ==============

class CStreamErrorDump : public CDataPipe<uint16_t>
{
	FILE *f;	
	bool good;
	unsigned int m, n1, n2;

	uint16_t x;
	uint16_t Read();
	uint16_t ReadLast() { return x; }
public:
	CStreamErrorDump(const char *filename) { good = true; m = n1 = n2 = 0; f = fopen(filename, "wt"); }
	~CStreamErrorDump() { fclose(f); }
	unsigned int ByteCount() { return n2; }
};


// === CDataRecordScannerROCD (uint16_t, CDataRecord*) ==============

class CDataRecordScannerROC : public CDataPipe<uint16_t, CDataRecord*>
{
	unsigned int recCounter;
	bool nextStartDetected;
	CDataRecord record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return &record; }
public:
	CDataRecordScannerROC() : recCounter(0), nextStartDetected(false) {}
};


// === CDataRecordScannerMODD (uint16_t, CDataRecord*) ==============

class CDataRecordScannerMODD_old : public CDataPipe<uint16_t, CDataRecord*>
{
	unsigned int recCounter;
	bool nextStartDetected;
	CDataRecord record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return &record; }
public:
	CDataRecordScannerMODD_old() : recCounter(0), nextStartDetected(false) {}
};


class CDataRecordScannerMODD : public CDataPipe<uint16_t, CDataRecord*>
{
	unsigned int recCounter;
	bool nextStartDetected;
	CDataRecord record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return &record; }
public:
	CDataRecordScannerMODD() : recCounter(0), nextStartDetected(false) {}
};


// === CRocRawDataPrinter (CDataRecord*, CDataRecord*) ==============


class CRocRawDataPrinter : public CDataPipe<CDataRecord*, CDataRecord*>
{
	FILE *f;
	bool adc_samples;
	CDataRecord* record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return record; }
public:
	CRocRawDataPrinter(const char *filename, bool roc_ana = false)
	{ adc_samples = roc_ana, f = fopen(filename, "wt"); }
	~CRocRawDataPrinter() { fclose(f); }
};


// === CLevelHistogram (CDataRecord*, CDataRecord*) ==============

class CLevelHistogram : public CDataPipe<CDataRecord*>
{
	CHistogram h;
	CDataRecord* x;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return x; }
public:
	CLevelHistogram() : h(-500,500,1) {}
	~CLevelHistogram() {}
	void Report(CProtocol &log) { h.Print(log, 10); }
};


// === CReadBack (CDataRecord*, CDataRecord*) ====================

class CReadBack : public CDataPipe<CDataRecord*>
{
	unsigned int count;
	unsigned int shiftReg;
	unsigned int data;
	bool updated;
	bool valid;

	CDataRecord* x;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return x; }
public:
	CReadBack() : count(0), shiftReg(0), data(0), updated(false), valid(false) {}
	~CReadBack() {}
	bool IsUpdated() { return updated; }
	bool IsValid() { return valid; }
	unsigned int GetRdbData() { updated = false; return data; }
};


// === CRocAnaDecoder (CDataRecord*, CEvent*) ============================
// PSI46 analog

class CRocAnaDecoder : public CDataPipe<CDataRecord*, CEvent*>
{
	CAnalogLevelDecoder dec;
	CEvent x;
	CEvent* Read();
	CEvent* ReadLast() { return &x; }
public:
	void Calibrate(int ublackLevel, int blackLevel)
	{ dec.Calibrate(ublackLevel, blackLevel); }
};


// === CRocDigDecoder (CDataRecord*, CEvent*) ============================
// PSI46dig

class CRocDigDecoder : public CDataPipe<CDataRecord*, CEvent*>
{
	CEvent x;
	CEvent* Read();
	CEvent* ReadLast() { return &x; }
};

// === CRocDigLinearDecoder (CDataRecord*, CEvent*) ============================
// PROC600

class CRocDigLinearDecoder : public CDataPipe<CDataRecord*, CEvent*>
{
	CEvent x;
	CEvent* Read();
	CEvent* ReadLast() { return &x; }
};


// === CModDigDecoder_old (CDataRecord*, CEvent*) ============================

class CModDigDecoder_old : public CDataPipe<CDataRecord*, CEvent*>
{
	CEvent x;
	CEvent* Read();
	CEvent* ReadLast() { return &x; }
};


// === CModDigDecoder (CDataRecord*, CEvent*) ============================

class CModDigDecoder : public CDataPipe<CDataRecord*, CEvent*>
{
	CEvent x;
	CEvent* Read();
	CEvent* ReadLast() { return &x; }
};


// === CModDigLinearDecoder (CDataRecord*, CEvent*) ======================

class CModDigLinearDecoder : public CDataPipe<CDataRecord*, CEvent*>
{
	CEvent x;
	CEvent* Read();
	CEvent* ReadLast() { return &x; }
};


// === CAnalyzer (CEvent*, CEvent*) ===================================

class CAnalyzer : public CDataPipe<CEvent*>
{
protected:
	CEvent* x;
	CEvent* Read() { return x = Get(); };
	CEvent* ReadLast() { return x; }
};


// === CEventPrinter (CEvent*, CEvent*) ============================

class CEventPrinter : public CAnalyzer
{
	FILE *f;
	bool listAll;
	CEvent* Read();
public:
	CEventPrinter(const char *filename)
	{ x = 0; listAll = true; f = fopen(filename, "wt"); }
	~CEventPrinter() { fclose(f); }
	void ListOnlyErrors(bool on) { listAll = !on; }
};

// === CReadBackLogger(CEvent*, CEvent*) ============================

class CReadbackValue
{
	bool updated;
	unsigned short n;
	unsigned short shift;

	unsigned short value;
public:
	CReadbackValue() : updated(false), n(0), shift(0), value(0) {}
	void Reset() { updated = false; n = 0; shift = 0; value = 0;  }
	void Add(unsigned int v);
	bool Get(unsigned short &rdbValue);
};


class CReadbackLogger : public CAnalyzer
{
	FILE *f;
	CReadbackValue rdb[8];

	CEvent* Read();
public:
	CReadbackLogger(const char *filename) { f = fopen(filename, "wt"); }
	~CReadbackLogger() { fclose(f); }
};
