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


// error bits:{ ph | x | y | - | - | - | - | - }

void CRocPixel::DecodeRawLinear()
{ PROFILING
	ph = (raw & 0x0f) + ((raw >> 1) & 0xf0);
	error = (raw & 0x10) ? 128 : 0;

	x = ((raw >> 17) & 0x07) + ((raw >> 18) & 0x38);
	if (x >= 52) error |= 64;
	if (raw & 0x1000) error |= 64;

	y = ((raw >>  9) & 0x07) + ((raw >> 10) & 0x78);
//	if (!(y & 0x08)) y--; // !!! address defect. Calculate cluster address
	if (y >= 80) error |= 32;
	if (raw & 0x100000) error |= 32;
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
	return level1 ? y/level1 + 1 : 0;
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
	dtb.Daq_Select_ADC(timeout, // 1..65535
		0,  // source: tin/tout
		tinDelay,  // tin delay 0..63
		toutDelay); // tout delay 0..63
	if (!Open(dtb, 0, endless, dtbBufferSize))
	{
		dtb.Daq_DeselectAll();
		return false;
	}
	dtb.SignalProbeADC(PROBEA_SDATA1, GAIN_4);
	dtb.uDelay(800); // to stabilize ADC input signal
	return true;
}


bool CDtbSource::OpenRocDig(CTestboard &dtb, uint8_t deserAdjust,
		bool endless, unsigned int dtbBufferSize)
{ PROFILING
	dtb.Daq_Select_Deser160(deserAdjust);
	if (!Open(dtb, 0, endless, dtbBufferSize))
	{
		dtb.Daq_DeselectAll();
		return false;
	}
	return true;
}


bool CDtbSource::OpenModDig(CTestboard &dtb, unsigned int channel, bool endless, unsigned int dtbBufferSize)
{ PROFILING
	dtb.Daq_Select_Deser400();
	if (!Open(dtb, channel, endless, dtbBufferSize))
	{
		dtb.Daq_DeselectAll();
		return false;
	}
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


// === CDataRecordScannerROCD (CDataPipe<uint16_t, CRecord*>) =============

CDataRecord* CDataRecordScannerROC::Read()
{ PROFILING
	record.Clear();

	if (!nextStartDetected) Get();
	nextStartDetected = false;

	while (!(GetLast() & 0x8000)) Get();
	record.Add(GetLast() & 0x0fff);
	record.recordNr = recCounter++;

	while (!(GetLast() & 0x4000))
	{
		if (Get() & 0x8000)
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


// === CDataRecordScannerMODD_old (CDataPipe<uint16_t, CRecord*>) =============

CDataRecord* CDataRecordScannerMODD_old::Read()
{ PROFILING
	record.Clear();

	if (!nextStartDetected) Get();
	nextStartDetected = false;

	while ((GetLast() & 0x00f0) != 0x0080) Get();
	record.Add(GetLast());
	record.recordNr = recCounter++;

	while ((GetLast() & 0x00f0) != 0x00f0)
	{
		if ((Get() & 0x00f0) == 0x0080)
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


// === CDataRecordScannerMODD (CDataPipe<uint16_t, CRecord*>) =============

CDataRecord* CDataRecordScannerMODD::Read()
{ PROFILING
	record.Clear();

	if (!nextStartDetected) Get();
	nextStartDetected = false;

	while ((GetLast() & 0xe000) != 0xa000) Get();
	record.Add(GetLast());

	record.recordNr = recCounter++;

	while ((Get() & 0xe000) != 0xc000)
	{
		if ((GetLast() & 0xe000) == 0xa000)
		{
			record.SetEndError();
			nextStartDetected = true;
			return &record;
		}
		if (record.GetSize() < 40000) record.Add(GetLast());
		else record.SetOverflow();
	}
	record.Add(GetLast());

	return &record;
}


// === CStreamDump (uint16_t, uint16_t) ==============

uint16_t CStreamDump::Read()
{ PROFILING
	x = Get();
	if (f)
	{
		if (row < 15)
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


// === CRocRawDataPrinter (CDateRecord*, CDataRecord*) =============



CDataRecord* CRocRawDataPrinter::Read()
{ PROFILING
	CDataRecord *x = Get();
	if (f)
	{
		unsigned int n = x->GetSize();
		fprintf(f, "%4u[%4u]:", x->recordNr, n);
		if (adc_samples)
			for (unsigned int i=0; i<n; i++)
				fprintf(f, " %5i", CAnalogLevelDecoder::ExpandSign((*x)[i]));
		else
		for (unsigned int i=0; i<n; i++)
			fprintf(f, " %04X", (unsigned int)((*x)[i]));

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


// === CRocAnaDecoder (CDataRecord*, CEvent*) ===============================

CEvent* CRocAnaDecoder::Read()
{ PROFILING
	x.roc.clear();

	CDataRecord *sample = Get();
	x.recordNr = sample->recordNr;
	x.deviceType = CEvent::ROCA;
	x.header = x.trailer = 0;
	x.roc.resize(1);

	unsigned int n = sample->GetSize();
	if (n >= 3)
	{
		if (n > 15) x.roc[0].pixel.reserve((n-3)/6);
		x.roc[0].header = CAnalogLevelDecoder::ExpandSign((*sample)[2]);
		unsigned int pos = 3;
		while (pos+6 <= n)
		{
			CRocPixel pix;
			pix.raw = 0;
			pix.DecodeAna(dec, &((*sample)[pos]));
			x.roc[0].pixel.push_back(pix);
			pos += 6;
		}
	}
	return &x;
}


// === CRocDigDecoder (CDataRecord*, CEvent*) ===============================

CEvent* CRocDigDecoder::Read()
{ PROFILING
	x.roc.clear();

	CDataRecord *sample = Get();
	x.recordNr = sample->recordNr;
	x.deviceType = CEvent::ROCD;
	x.header = x.trailer = 0;
	x.roc.resize(1);

	unsigned int n = sample->GetSize();
	if (n > 0)
	{
		if (n > 4) x.roc[0].pixel.reserve((n-1)/2);
		x.roc[0].header = (*sample)[0];
		unsigned int pos = 1;
		while (pos < n-1)
		{
			CRocPixel pix;
			pix.raw =  (*sample)[pos++] << 12;
			pix.raw += (*sample)[pos++];
			pix.DecodeRaw();
			x.roc[0].pixel.push_back(pix);
		}
	}
	return &x;
}


// === CRocDigLinearDecoder (CDataRecord*, CEvent*) =========================

CEvent* CRocDigLinearDecoder::Read()
{ PROFILING
	x.roc.clear();

	CDataRecord *sample = Get();
	x.recordNr = sample->recordNr;
	x.deviceType = CEvent::ROCX;
	x.header = x.trailer = 0;
	x.roc.resize(1);

	unsigned int n = sample->GetSize();
	if (n > 0)
	{
		if (n > 4) x.roc[0].pixel.reserve((n-1)/2);
		x.roc[0].header = (*sample)[0];
		unsigned int pos = 1;
		while (pos < n-1)
		{
			CRocPixel pix;
			pix.raw =  (*sample)[pos++] << 12;
			pix.raw += (*sample)[pos++];
			pix.DecodeRawLinear();
			x.roc[0].pixel.push_back(pix);
		}
	}
	return &x;
}


// === CModDigDecoder_old (CDataRecord*, CEvent*) ===============================

CEvent* CModDigDecoder_old::Read()
{ PROFILING
	x.roc.clear();

	CDataRecord *sample = Get();
	x.recordNr = sample->recordNr;
	x.deviceType = CEvent::MODD;
	x.header = x.trailer = 0;
	x.roc.reserve(8);
	x.error = 0;

	unsigned int pos = 0;
	unsigned int size = sample->GetSize();
	uint16_t v;

	// --- decode TBM header ---------------------------------
	unsigned int raw = 0;

	// H1
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0x80) x.error |= 0x0800;
	raw = v & 0x00f;

	// H2
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0x90) x.error |= 0x0400;
	raw = (raw << 4) + (v & 0x00f);

	// H3
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0xa0) x.error |= 0x0200;
	raw = (raw << 4) + (v & 0x00f);

	// H4
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0xb0) x.error |= 0x0100;
	raw = (raw << 4) + (v & 0x00f);

	x.header = raw;

	// --- decode ROC data -----------------------------------
	CRocEvent roc;

	// while ROC header
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	while ((v & 0x1f0) == 0x070) // R7
	{
		roc.pixel.clear();
		roc.header = v & 0x00f;
		roc.error = 0;

		int px_error;
		CRocPixel pixel;
		v = (pos < size) ? (*sample)[pos++] : 0x100;
		while ((v & 0x1f0) <= 0x060) // R1 ... R6
		{
			px_error = 0;
			pixel.raw = 0;
			for (unsigned int i=1; i<=6; i++)
			{
				if (((v & 0x1f0)>>4) != i) // R<i>
				{
					px_error |= (1<<i);
					if (v & 0x080)
					{
						pixel.error = 0x1fff;
						roc.error |= 0x0001;
						roc.pixel.push_back(pixel);
						x.roc.push_back(roc);
						v = (pos < size) ? (*sample)[pos++] : 0x100;
						goto trailer;
					}
				}
				pixel.raw = (pixel.raw << 4) + (v & 0x00f);
				v = (pos < size) ? (*sample)[pos++] : 0x100;
			}
			pixel.DecodeRaw();
			pixel.error |= (px_error << 7);
			if (pixel.error) roc.error |= 0x0001;
			roc.pixel.push_back(pixel);
		}
		if (roc.error) x.error |= 0x0001;
		x.roc.push_back(roc);
	}

	// --- decode TBM trailer --------------------------------
	trailer:
	raw = 0;

	// T1
	if ((v & 0x1f0) != 0xc0) x.error |= 0x0080;
	raw = v & 0x00f;

	// T2
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0xd0) x.error |= 0x0040;
	raw = (raw << 4) + (v & 0x00f);

	// T3
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0xe0) x.error |= 0x0020;
	raw = (raw << 4) + (v & 0x00f);

	// T4
	v = (pos < size) ? (*sample)[pos++] : 0x100;
	if ((v & 0x1f0) != 0xf0) x.error |= 0x0010;
	raw = (raw << 4) + (v & 0x00f);

	x.trailer = raw;

	return &x;
}


// === CModDigDecoder (CDataRecord*, CEvent*) ===============================

#define MDD_ERROR_MARKER 0x6000

CEvent* CModDigDecoder::Read()
{ PROFILING
	x.roc.clear();

	CDataRecord *sample = Get();
	x.recordNr = sample->recordNr;
	x.deviceType = CEvent::MODD;
	x.header = x.trailer = 0;
	x.roc.reserve(8);
	x.error = 0;

	unsigned int pos = 0;
	unsigned int size = sample->GetSize();
	uint16_t v;

	// --- decode TBM header ---------------------------------
	unsigned int raw = 0;

	// H0
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	if ((v & 0xe000) != 0xa000) x.error |= 0x0800;
	raw = (v & 0x00ff) << 8;

	// H1
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	if ((v & 0xe000) != 0x8000) x.error |= 0x0400;
	raw += v & 0x00ff;

	x.header = raw;

	// --- decode ROC data -----------------------------------
	CRocEvent roc;
	CRocPixel pixel;

	// while ROC header
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	while ((v & 0xe000) == 0x4000) // RH
	{
		roc.pixel.clear();
		roc.header = v & 0x0fff;
		roc.error = 0;

		v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
		while ((v & 0xe000) <= 0x2000) // R0 ... R1
		{
			pixel.error = 0;
			pixel.raw = 0;

			for (unsigned int i=0; i<=1; i++)
			{
				if ((v >> 13) != i) // R<i>
				{
					pixel.error |= i ? 0x0100 : 0x200;
					if (v & 0x8000) // TBM header/trailer
					{
						pixel.error = 0x0400;
						roc.error |= 0x0001;
						roc.pixel.push_back(pixel);
						x.roc.push_back(roc);
						v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
						goto trailer;
					}
				}
				pixel.raw = (pixel.raw << 12) + (v & 0x0fff);
				v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
			}
			if (pixel.raw != 0xffffff)
			{
				pixel.DecodeRaw();
				if (pixel.error) roc.error |= 0x0001;
				roc.pixel.push_back(pixel);
			}
		}
		if (roc.error) x.error |= 0x0001;
		x.roc.push_back(roc);
	}

	// --- decode TBM trailer --------------------------------
	trailer:
	raw = 0;

	// T0
	if ((v & 0xe000) != 0xe000) x.error |= 0x0200;
	else x.error |= (v & 0x1f00) << 1;
	raw = (v & 0x00ff) << 8;

	// T1
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	if ((v & 0xe000) != 0xc000) x.error |= 0x0100;
	raw += v & 0x00ff;

	x.trailer = raw;

	return &x;
}


// === CModDigLinearDecoder (CDataRecord*, CEvent*) =========================

#define MDD_ERROR_MARKER 0x6000

CEvent* CModDigLinearDecoder::Read()
{ PROFILING
	x.roc.clear();

	CDataRecord *sample = Get();
	x.recordNr = sample->recordNr;
	x.deviceType = CEvent::MODX;
	x.header = x.trailer = 0;
	x.roc.reserve(8);
	x.error = 0;

	unsigned int pos = 0;
	unsigned int size = sample->GetSize();
	uint16_t v;

	// --- decode TBM header ---------------------------------
	unsigned int raw = 0;

	// H0
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	if ((v & 0xe000) != 0xa000) x.error |= 0x0800;
	raw = (v & 0x00ff) << 8;

	// H1
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	if ((v & 0xe000) != 0x8000) x.error |= 0x0400;
	raw += v & 0x00ff;

	x.header = raw;

	// --- decode ROC data -----------------------------------
	CRocEvent roc;
	CRocPixel pixel;

	// while ROC header
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	while ((v & 0xe000) == 0x4000) // RH
	{
		roc.pixel.clear();
		roc.header = v & 0x0fff;
		roc.error = 0;

		v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
		while ((v & 0xe000) <= 0x2000) // R0 ... R1
		{
			pixel.error = 0;
			pixel.raw = 0;

			for (unsigned int i=0; i<=1; i++)
			{
				if ((v >> 13) != i) // R<i>
				{
					pixel.error |= i ? 0x0100 : 0x200;
					if (v & 0x8000) // TBM header/trailer
					{
						pixel.error = 0x0400;
						roc.error |= 0x0001;
						roc.pixel.push_back(pixel);
						x.roc.push_back(roc);
						v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
						goto trailer;
					}
				}
				pixel.raw = (pixel.raw << 12) + (v & 0x0fff);
				v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
			}
			if (pixel.raw != 0xffffff)
			{
				pixel.DecodeRawLinear();
				if (pixel.error) roc.error |= 0x0001;
				roc.pixel.push_back(pixel);
			}
		}
		if (roc.error) x.error |= 0x0001;
		x.roc.push_back(roc);
	}

	// --- decode TBM trailer --------------------------------
	trailer:
	raw = 0;

	// T0
	if ((v & 0xe000) != 0xe000) x.error |= 0x0200;
	else x.error |= (v & 0x1f00) << 1;
	raw = (v & 0x00ff) << 8;

	// T1
	v = (pos < size) ? (*sample)[pos++] : MDD_ERROR_MARKER;
	if ((v & 0xe000) != 0xc000) x.error |= 0x0100;
	raw += v & 0x00ff;

	x.trailer = raw;

	return &x;
}


// === CEventPrinter (CEvent*, CEvent*) ============================

CEvent* CEventPrinter::Read()
{ PROFILING
	x = Get();
	unsigned int d;
	if (f && (listAll || x->error))
	{
		switch (x->deviceType)
		{
			case CEvent::ROCA:
			case CEvent::ROCD:
			case CEvent::ROCX:
				fprintf(f, "%03X(%u):", int(x->header), (unsigned int)(x->roc[0].pixel.size()));
				for (unsigned int i=0; i<x->roc[0].pixel.size(); i++)
				{
					fprintf(f, " (%2i/%2i/%3i)", x->roc[0].pixel[i].x, x->roc[0].pixel[i].y, x->roc[0].pixel[i].ph);
				}
				fprintf(f, "\n");
				break;
			case CEvent::MODD:
			case CEvent::MODX:
				// print TBM header
				fprintf(f, "\nEvent: %u\nHeader: ", x->recordNr);
				d = x->header;
				fprintf(f, " EVENT(%u) DATA(%u:%u)",
					(unsigned int)((d>>8) & 0x00ff), // Event
					(unsigned int)((d>>6) & 0x0003), // Data ID
					(unsigned int)(d & 0x003f));     // Data
				if (x->error) fprintf(f, "  ERROR <%03x>", int(x->error));
				fprintf(f, "\n");

				// print ROC header and pixel data
				for (unsigned int r = 0; r < x->roc.size(); r++)
				{
					fprintf(f, "  ROC%2u:%c%03X(%3u):", r, x->roc[r].error ? '*':' ',
						int(x->roc[r].header), (unsigned int)(x->roc[r].pixel.size()));
					for (unsigned int i=0; i<x->roc[r].pixel.size(); i++)
					{
						if (x->roc[r].pixel[i].error)
							fprintf(f, " (%2i/%2i*%3i<%04X>)", x->roc[r].pixel[i].x, x->roc[r].pixel[i].y,
								x->roc[r].pixel[i].ph, x->roc[r].pixel[i].error);
						else
							fprintf(f, " (%2i/%2i/%3i)", x->roc[r].pixel[i].x, x->roc[r].pixel[i].y, x->roc[r].pixel[i].ph);
					}
					fprintf(f, "\n");
				}

				// print TBM trailer
				fprintf(f, "Trailer: ");
				d = x->trailer;
				fprintf(f, " NTOK(%c) RES(%c%c) SYN(%c%c) CTC(%c) CAL(%c) STK(%c%2u) RES(%c%c)",
					(d & 0x8000)?'1':'0', // NTOK
					(d & 0x4000)?'T':' ', // RES
					(d & 0x2000)?'R':' ',
					(d & 0x1000)?'E':' ', // SYN
					(d & 0x0800)?'T':' ',
					(d & 0x0400)?'1':'0', // CTC
					(d & 0x0200)?'1':'0', // CAL
					(d & 0x0100)?'F':' ', // STK
					(unsigned int)(d & 0x003f),
					(d & 0x0080)?'A':' ',
					(d & 0x0040)?'P':' ');
				fprintf(f, "\n");

				break;
			default: break;
		}
	}
	return x;
}


// === CReadBackLogger(CEvent*, CEvent*) ============================


void CReadbackValue::Add(unsigned int v)
{
	shift <<= 1; n++;
	if (v&1) shift++;
	if (v&2)
	{
		value = shift;
		updated = true;
		shift = 0; n = 0;
	}
}

bool CReadbackValue::Get(unsigned short &rdbValue)
{
	rdbValue = value;
	bool upd = updated;
	updated = false;
	return upd;
}


CEvent* CReadbackLogger::Read()
{
	x = Get();
	unsigned int r;
	if (!x->error)
	{
		for (r = 0; r < 8; r++)
		{
			if (r < x->roc.size())
			{
				rdb[r].Add(x->roc[r].header);
			} else rdb[r].Reset();
		}
	}
	else for (r = 0; r < 8; r++) rdb[r].Reset();

	if (f)
	{
		fprintf(f, "%4u:", x->recordNr);
		unsigned short value;
		for (r = 0; r < 8; r++)
		{
			if (rdb[r].Get(value))
				fprintf(f, " %04X", int(value));
			else fputs(" ----", f);
		}
		fputs("\n", f);
	}
	return x;
}

