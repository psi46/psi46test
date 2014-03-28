// datastream.h

#pragma once

#include <stdint.h>
#include <list>
#include <vector>

#include "config.h"
#include "psi46test.h"
#include "datapipe.h"
#include "protocol.h"

using namespace std;


// === Error Messages =======================================================


class DS_no_dtb_access : public DataPipeException
{
public:
	DS_no_dtb_access() : DataPipeException("No DTB connection") {};
};

class DS_buffer_overflow : public DataPipeException
{
public:
	DS_buffer_overflow() : DataPipeException("Buffer overflow") {};
};

class DS_empty : public DataPipeException
{
public:
	DS_empty() : DataPipeException("Buffer empty") {};
};


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
	uint16_t operator[](int index) { return data[index]; }
};


struct CRocPixel
{
	int raw;
	int x;
	int y;
	int ph;
	void DecodeRaw();
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
	
	bool OpenRocAna(CTestboard &dtb, uint8_t tinDelay, uint8_t toutDel, uint16_t timeout,
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

class CStreamDump : public CDataPipe<uint16_t,uint16_t>
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

class CStreamErrorDump : public CDataPipe<uint16_t,uint16_t>
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


// === CRocRawDataPrinter (CDateRecord*, CDataRecord) ==============

class CRocRawDataPrinter : public CDataPipe<CDataRecord*, CDataRecord*>
{
	FILE *f;
	CDataRecord* record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return record; }
public:
	CRocRawDataPrinter(const char *filename) { f = fopen(filename, "wt"); }
	~CRocRawDataPrinter() { fclose(f); }
};


// === CDecoder (CDataRecord*, CRocEvent*) ==================================

class CRocDecoder : public CDataPipe<CDataRecord*, CRocEvent*>
{
	CRocEvent roc_event;
	CRocEvent* Read();
	CRocEvent* ReadLast() { return &roc_event; }
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
