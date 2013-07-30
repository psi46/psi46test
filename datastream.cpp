// datastream.cpp

#include "datastream.h"
#include "protocol.h"

#ifdef _WIN32
#define QWFMT "%15I64u"  // !!! only for windows
#else
#define QWFMT "%15llu"   // !!! probably works for Linux
#endif

// === Data structures ======================================================

void CDataRecord::Print(CProtocol &f)
{

}


void CPixel::DecodeRaw()
{
	unsigned int temp = raw;
	pulseheight = (temp & 0x0f) + ((temp >> 1) & 0xf0);
	temp >>= 9;
	int c =    (temp >> 12) & 7;
	c = c*6 + ((temp >>  9) & 7);
	int r =    (temp >>  6) & 7;
	r = r*6 + ((temp >>  3) & 7);
	r = r*6 + ( temp        & 7);
	y = 80 - r/2;
	x = 2*c + (r&1);
}


// === CBinaryDTBSource (CSource<uint16_t>) ================================

uint16_t CBinaryDTBSource::FillBuffer()
{
	uint32_t data_available;
	pos = 0;
	do
	{
		tb->Daq_Read(buffer, DTB_SOURCE_BLOCK_SIZE, data_available);
		if (data_available < 100000)
		{
			if      (data_available > 1000) tb->mDelay(  5);
			else if (data_available >    0) tb->mDelay( 50);
			else                            tb->mDelay(500);

		}
	} while(buffer.size() == 0);

	return lastSample = buffer[pos++];
}


// === CBinaryFileSource (CSource<uint16_t>) ================================

uint16_t CBinaryFileSource::FillBuffer()
{
	pos = 0;
	do
	{
		buffer.resize(FILE_SOURCE_BLOCK_SIZE);
		size = fread(buffer.data(), sizeof(uint16_t), FILE_SOURCE_BLOCK_SIZE, f); 
	} while(size == 0);

	return lastSample = buffer[pos++];
}


// === CDataRecordScanner (CDataPipe<uint16_t, CRecord*>) =============

CDataRecord* CDataRecordScanner::Read()
{
	record.eventNr = currentEventNr++;
	record.data.clear();
	while (!(GetLast() & 0x8000)) Get();
	do
	{
		if (record.data.size() >= 40000) break;
		record.data.push_back(GetLast());

	} while (!(Get() & 0x8000));

	return &record;
}


// === CDecoder (CDataRecord*, CEvent*) =====================================

CRocEvent* CRocDecoder::Read()
{
	CDataRecord *sample = Get();
	roc_event.eventNr = sample->eventNr;
	roc_event.header = 0;
	roc_event.pixel.clear();
	unsigned int n = sample->data.size();
	if (n > 0)
	{
		roc_event.header = sample->data[0] & 0xfff;
		unsigned int pos = 2;
		while (pos < n-1)
		{
			CPixel pix;
			pix.raw = (sample->data[pos++] & 0xfff) << 12;
			pix.raw += sample->data[pos++] & 0xfff;
			pix.DecodeRaw();
			roc_event.pixel.push_back(pix);
		}
	}
	return &roc_event;
}
