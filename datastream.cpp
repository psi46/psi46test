// datastream.cpp

#include "datastream.h"
#include "protocol.h"


// === Data structures ======================================================

void CRocPixel::DecodeRaw()
{
	ph = (raw & 0x0f) + ((raw >> 1) & 0xf0);
	int c =    (raw >> 21) & 7;
	c = c*6 + ((raw >> 18) & 7);
	int r =    (raw >> 15) & 7;
	r = r*6 + ((raw >> 12) & 7);
	r = r*6 + ((raw >>  9) & 7);
	y = 80 - r/2;
	x = 2*c + (r&1);
}


// === CDtbSource (CSource<uint16_t>) ================================

uint16_t CDtbSource::FillBuffer()
{
	pos = 0;
	do
	{
		dtbState = tb.Daq_Read(buffer, DTB_SOURCE_BLOCK_SIZE, dtbRemainingSize);

/*		if (dtbRemainingSize < 100000)
		{
			if      (dtbRemainingSize > 1000) tb.mDelay(  1);
			else if (dtbRemainingSize >    0) tb.mDelay( 10);
			else                              tb.mDelay(100);
		}
*/
		if (buffer.size() == 0)
		{
			if (stopAtEmptyData) throw DS_empty();
			if (dtbState) throw DS_buffer_overflow();
		}

	} while (buffer.size() == 0);

/*	printf("----------------\n");
	for (unsigned int i=0; i<buffer.size(); i++)
		printf(" %4X", (unsigned int)(buffer[i]));
	printf("\n----------------\n");
*/
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
	record.Clear();

	if (GetLast() & 0x4000)	Get();
	if (!(GetLast() & 0x8000))
	{
		record.SetStartError();
		while (!(GetLast() & 0x8000)) Get();
	}

	do
	{
		if (record.GetSize() >= 40000)
		{
			record.SetOverflow();
			break;
		}
		record.Add(GetLast() & 0x0fff);

	} while ((Get() & 0xc000) == 0);
	
	if (GetLast() & 0x4000) record.Add(GetLast() & 0x0fff);
	else record.SetEndError();

/*	for (unsigned int i=0; i<record.data.size(); i++)
		printf(" %4X", (unsigned int)(record.data[i]));
	printf("\n");
*/
	return &record;
}


// === CDecoder (CDataRecord*, CRocEvent*) =====================================

CRocEvent* CRocDecoder::Read()
{
	CDataRecord *sample = Get();
	roc_event.header = 0;
	roc_event.pixel.clear();
	unsigned int n = sample->GetSize();
	if (n > 0)
	{
		if (n > 1) roc_event.pixel.reserve((n-1)/2);
		roc_event.header = (*sample)[0];
		unsigned int pos = 1;
		while (pos < n-1)
		{
			CRocPixel pix;
			pix.raw =  (*sample)[pos++] << 12;
			pix.raw += (*sample)[pos++];
			pix.DecodeRaw();
			roc_event.pixel.push_back(pix);
		}
	}

/*	printf("====== %03X ======\n", (unsigned int)(roc_event.header));
	for (unsigned int i=0; i<roc_event.pixel.size(); i++)
		printf(" %06X (%05o) [%3i, %3i, %3i]\n", 
			(unsigned int)(roc_event.pixel[i].raw), (unsigned int)(roc_event.pixel[i].raw >> 9),
			int(roc_event.pixel[i].x), int(roc_event.pixel[i].y), int(roc_event.pixel[i].ph));
	printf("\n");
*/
	return &roc_event;
}
