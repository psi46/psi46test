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

#define DTB_SOURCE_BLOCK_SIZE 4096

class CDtbSource : public CSource<uint16_t>
{
	volatile bool stopAtEmptyData;

	// --- DTB control/state
	CTestboard &tb;
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
public:
	CDtbSource(CTestboard &src, bool endlesStream)
		: stopAtEmptyData(endlesStream), tb(src), lastSample(0x4000), pos(0) {}

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


// === CDataRecordScanner (CDataPipe<uint16_t>, CDataRecord*>) ==============

class CDataRecordScanner : public CDataPipe<uint16_t, CDataRecord*>
{
	CDataRecord record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return &record; }
public:
	CDataRecordScanner() {}
};


// === CDecoder (CDataRecord*, CRocEvent*) ==================================

class CRocDecoder : public CDataPipe<CDataRecord*, CRocEvent*>
{
	CRocEvent roc_event;
	CRocEvent* Read();
	CRocEvent* ReadLast() { return &roc_event; }
};


// === CAnalyzer (CRocEvent*, CRocEvent*) ===================================

class CAnalyzer : public CDataPipe<CRocEvent*, CRocEvent*>
{
	CRocEvent* Read() { return Get(); };
	CRocEvent* ReadLast() { return GetLast(); }
};
