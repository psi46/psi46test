// datastream.h

#pragma once

#include <stdint.h>
#include <list>
#include <vector>

#include "psi46test.h"
#include "datapipe.h"
#include "protocol.h"

using namespace std;


// === Binary Data Record Format ============================================

struct CDataRecord
{
	unsigned int eventNr;
	vector<uint16_t> data;
	void Print(CProtocol &f);
};


struct CPixel
{
	int raw;
	int x;
	int y;
	int pulseheight;
	void DecodeRaw();
};


struct CRocEvent
{
	unsigned long eventNr;
	unsigned short header;
	list<CPixel> pixel;
};


// === Data Sources =========================================================

// --- pixelDTB

#define DTB_SOURCE_BLOCK_SIZE 16384

class CBinaryDTBSource : public CSource<uint16_t>
{
	CTestboard *tb;
	uint16_t lastSample;

	unsigned int pos;
	vector<uint16_t> buffer;
	uint16_t FillBuffer();

	uint16_t Read() { return (pos < buffer.size()) ? lastSample = buffer[pos++] : FillBuffer(); }
	uint16_t ReadLast() { return lastSample; }
public:
	CBinaryDTBSource(CTestboard &src) : tb(&src), pos(0), lastSample(0) { buffer.reserve(DTB_SOURCE_BLOCK_SIZE); }
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
	CBinaryFileSource() : f(0), size(0), pos(0), lastSample(0) { buffer.reserve(FILE_SOURCE_BLOCK_SIZE); }
	~CBinaryFileSource() { Close(); }
	bool Open(const char *filename) { return (f = fopen(filename, "rb")) != 0; }
	void Close() { if (f) { fclose(f); f = 0; } }
};


// === CDataRecordScanner (CDataPipe<uint16_t, CRecord*>) ===================

class CDataRecordScanner : public CDataPipe<uint16_t, CDataRecord*>
{
	unsigned long currentEventNr;
	CDataRecord record;
	CDataRecord* Read();
	CDataRecord* ReadLast() { return &record; }
public:
	CDataRecordScanner() : currentEventNr(0) {}
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
