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
	
	unsigned int GetSize() { return data.size(); }
	void Add(uint16_t value) { data.push_back(value); }
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
	void DecodeRaw();
	void DecodeAna(CAnalogLevelDecoder &dec, uint16_t *x);
};


struct CRocEvent
{
	unsigned short header;
	vector<CRocPixel> pixel;
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


// === CDataRecordScanner (CDataPipe<uint16_t>, CDataRecord*) ==============

class CDataRecordScanner : public CDataPipe<uint16_t, CDataRecord*>
{
	bool nextStartDetected;
	CDataRecord record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return &record; }
public:
	CDataRecordScanner() : nextStartDetected(false) {}
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


// === CRocDigDecoder (CDataRecord*, CRocEvent*) ============================

class CRocDigDecoder : public CDataPipe<CDataRecord*, CRocEvent*>
{
	CRocEvent x;
	CRocEvent* Read();
	CRocEvent* ReadLast() { return &x; }
};


// === CRocAnaDecoder (CDataRecord*, CRocEvent*) ============================

class CRocAnaDecoder : public CDataPipe<CDataRecord*, CRocEvent*>
{
	CAnalogLevelDecoder dec;
	CRocEvent x;
	CRocEvent* Read();
	CRocEvent* ReadLast() { return &x; }
public:
	void Calibrate(int ublackLevel, int blackLevel)
	{ dec.Calibrate(ublackLevel, blackLevel); }
};


// === CRocEventPrinter (CRocEvent*, CRocEvent*) ============================

class CRocEventPrinter : public CDataPipe<CRocEvent*, CRocEvent*>
{
	FILE *f;
	CRocEvent* x;
	CRocEvent* Read();
	CRocEvent* ReadLast() { return x; }
public:
	CRocEventPrinter(const char *filename) { x = 0; f = fopen(filename, "wt"); }
	~CRocEventPrinter() { fclose(f); }
};


// === CAnalyzer (CRocEvent*, CRocEvent*) ===================================

class CAnalyzer : public CDataPipe<CRocEvent*, CRocEvent*>
{
	CRocEvent* x;
	CRocEvent* Read() { return x = Get(); };
	CRocEvent* ReadLast() { return x; }
};
