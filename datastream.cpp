// datastream.cpp

#include "datastream.h"
#include "protocol.h"


// === Data structures ======================================================

void CRocPixel::DecodeRaw()
{ PROFILING
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


bool CDtbSource::OpenRocAna(CTestboard &dtb, uint8_t tinDelay, uint8_t toutDel, uint16_t timeout,
	bool endless, unsigned int dtbBufferSize)
{ PROFILING
	if (!Open(dtb, 0, endless, dtbBufferSize)) return false;
	dtb.Daq_Select_ADC(timeout, // 1..65535
		0,  // source: tin/tout
		1,  // tin delay 0..63
		1); // tout delay 0..63
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
		fprintf(f, "%u:", n);
		for (unsigned int i=0; i<n; i++) fprintf(f, " %03X", (unsigned int)((*x)[i]));
		fprintf(f, "\n");
	}
	return x;
}





// === CDecoder (CDataRecord*, CRocEvent*) =====================================

CRocEvent* CRocDecoder::Read()
{ PROFILING
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
	return &roc_event;
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
