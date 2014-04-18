/* -------------------------------------------------------------
 *
 *  file:        command.cpp
 *
 *  description: command line interpreter for Chip/Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    31.8.2007
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */


#include "cmd.h"


CMD_PROC(takedata)
{
	FILE *f = fopen("daqdata.bin", "wb");
	if (f == 0)
	{
		printf("Could not open data file\n");
		return true;
	}

	uint8_t status = 0;
	uint32_t n;
	vector<uint16_t> data;

	unsigned int sum = 0;
	double mean_n = 0.0;
	double mean_size = 0.0;

	clock_t t = clock();
	unsigned int memsize = tb.Daq_Open(10000000);
	tb.Daq_Start();
	clock_t t_start = clock();
	while (!keypressed())
	{
		// read data and status from DTB
		status = tb.Daq_Read(data, 40000, n);

		// make statistics
		sum += data.size();
		mean_n    = /* 0.2*mean_n    + 0.8* */ n;
		mean_size = /* 0.2*mean_size + 0.8* */ data.size();

		// write statistics every second
//		printf(".");
		if ((clock() - t) > CLOCKS_PER_SEC/4)
		{
			printf("%5.1f%%  %5.0f  %u\n", mean_n*100.0/memsize, mean_size, sum);
			t = clock();
		}

		// write data to file
		if (fwrite(data.data(), sizeof(uint16_t), data.size(), f) != data.size())
		{ printf("\nFile write error"); break; }

		// abort after overflow error
//		if (((status & 1) == 0) && (n == 0)) break;
		if (status & 0x6) break;

//		tb.mDelay(5);
	}
	tb.Daq_Stop();
	double t_run = double(clock() - t_start)/CLOCKS_PER_SEC;

	tb.Daq_Close();

	if (keypressed()) getchar();

	printf("\n");
	if      (status & 4) printf("FIFO overflow\n");
	else if (status & 2) printf("Memory overflow\n");
	printf("Data taking aborted\n%u samples in %0.1f s read (%0.0f samples/s)\n", sum, t_run, sum/t_run);

	fclose(f);
	return true;
}



#define DECBUFFERSIZE 2048

class Decoder
{
	int printEvery;

	int nReadout;
	int nPixel;

	FILE *f;
	int nSamples;
	uint16_t *samples;

	int x, y, ph;
	void Translate(unsigned long raw);
public:
    Decoder() : printEvery(0), nReadout(0), nPixel(0), f(0), nSamples(0), samples(0) {}
	~Decoder() { Close(); }
	bool Open(const char *filename);
	void Close() { if (f) fclose(f); f = 0; delete[] samples; }
	bool Sample(uint16_t sample);
	void AnalyzeSamples();
	void DumpSamples(int n);
};

bool Decoder::Open(const char *filename)
{
	samples = new uint16_t[DECBUFFERSIZE];
	f = fopen(filename, "wt");
	return f != 0;
}

void Decoder::Translate(unsigned long raw)
{
	ph = (raw & 0x0f) + ((raw >> 1) & 0xf0);
	raw >>= 9;
	int c =    (raw >> 12) & 7;
	c = c*6 + ((raw >>  9) & 7);
	int r =    (raw >>  6) & 7;
	r = r*6 + ((raw >>  3) & 7);
	r = r*6 + ( raw        & 7);
	y = 80 - r/2;
	x = 2*c + (r&1);
}

void Decoder::AnalyzeSamples()
{
	if (nSamples < 1) { nPixel = 0; return; }
	fprintf(f, "%5i: %03X: ", nReadout, (unsigned int)(samples[0] & 0xfff));
	nPixel = (nSamples-1)/2;
	int pos = 1;
	for (int i=0; i<nPixel; i++)
	{
		unsigned long raw = (samples[pos++] & 0xfff) << 12;
		raw += samples[pos++] & 0xfff;
		Translate(raw);
		fprintf(f, " %2i", x);
	}

//	for (pos = 1; pos < nSamples; pos++) fprintf(f, " %03X", int(samples[pos]));
	fprintf(f, "\n");

}

void Decoder::DumpSamples(int n)
{
	if (nSamples < n) n = nSamples;
	for (int i=0; i<n; i++) fprintf(f, " %04X", (unsigned int)(samples[i]));
	fprintf(f, " ... %04X\n", (unsigned int)(samples[nSamples-1]));
}

bool Decoder::Sample(uint16_t sample)
{
	if (sample & 0x8000) // start marker
	{
		if (nReadout && printEvery >= 1000)
		{
			AnalyzeSamples();
			printEvery = 0;
		} else printEvery++;
		nReadout++;
		nSamples = 0;
	}
	if (nSamples < DECBUFFERSIZE)
	{
		samples[nSamples++] = sample;
		return true;
	}
	return false;
}



CMD_PROC(takedata2)
{
	Decoder dec;
	if (!dec.Open("daqdata2.txt"))
	{
		printf("Could not open data file\n");
		return true;
	}

	uint8_t status = 0;
	uint32_t n;
	vector<uint16_t> data;

	unsigned int sum = 0;
	double mean_n = 0.0;
	double mean_size = 0.0;

	clock_t t = clock();
	unsigned long memsize = tb.Daq_Open(60000000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);
	tb.Daq_Start();
	clock_t t_start = clock();
	while (!keypressed())
	{
		// read data and status from DTB
		status = tb.Daq_Read(data, 8000000, n, 0);

		// make statistics
		sum += data.size();
		mean_n    = /* 0.2*mean_n    + 0.8* */ n;
		mean_size = /* 0.2*mean_size + 0.8* */ data.size();

		// write statistics every second
//		printf(".");
		if ((clock() - t) > CLOCKS_PER_SEC)
		{
			printf("%5.1f%%  %5.0f  %u\n", mean_n*100.0/memsize, mean_size, sum);
			t = clock();
		}

		// decode file
		for (unsigned int i=0; i<data.size(); i++) dec.Sample(data[i]);

		// abort after overflow error
		if (((status & 1) == 0) && (n == 0)) break;

//		tb.mDelay(5);
	}
	tb.Daq_Stop();
	double t_run = double(clock() - t_start)/CLOCKS_PER_SEC;

	tb.Daq_Close();

	if (keypressed()) getchar();

	printf("\nstatus: %02X ", int(status));
	if      (status & 4) printf("FIFO overflow\n");
	else if (status & 2) printf("Memory overflow\n");
	printf("Data taking aborted\n%u samples in %0.1f s read (%0.0f samples/s)\n", sum, t_run, sum/t_run);

	return true;
}


CMD_PROC(showclk)
{
	const unsigned int nSamples = 20;
	const int gain = 1;
//	PAR_INT(gain,1,4);

	unsigned int i, k;
	vector<uint16_t> data[20];

	tb.Pg_Stop();
	tb.Pg_SetCmd( 0, PG_SYNC +  5);
	tb.Pg_SetCmd( 1, PG_CAL  +  6);
	tb.Pg_SetCmd( 2, PG_TRG  +  6);
	tb.Pg_SetCmd( 3, PG_RESR +  6);
	tb.Pg_SetCmd( 4, PG_REST +  6);
	tb.Pg_SetCmd( 5, PG_TOK);

	tb.SignalProbeD1(9);
	tb.SignalProbeD2(17);
	tb.SignalProbeA2(PROBEA_CLK);
	tb.uDelay(10);
	tb.SignalProbeADC(PROBEA_CLK, gain-1);
	tb.uDelay(10);

	tb.Daq_Select_ADC(nSamples, 0, 1, 1);
	tb.uDelay(1000);
	tb.Daq_Open(1024);
	for (i=0; i<20; i++)
	{
		tb.Sig_SetDelay(SIG_CLK, 26-i);
		tb.uDelay(10);
		tb.Daq_Start();
		tb.Pg_Single();
		tb.uDelay(1000);
		tb.Daq_Stop();
		tb.Daq_Read(data[i], 1024);
		if (data[i].size() != nSamples)
		{
			printf("Data size %i: %i\n", i, int(data[i].size()));
			return true;
		}
	}
	tb.Daq_Close();
	tb.Flush();

	int n = 20*nSamples;
	vector<double> values(n);
	int x = 0;
	for (k=0; k<nSamples; k++) for (i=0; i<20; i++)
	{
		int y = (data[i])[k] & 0x0fff;
		if (y & 0x0800) y |= 0xfffff000;
		values[x++] = y;
	}
	Scope("CLK", values);

	return true;
}

CMD_PROC(showctr)
{
	const unsigned int nSamples = 60;
	const int gain = 1;
//	PAR_INT(gain,1,4);

	unsigned int i, k;
	vector<uint16_t> data[20];

	tb.Pg_Stop();
	tb.Pg_SetCmd( 0, PG_SYNC +  5);
	tb.Pg_SetCmd( 1, PG_CAL  +  6);
	tb.Pg_SetCmd( 2, PG_TRG  +  6);
	tb.Pg_SetCmd( 3, PG_RESR +  6);
	tb.Pg_SetCmd( 4, PG_REST +  6);
	tb.Pg_SetCmd( 5, PG_TOK);

	tb.SignalProbeD1(9);
	tb.SignalProbeD2(17);
	tb.SignalProbeA2(PROBEA_CTR);
	tb.uDelay(10);
	tb.SignalProbeADC(PROBEA_CTR, gain-1);
	tb.uDelay(10);

	tb.Daq_Select_ADC(nSamples, 1, 1);
	tb.uDelay(1000);
	tb.Daq_Open(1024);
	for (i=0; i<20; i++)
	{
		tb.Sig_SetDelay(SIG_CTR, 26-i);
		tb.uDelay(10);
		tb.Daq_Start();
		tb.Pg_Single();
		tb.uDelay(1000);
		tb.Daq_Stop();
		tb.Daq_Read(data[i], 1024);
		if (data[i].size() != nSamples)
		{
			printf("Data size %i: %i\n", i, int(data[i].size()));
			return true;
		}
	}
	tb.Daq_Close();
	tb.Flush();

	int n = 20*nSamples;
	vector<double> values(n);
	int x = 0;
	for (k=0; k<nSamples; k++) for (i=0; i<20; i++)
	{
		int y = (data[i])[k] & 0x0fff;
		if (y & 0x0800) y |= 0xfffff000;
		values[x++] = y;
	}
	Scope("CTR", values);

/*
	FILE *f = fopen("X:\\developments\\adc\\data\\adc.txt", "wt");
	if (!f) { printf("Could not open File!\n"); return true; }
	double t = 0.0;
	for (k=0; k<100; k++) for (i=0; i<20; i++)
	{
		int x = (data[i])[k] & 0x0fff;
		if (x & 0x0800) x |= 0xfffff000;
		fprintf(f, "%7.2f %6i\n", t, x);
		t += 1.25;
	}
	fclose(f);
*/
	return true;
}


CMD_PROC(showsda)
{
	const unsigned int nSamples = 52;
	unsigned int i, k;
	vector<uint16_t> data[20];

	tb.SignalProbeD1(9);
	tb.SignalProbeD2(17);
	tb.SignalProbeA2(PROBEA_SDA);
	tb.uDelay(10);
	tb.SignalProbeADC(PROBEA_SDA, 0);
	tb.uDelay(10);

	tb.Daq_Select_ADC(nSamples, 2, 7);
	tb.uDelay(1000);
	tb.Daq_Open(1024);
	for (i=0; i<20; i++)
	{
		tb.Sig_SetDelay(SIG_SDA, 26-i);
		tb.uDelay(10);
		tb.Daq_Start();
		tb.roc_Pix_Trim(12, 34, 5);
		tb.uDelay(1000);
		tb.Daq_Stop();
		tb.Daq_Read(data[i], 1024);
		if (data[i].size() != nSamples)
		{
			printf("Data size %i: %i\n", i, int(data[i].size()));
			return true;
		}
	}
	tb.Daq_Close();
	tb.Flush();

	int n = 20*nSamples;
	vector<double> values(n);
	int x = 0;
	for (k=0; k<nSamples; k++) for (i=0; i<20; i++)
	{
		int y = (data[i])[k] & 0x0fff;
		if (y & 0x0800) y |= 0xfffff000;
		values[x++] = y;
	}
	Scope("SDA", values);

	return true;
}


CMD_PROC(dselmod)
{
	tb.Daq_Select_Deser400();
	DO_FLUSH
	return true;
}

CMD_PROC(dmodres)
{
	int reset;
	if (!PAR_IS_INT(reset, 0, 3)) reset = 3;
	tb.Daq_Deser400_Reset(reset);
	DO_FLUSH
	return true;
}

CMD_PROC(dselroc)
{
	int shift;
	PAR_INT(shift,0,7);
	tb.Daq_Select_Deser160(shift);
	DO_FLUSH
	return true;
}

CMD_PROC(dselroca)
{
	int datasize;
	PAR_INT(datasize, 1, 2047);
	tb.Daq_Select_ADC(datasize, 1, 4, 6);
	DO_FLUSH
	return true;
}

CMD_PROC(dselsim)
{
	int startvalue;
	if (!PAR_IS_INT(startvalue, 0, 16383)) startvalue = 0;
	tb.Daq_Select_Datagenerator(startvalue);
	DO_FLUSH
	return true;
}

CMD_PROC(dseloff)
{
	tb.Daq_DeselectAll();
	DO_FLUSH
	return true;
}





void decoding_show2(vector<uint16_t> &data)
{
	int size = data.size();
	if (size > 6) size = 6;
	for (int i=0; i<size; i++)
	{
		uint16_t x = data[i];
		for (int k=0; k<12; k++)
		{
			Log.puts((x & 0x0800)? "1" : "0");
			x <<= 1;
			if (k == 3 || k == 7 || k == 11) Log.puts(" ");
		}
	}
}

void decoding_show(vector<uint16_t> *data)
{
	int i;
	for (i=1; i<16; i++) if (data[i] != data[0]) break;
	decoding_show2(data[0]);
	Log.puts("  ");
	if (i<16) decoding_show2(data[i]); else Log.puts(" no diff");
	Log.puts("\n");
}

CMD_PROC(decoding)
{
	unsigned short t;
	vector<uint16_t> data[16];
	tb.Pg_SetCmd(0, PG_TOK + 0);
	tb.Daq_Select_Deser160(2);
	tb.uDelay(10);
	Log.section("decoding");
	tb.Daq_Open(100);
	for (t=0; t<44; t++)
	{
		tb.Sig_SetDelay(SIG_CLK, t);
		tb.Sig_SetDelay(SIG_TIN, t+5);
		tb.uDelay(10);
		for (int i=0; i<16; i++)
		{
			tb.Daq_Start();
			tb.Pg_Single();
			tb.uDelay(200);
			tb.Daq_Stop();
			tb.Daq_Read(data[i], 200);
		}
		Log.printf("%3i ", int(t));
		decoding_show(data);
	}
	tb.Daq_Close();
	Log.flush();
	return true;
}




// =======================================================================
//  experimential ROC test commands
// =======================================================================


CMD_PROC(daqtest)
{
	// setup pg data sequence (0, 1, 2, ... 99)
	tb.Pg_SetCmd(0, PG_RESR + 4);
	for (int i=1; i<100; i++) tb.Pg_SetCmd(i, PG_TOK + 4);
	tb.Pg_SetCmd(100, PG_TOK);

	do
	{
		// create data
		tb.Daq_Open(500000, 0);
		tb.Daq_Select_Datagenerator(0);
		tb.Daq_Start(0);
		for (int i=0; i<1000; i++) tb.Pg_Single();
		tb.Daq_Stop(0);

		try
		{
			uint32_t n;
			vector<uint16_t> data;
			do
			{
				tb.Daq_Read(data, 32768, n, 0);
			} while (n != 0);
		}
		catch (DataPipeException e) { printf("\nERROR: %s\n", e.what()); return false; }
	} while (!keypressed());

	tb.Daq_Close(0);
	return true;
}


class CDemoAnalyzer : public CAnalyzer
{
	CRocEvent* Read();
};

CRocEvent* CDemoAnalyzer::Read()
{
	CRocEvent* ev = Get();

	Log.printf("Header: %03X\n", int(ev->header));
	for (unsigned int i=0; i<ev->pixel.size(); i++)
		Log.printf("[%3i %3i %3i]\n", ev->pixel[i].x, ev->pixel[i].y, ev->pixel[i].ph);
	Log.printf("\n");
	return ev;
};


class CDemoAnalyzer2 : public CAnalyzer
{
	CRocEvent* Read();
};


CRocEvent* CDemoAnalyzer2::Read()
{
	CRocEvent* ev = Get();

	Log.printf("%03X ", int(ev->header));
	for (unsigned int i=0; i<ev->pixel.size(); i++)
		Log.printf(" %3i", ev->pixel[i].ph);
	Log.printf("\n");
	return ev;
};


// --- Level Histo

class CLevelHisto : public CAnalyzer
{
	unsigned int pos;
	unsigned int h[256];
	CRocEvent* Read();
public:
	CLevelHisto(unsigned int ro_pos);
	void Clear();
	void Report(FILE *f, int min, int max);
};


CLevelHisto::CLevelHisto(unsigned int rp_pos)
{
	pos = rp_pos;
	Clear();
}

void CLevelHisto::Clear()
{
	for (int i=0; i<256; i++) h[i] = 0;
}

void CLevelHisto::Report(FILE *f, int min, int max)
{
	for (int i=min; i<=max; i++) fprintf(f, "%3i, %5u\n", i, h[i]);
}


CRocEvent* CLevelHisto::Read()
{
	CRocEvent* ev = Get();
	if (ev->pixel.size() > pos)
	{
		unsigned int x = ev->pixel[pos].ph;
		if (x < 256) h[x]++;
	}
	return ev;
};


// ---
class CPrint : public CAnalyzer
{
	CRocEvent* Read();
};


CRocEvent* CPrint::Read()
{
	CRocEvent* ev = Get();
	for (unsigned int i = 0; i < ev->pixel.size(); i++)
	{
		printf(" %3u", ev->pixel[i].ph);
	}
	printf("\n");
	return ev;
};


CMD_PROC(analyze)
{ PROFILING
	int vc;
	PAR_INT(vc,0,255)

	CDtbSource src;
	src.Logging(true);
	CStreamDump srcdump("streamdump.txt");
	CDataRecordScanner rec;
	CRocRawDataPrinter rawList("raw.txt");

//	CSink<CDataRecord*> pump;

	CRocDigDecoder decoder;
	CRocEventPrinter evList("eventlist.txt");
	CSink<CRocEvent*> pump;
//	CLevelHisto l(0);

//	src >> rec >> decoder >> pump;
	src >> srcdump >> rec >> rawList >> decoder >> evList >> pump;

	src.OpenRocDig(tb, settings.deser160_tinDelay, true, 20000000);
//	src.OpenSimulator(tb, true, 1000000);
	src.Enable();
	tb.uDelay(100);
	tb.Pg_Loop(2500);
//	printf("waiting...\n"); tb.mDelay(30000);

	/*	tb.Pg_Single(); tb.uDelay(100);
	src.Clear();
	for (int i=0; i<10; i++)
	{
		tb.Pg_Single();
		tb.uDelay(100);
	}
*/
	try
	{
		int i=0;
		while (i++ < 500000 && !keypressed()) { pump.Get(); /* tb.uDelay(10); */ }
		tb.Pg_Stop();
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

//	printf("Bytes Transfered: %u\n", srcdump.ByteCount());

	src.Disable();
	return true;
}


CMD_PROC(analyzeana)
{ PROFILING
	int vc;
	PAR_INT(vc,0,255)

	CDtbSource src;
	src.Logging(true);
	CStreamDump srcdump("streamdump.txt");
	CDataRecordScanner rec;
	CLevelHistogram hist;
	CRocRawDataPrinter rawList("raw.txt", true);
	CRocAnaDecoder decode;
	decode.Calibrate(-364, -30);
	CRocEventPrinter evList("eventlist.txt");
	CSink<CRocEvent*> pump;

	src >> srcdump >> rec >> hist >> rawList >> decode >> evList >> pump;

	src.OpenRocAna(tb, 14, 10, 100, true, 20000);
	src.Enable();
	tb.uDelay(100);
	tb.Pg_Loop(20000);

	try
	{
		int i=0;
		while (i++ < 500000 && !keypressed()) { pump.Get(); /* tb.uDelay(10); */ }
		tb.Pg_Stop();
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

//	printf("Bytes Transfered: %u\n", srcdump.ByteCount());

	src.Disable();

	Log.section("ALEVEL");
	hist.Report(Log);
	return true;
}


CMD_PROC(adcsingle)
{
/*	CDtbSource src(tb, false);
	CDataRecordScanner rec;
	CRocDecoder dec;
	CPrint print;
	CDemoAnalyzer print;
	CSink<CRocEvent*> pump;

	src >> rec >> dec >> print >> pump;

	tb.Daq_Open(1000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);
	tb.Daq_Start();
	tb.uDelay(10);

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 20);
	tb.Pg_SetCmd(2, PG_SYNC|PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Pg_Single();
	tb.uDelay(100);

	try { pump.GetAll(); }
	catch (DS_empty &) {}
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

	tb.Daq_Stop();
	tb.Daq_Close();
	Log.flush();
	*/
	return true;

}


CMD_PROC(adcpeak)
{
/*	CDtbSource src(tb, false);
	CDataRecordScanner rec;
	CRocDecoder dec;
	CLevelHisto l(0);
	CSink<CRocEvent*> pump;

	src >> rec >> dec >> l >> pump;
*/
//	tb.roc_SetDAC(WBC,  15);
//	tb.roc_SetDAC(CtrlReg,0x04); // high range
/*
	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 20);
	tb.Pg_SetCmd(2, PG_SYNC|PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(2000000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);

	tb.Daq_Start();
	tb.uDelay(10);
	tb.roc_Pix_Trim(5, 5, 15);
	tb.roc_Pix_Cal( 5, 5);
//	tb.roc_SetDAC();
	tb.uDelay(100);

	tb.Pg_Loop(2000);
	try
	{
		putchar('#');
		for (int i=0; i<50; i++)
		{
			for (int k=0; k<1000; k++) pump.Get();
			putchar('.');
			if (keypressed()) break;
		}
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

	tb.Pg_Stop();
	printf(" \n");
	tb.Daq_Stop();
	tb.Daq_Close();

	Log.section("ADCPEAK");
	l.Report(Log.File(), 0, 255);
	FILE *f = fopen("adchisto\\adchisto.txt", "wt");
	if (f)
	{
		l.Report(f, 0, 255);
		fclose(f);
	} else printf("Error writing adchisto.txt\n");

	Log.flush();
*/
	return true;
}


CMD_PROC(adchisto)
{
/*	CDtbSource src(tb, false);
	CDataRecordScanner rec;
	CRocDecoder dec;
	CLevelHisto l(0);
	CSink<CRocEvent*> pump;

	src >> rec >> dec >> l >> pump;

//	tb.roc_SetDAC(WBC,  15);
	tb.roc_SetDAC(CtrlReg,0x04); // high range

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 20);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(1000000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);

	tb.Daq_Start();
	tb.uDelay(10);
	tb.roc_Pix_Trim(5, 5, 15);
	tb.roc_Pix_Cal( 5, 5);

	tb.uDelay(100);

	tb.roc_SetDAC(Vcal, 0);
	tb.Pg_Loop(2000);
	try
	{
		for (int t=20; t<120; t++)
		{
			tb.roc_SetDAC(Vcal, t);
			for (int s=100; s<130; s++)
			{
				tb.roc_SetDAC(VoffsetRO, s);
				tb.uDelay(100);
				for (int i=0; i<500; i++) pump.Get();
			}
			putchar('.');
			if (keypressed()) break;
		}
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }
	printf("\n");
	tb.Pg_Stop();

	tb.Daq_Stop();

	tb.Daq_Close();

	Log.section("ADCHISTO");
	l.Report(Log.File(), 0, 255);
	FILE *f = fopen("adchisto\\adchisto.txt", "wt");
	if (f)
	{
		l.Report(f, 0, 255);
		fclose(f);
	} else printf("Error writing adchisto.txt\n");

	Log.flush();
*/
	return true;
}



class CAdcLevelHisto : public CAnalyzer
{
	unsigned int n;
	unsigned int m;
	CRocEvent* Read();
public:
	unsigned int h[256];
	CAdcLevelHisto() { Clear(); }
	void Clear();
	double Mean() { return n ? double(m)/n : 0.0; }
	void Report(FILE *f, int min, int max);
};


void CAdcLevelHisto::Clear()
{
	n = m = 0;
	for (int i=0; i<256; i++) h[i] = 0;
}

CRocEvent* CAdcLevelHisto::Read()
{
	CRocEvent* ev = Get();
	if (ev->pixel.size() > 0)
	{
		unsigned int x = ev->pixel[0].ph;
		if (x < 256)
		{
			h[x]++;
			m += x;
			n++;
		}
	}
	return ev;
};

void CAdcLevelHisto::Report(FILE *f, int min, int max)
{
	if (min < 0) min = 0;
	if (max >=256) max = 255;
	fprintf(f, " %5.1f", Mean());
	for (int i=min; i<=max; i++) fprintf(f, " %3u", h[i]);
	fprintf(f, "\n");
}


CMD_PROC(adctransfer)
{
/*
	int mode;
	PAR_INT(mode, 0, 1);

	FILE *f = fopen("adchisto\\adctransfer.txt", "wt");
	if (!f)
	{
		printf("Error open adctransfer.txt\n");
		return true;
	}
	CDotPlot plot;

	CDtbSource src(tb, false);
	CDataRecordScanner rec;
	CRocDecoder dec;
	CAdcLevelHisto l;
	CSink<CRocEvent*> pump;
	src >> rec >> dec >> l >> pump;

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 20);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(1000000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);

	tb.Daq_Start();
	tb.uDelay(10);
//	tb.roc_Pix_Trim(5, 5, 15);
//	tb.roc_Pix_Cal( 5, 5);

	tb.uDelay(100);
	const int cntPerStep = 100;
	try
	{
#define DAC_PARAM
#ifdef DAC_PARAM
		for (int offs = 0; offs <= 256; offs += 32)
		{
			tb.roc_SetDAC(VoffsetRO, (offs==256)? 255 : offs);
#endif
			for (int t=40; t<200; t+=1)
			{
				l.Clear();
				tb.roc_SetDAC(Vcal, t);
				tb.uDelay(100);
				int s;
				for (s=0; s<cntPerStep; s++)
				{
					tb.Pg_Single();
					tb.uDelay(100);
				}
				for (s=0; s<cntPerStep; s++) pump.Get();

				if (mode == 1) for (s=0; s<256; s++) plot.Add(t, s, l.h[s]);
				else plot.AddMean(t, l.Mean());

				fprintf(f, "%3i ", t);
				l.Report(f, 0, 255);
				putchar('.');
				if (keypressed()) break;
			}
#ifdef DAC_PARAM
		}
#endif
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }
	printf("\n");

	tb.Pg_Stop();

	tb.Daq_Stop();

	tb.Daq_Close();

	fclose(f);
	plot.Show();
	*/
	return true;
}


void adctest_single()
{
/*	int x, y = 10;
	tb.Daq_Start();
	for (x=0; x<ROC_NUMCOLS; x++)
	{
		tb.roc_Pix_Trim(x, y, 0);
		tb.Pg_Single();
		tb.uDelay(100);
		tb.roc_Pix_Mask(x, y);
	}
	tb.Daq_Stop();

	CReadout data;
	data.Init();

	int p[ROC_NUMCOLS];
	for (x=0; x<ROC_NUMCOLS; x++)
	{
		data.Read();
		p[x] = (data.pixel.size() != 0)? (data.pixel.front()).ph : -1;
	}

	Log.section("ADCTEST");
	for (x=0; x<ROC_NUMCOLS; x++) Log.printf(" %3i", p[x]);
	Log.printf("\n");
*/
}


CMD_PROC(adctest)
{
	tb.roc_SetDAC(WBC, 100);
	tb.roc_SetDAC(Vcal, 20);
	tb.roc_SetDAC(CtrlReg,0x04); // high range

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 100);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(100000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);

	// single pixel readout
	try
	{
		adctest_single();
	}
	catch (int e)
	{
		printf("ERROR %i\n", e);
	}

	tb.Daq_Close();
	return true;
}


CMD_PROC(ethsend)
{
	char msg[45];
	PAR_STRINGEOL(msg,45);
	string message = msg;
	tb.Ethernet_Send(message);
	DO_FLUSH
	return true;
}

CMD_PROC(ethrx)
{
	unsigned int n = tb.Ethernet_RecvPackets();
	printf("%u packets received\n", n);
	return true;
}


void PrintScale(int min, int max)
{
	int x;
	Log.puts("     |");
	for (x=min; x<=max; x++) Log.printf("%i", (x/100)%10);
	Log.puts("\n     |");
	for (x=min; x<=max; x++) Log.printf("%i", (x/10)%10);
	Log.puts("\n     |");
	for (x=min; x<=max; x++) Log.printf("%i", x%10);
	Log.puts("\n-----+");
	for (x=min; x<=max; x++) Log.puts("-");
	Log.puts("\n");
}


void Scan1D(CDtbSource &src, CSink<CRocEvent*> &data, int vx, int xmin, int xmax, int xstep)
{
	int x, k;

	// --- take data
	src.Enable();
	for (x = xmin; x<=xmax; x)
	{
		tb.roc_SetDAC(vx, x);
		tb.uDelay(100);
		for (k=0; k<10; k++)
		{
			tb.Pg_Single();
			tb.uDelay(5);
		}
	}
	src.Disable();

	// --- analyze data
	int count;
	int spos = 0;
	char s[260];

	try
	{
		for (x = xmin; x<=xmax; x++)
		{
			count = 0;
			for (k=0; k<10; k++)
			{
				CRocEvent *ev = data.Get();
				if (ev->pixel.size() != 0) count++;
			}
			if (count == 0) s[spos++] = '.';
			else if (count >= 10) s[spos++] = '*';
			else s[spos++] = count + '0';
		}
	} catch (DataPipeException e) { printf("\nERROR Scan1D: %s\n", e.what()); return; }
	s[spos] = 0;
	Log.printf("%s\n", s);
	Log.flush();
}



CMD_PROC(shmoo)
{
	int vx, xmin, xmax, vy, ymin, ymax;
	PAR_INT(vx, 0, 0xff);
	PAR_RANGE(xmin,xmax, 0,255);
	PAR_INT(vy, 0, 0xff);
	PAR_RANGE(ymin,ymax, 0,255);

	int count = xmax-xmin;
	if (count < 1 || count > 256) return true;

	Log.section("SHMOO",false);
	Log.printf("regX(%i)=%i:%i;  regY(%i)=%i:%i\n",
		vx, xmin, xmax, vy, ymin, ymax);

	CDtbSource src;
	CDataRecordScanner raw;
	CRocDigDecoder dec;
	CSink<CRocEvent*> data;
	src >> raw >> dec >> data;
	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 8000000);

	PrintScale(xmin, xmax);
	for (int y=ymin; y<=ymax; y++)
	{
		tb.roc_SetDAC(vy,y);
		tb.uDelay(100);
		Log.printf("%5i|", y);
		Scan1D(src, data, vx, xmin, xmax, 1);
	}

	tb.Daq_Close();

	return true;
}



/*
CMD_PROC(phscan)
{
	int col, row;
	PAR_INT(col, 0, 51)
	PAR_INT(row, 0, 79)

	const int vcalmin = 0;
	const int vcalmax = 140;

	// load settings
//	tb.roc_SetDAC(CtrlReg,0x00); // 0x04

	tb.Pg_Stop();
	tb.Pg_SetCmd(0, PG_RESR + 15);
	tb.Pg_SetCmd(1, PG_CAL  + 20);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);

	tb.roc_SetDAC(WBC,  15);

	tb.Daq_Open(50000);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);
	tb.Daq_Start();

	// --- scan vcal
	tb.roc_Col_Enable(col, true);
	tb.roc_Pix_Trim(col, row, 15);
	tb.roc_Pix_Cal (col, row, false);

	for (int cal = vcalmin; cal < vcalmax; cal++)
	{
		tb.roc_SetDAC(Vcal, cal);
		tb.uDelay(100);
		for (int k=0; k<5; k++)
		{
			tb.Pg_Single();
			tb.uDelay(50);
		}
	}

	tb.roc_Pix_Mask(col, row);
	tb.roc_Col_Enable(col, false);
	tb.roc_ClrCal();

	tb.Daq_Stop();
	vector<uint16_t> data;
	tb.Daq_Read(data, 4000);
	tb.Daq_Close();

	// --- plot data
	PixelReadoutData pix;
	int pos = 0;
	int dpos = 0;
	vector<double> y(vcalmax-vcalmin, 0.0);
	try
	{
		for (int cal = vcalmin; cal < vcalmax; cal++)
		{
			int cnt = 0;
			double yi = 0.0;
			for (int k=0; k<5; k++)
			{
				DecodePixel(data, pos, pix);
				if (pix.n > 0) { yi += pix.p; cnt++; }
			}
			y[dpos++] = (cnt > 0) ? yi/cnt : 0.0;
		}
	} catch (int e)
	{
		printf("Read error %i\n", e);
		if (data.size() > 5) for (int i=0; i<=5; i++) printf(" %04X", int(data[i]));
		printf("\n");
		return true;
	}

	PlotData("ADC scan", "Vcal", "ADC value", double(vcalmin), double(vcalmax), y);

	return true;
}
*/

CMD_PROC(deser160)
{
	tb.Daq_Open(1000);
	tb.Pg_SetCmd(20, PG_RESR);
	tb.Pg_SetCmd(0, PG_TOK);

	vector<uint16_t> data;

	int x, y;
	printf("      0        1        2        3        4        5        6        7\n");
	for (y = 0; y < 20; y++)
	{
		printf("%2i:", y);
		for (x = 0; x < 8; x++)
		{
			tb.Daq_Select_Deser160(x);
			tb.Sig_SetDelay(SIG_CLK,  y);
			tb.Sig_SetDelay(SIG_SDA,  y+15);
			tb.Sig_SetDelay(SIG_CTR,  y);
			tb.Sig_SetDelay(SIG_TIN,  y+5);
			tb.uDelay(100);

			tb.Daq_Start();
			tb.Pg_Single();
			tb.uDelay(50);
			tb.Daq_Stop();
			tb.Daq_Read(data, 10);
			if (data.size())
			{
				int h = data[0] & 0xffc;
				if (h == 0x7f8)
					printf(" <%03X>", int(data[0] & 0xffc));
				else
					printf("  %03X ", int(data[0] & 0xffc));

				if (data.size()<10)
					printf("[%u]", (unsigned int)(data.size()));
				else
					printf("[*]");
			}
			else printf("  ... ");
		}
		printf("\n");
	}
	tb.Daq_Close();

	return true;
}


/*
0000 I2C data
0001 I2C address
0010 I2C pixel column
0011 I2C pixel row

1000 VD unreg
1001 VA unreg
1010 VA reg
1100 IA

{ rocaddr[3:0], sana, s[2:0], data[7:0] }

*/

/*
CMD_PROC(readback)
{
	int i;

	tb.Daq_Open(100);
	tb.Pg_Stop();
	tb.Pg_SetCmd(0, PG_TOK);
	tb.Daq_Select_Deser160(settings.deser160_tinDelay);

	// --- take data
	tb.Daq_Start();
	for (i=0; i<36; i++)
	{
		tb.Pg_Single();
		tb.uDelay(20);
	}
	tb.Daq_Stop();

	// read out data
	vector<uint16_t> data;
	tb.Daq_Read(data, 40);
	tb.Daq_Close();

	//analyze data
	int pos = 0;
	PixelReadoutData pix;

	unsigned int value = 0;
	try
	{
		// find start bit
		do
		{
			DecodePixel(data, pos, pix);
		} while ((pix.hdr & 2) == 0);

		// read data
		for (i=0; i<16; i++)
		{
			DecodePixel(data, pos, pix);
			value = (value << 1) + (pix.hdr & 1);
		}

	} catch (int e)
	{
		printf("error %i (size=%i, pos=%i)\n", e, int(data.size()), pos);
		return true;
	}

	int roc = (value >> 12) & 0xf;
	int cmd = (value >>  8) & 0xf;
	int x = value & 0xff;

	printf("%04X: roc[%i].", value, roc);
	switch (cmd)
	{
	case  0: printf("I2C_data = %02X\n", x); break;
	case  1: printf("I2C_addr = %02X\n", x); break;
	case  2: printf("col = %02X\n", x); break;
	case  3: printf("row = %02X\n", x); break;
	case  8: printf("VD_unreg = %02X\n", x); break;
	case  9: printf("VA_unreg = %02X\n", x); break;
	case 10: printf("VA_reg = %02X\n", x); break;
	case 12: printf("IA = %02X\n", x); break;
	default: printf("? = %04X\n", value); break;
	}

	return true;
}
*/
