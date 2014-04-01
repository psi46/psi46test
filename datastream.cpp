// datastream.cpp

#include "datastream.h"
#include "protocol.h"


// === Data structures ======================================================

// error bits:{ ph | x | y | c1 | c0 | r2 | r1 | r0 }

void CRocPixel::DecodeRaw()
{ PROFILING
	ph = (raw & 0x0f) + ((raw >> 1) & 0xf0);
	error = (raw & 0x10) ? 128 : 0;

	int c1 = (raw >> 21) & 7;  if (c1>=6) error |= 16;
	int c0 = (raw >> 18) & 7;  if (c0>=6) error |= 8;
	int c  = c1*6 + c0;

	int r2 = (raw >> 15) & 7;  if (r2>=6) error |= 4;
	int r1 = (raw >> 12) & 7;  if (r1>=6) error |= 2;
	int r0 = (raw >>  9) & 7;  if (r0>=6) error |= 1;
	int r  = (r2*6 + r1)*6 + r0;

	y = 80 - r/2;    if ((unsigned int)y >= 80) error |= 32;
	x = 2*c + (r&1); if ((unsigned int)x >= 52) error |= 64;
}


void CRocPixel::DecodeAna(CAnalogLevelDecoder &dec, uint16_t *v)
{ PROFILING
	error = 0;

	int c1 = dec.Translate(v[0]);  if (c1>=6) error |= 16;
	int c0 = dec.Translate(v[1]);  if (c0>=6) error |= 8;
	int c  = c1*6 + c0;

	int r2 = dec.Translate(v[2]);  if (r2>=6) error |= 4;
	int r1 = dec.Translate(v[3]);  if (r1>=6) error |= 2;
	int r0 = dec.Translate(v[4]);  if (r0>=6) error |= 1;
	int r  = (r2*6 + r1)*6 + r0;

	ph = dec.CorrectOffset(v[5]);

	y = 80 - r/2;    if ((unsigned int)y >= 80) error |= 32;
	x = 2*c + (r&1); if ((unsigned int)x >= 52) error |= 64;
}


void CAnalogLevelDecoder::Calibrate(int ublackLevel, int blackLevel)
{
	level0 = blackLevel;
	level1 = (blackLevel - ublackLevel)/4;
	levelS = level1/2;
}


int CAnalogLevelDecoder::Translate(uint16_t x)
{
	int y = ExpandSign(x) - level0;
	if (y >= 0) y += levelS; else y -= levelS;
	return y/level1 + 1;
}


// === CDtbSource (CSource<uint16_t>) ================================

bool CDtbSource::Open(CTestboard &dtb, unsigned int dataChannel,
		bool endless, unsigned int dtbBufferSize)

{ PROFILING
	if (isOpen) Close();
	if (dataChannel > 8) return false;
	channel = dataChannel;
	tb = &dtb;
	stopAtEmptyData = !endless;
	dtbFifoSize = dtbBufferSize;

	// --- DTB control/state
	dtbRemainingSize = 0;
	dtbState = 0;

	// --- data buffer
	lastSample = 0;
	pos = 0;
	buffer.clear();

	isOpen = tb->Daq_Open(dtbFifoSize, channel) != 0;
	return isOpen;
}


bool CDtbSource::OpenRocAna(CTestboard &dtb, uint8_t tinDelay, uint8_t toutDelay, uint16_t timeout,
	bool endless, unsigned int dtbBufferSize)
{ PROFILING
	if (!Open(dtb, 0, endless, dtbBufferSize)) return false;
	dtb.Daq_Select_ADC(timeout, // 1..65535
		0,  // source: tin/tout
		tinDelay,  // tin delay 0..63
		toutDelay); // tout delay 0..63
	dtb.SignalProbeADC(PROBEA_SDATA1, GAIN_4);
	return true;
}

bool CDtbSource::OpenRocDig(CTestboard &dtb, uint8_t deserAdjust,
		bool endless, unsigned int dtbBufferSize)
{ PROFILING
	if (!Open(dtb, 0, endless, dtbBufferSize)) return false;
	tb->Daq_Select_Deser160(deserAdjust);
	return true;
}


bool CDtbSource::OpenSimulator(CTestboard &dtb, bool endless, unsigned int dtbBufferSize)
{ PROFILING
	if (!Open(dtb, 0, endless, dtbBufferSize)) return false;
	tb->Daq_Select_Datagenerator(0);
	return true;
}


void CDtbSource::Close()
{ PROFILING
	if (!isOpen) return;
	tb->Daq_Close(channel);
	isOpen = false;
}

void CDtbSource::Enable()
{ PROFILING
	if (!isOpen) return;
	tb->Daq_Start(channel);
}

void CDtbSource::Disable()
{ PROFILING
	if (!isOpen) return;
	tb->Daq_Stop(channel);
}


uint16_t CDtbSource::FillBuffer()
{ PROFILING
	if (!isOpen) throw DS_no_dtb_access();
	pos = 0;
	do
	{
		dtbState = tb->Daq_Read(buffer, DTB_SOURCE_BLOCK_SIZE, dtbRemainingSize, channel);
		if (logging) printf("%i(%u/%u)\n", int(dtbState), (unsigned int)(buffer.size()), dtbRemainingSize);
		if (buffer.size() == 0)
		{
			if (stopAtEmptyData) throw DS_empty();
			if (dtbState & (DAQ_FIFO_OVFL | DAQ_MEM_OVFL)) throw DS_buffer_overflow();
		}

	} while (buffer.size() == 0);

	return lastSample = buffer[pos++];
}


// === CBinaryFileSource (CSource<uint16_t>) ================================

uint16_t CBinaryFileSource::FillBuffer()
{ PROFILING
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
{ PROFILING
	record.Clear();
	uint16_t x=0;


	if (!nextStartDetected) x=Get();
	nextStartDetected = false;

	while (!((x=GetLast()) & 0x8000)) Get();
	record.Add(GetLast() & 0x0fff);

	while (!((x=GetLast()) & 0x4000))
	{
		if ((x=Get()) & 0x8000)
		{
			record.SetEndError();
			nextStartDetected = true;
			return &record;
		}
		if (record.GetSize() < 40000) record.Add(GetLast() & 0x0fff);
		else record.SetOverflow();
	}
	return &record;
}



// === CStreamDump (uint16_t, uint16_t) ==============


uint16_t CStreamDump::Read()
{ PROFILING
	x = Get();
	if (f)
	{
		if (row < 9)
		{
			fprintf(f, "%04X ", (unsigned int)x);
			row++;
		}
		else
		{
			fprintf(f, "%04X\n", (unsigned int)x);
			row = 0;
		}
	}
	return x;
}


// === CStreamErrorDump (uint16_t, uint16_t) ==============


uint16_t CStreamErrorDump::Read()
{ PROFILING
	x = Get();
	if (!f) return x;

	n2++;
	if (good && (x & 0x3000))
	{
		good = false;
		fprintf(f, "%7u good\n", n2-n1);
		n1 = n2;
		m = 0;
	}
	else if (!good)
	{
		if (!(x & 0x3000)) m++; else m = 0;
		if (m > 2)
		{
			good = true;
			fprintf(f, "%7u bad\n", n2-n1-2);
			n1 = n2-2;
		}
	}

	return x;
}


// === CRocRawDataPrinter (CDateRecord*, CDataRecord) ==============



CDataRecord* CRocRawDataPrinter::Read()
{ PROFILING
	CDataRecord *x = Get();
	if (f)
	{
		unsigned int n = x->GetSize();
		fprintf(f, "%4u:", n);
		if (adc_samples)
			for (unsigned int i=0; i<n; i++)
				fprintf(f, " %5i", CAnalogLevelDecoder::ExpandSign((*x)[i]));
		else
		for (unsigned int i=0; i<n; i++)
			fprintf(f, " %03X", (unsigned int)((*x)[i]));

		fprintf(f, "\n");
	}
	return x;
}


// === CLevelHistogram (CDataRecord*, CDataRecord*) ==============

//	CHistogram h;
	
CDataRecord* CLevelHistogram::Read()
{
	x = Get();
	if (x->GetSize() >= 3)
	{
		for (unsigned int i = 0; i < x->GetSize(); i++)
			if (i%6 != 2) h.AddData(CAnalogLevelDecoder::ExpandSign((*x)[i]));
	}
	return x;
}


// === CReadBack (CDataRecord*, CDataRecord*) ====================

CDataRecord* CReadBack::Read()
{
	x = Get();
	unsigned int header = x->GetSize() ? (*x)[0] : 0;

	if ((header & 0xffc) == 0x7f8)
	{
		shiftReg <<= 1;	if (header & 1) shiftReg++;
		count++;

		if (header & 2) // start marker
		{
			if (count == 16)
			{
				data = shiftReg & 0xffff;
				updated = true;
				valid = true;
			}
			count = 0;
		}
	}
	else count = 0;

	return x;
}


// === CRocDigDecoder (CDataRecord*, CRocEvent*) ===============================

CRocEvent* CRocDigDecoder::Read()
{ PROFILING
	CDataRecord *sample = Get();
	x.header = 0;
	x.pixel.clear();
	unsigned int n = sample->GetSize();
	if (n > 0)
	{
		if (n > 4) x.pixel.reserve((n-1)/2);
		x.header = (*sample)[0];
		unsigned int pos = 1;
		while (pos < n-1)
		{
			CRocPixel pix;
			pix.raw =  (*sample)[pos++] << 12;
			pix.raw += (*sample)[pos++];
			pix.DecodeRaw();
			x.pixel.push_back(pix);
		}
	}
	return &x;
}


// === CRocAnaDecoder (CDataRecord*, CRocEvent*) ===============================

CRocEvent* CRocAnaDecoder::Read()
{ PROFILING
	CDataRecord *sample = Get();
	x.header = 0;
	x.pixel.clear();
	unsigned int n = sample->GetSize();
	if (n >= 3)
	{
		if (n > 15) x.pixel.reserve((n-3)/6);
		x.header = CAnalogLevelDecoder::ExpandSign((*sample)[2]);
		unsigned int pos = 3;
		while (pos+6 <= n)
		{
			CRocPixel pix;
			pix.raw = 0;
			pix.DecodeAna(dec, &((*sample)[pos]));
			x.pixel.push_back(pix);
			pos += 6;
		}
	}
	return &x;
}


// === CRocEventPrinter (CRocEvent*, CRocEvent*) ============================

CRocEvent* CRocEventPrinter::Read()
{ PROFILING
	x = Get();
	if (f)
	{
		fprintf(f, "%03X(%u):", int(x->header), (unsigned int)(x->pixel.size()));
		for (unsigned int i=0; i<x->pixel.size(); i++)
		{
			fprintf(f, " (%2i/%2i/%3i)", x->pixel[i].x, x->pixel[i].y, x->pixel[i].ph);
		}
		fprintf(f, "\n");
	}
	return x;
}
