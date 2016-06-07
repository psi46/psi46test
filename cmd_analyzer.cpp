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
			return;
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
			return;
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
			return;
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
}


CMD_PROC(showrocdata)
{
	const unsigned int nSamples = 30;

	unsigned int i, k;
	vector<uint16_t> data[20];

	tb.Pg_Stop();
	tb.Pg_SetCmd( 0, PG_RESR + 10);
	tb.Pg_SetCmd( 0, PG_SYNC|PG_TOK);

	tb.SignalProbeD1(9);
	tb.SignalProbeD2(17);
	tb.SignalProbeA2(PROBEA_SDATA1);
	tb.uDelay(10);
	tb.SignalProbeADC(PROBEA_SDATA1, GAIN_4);
	tb.uDelay(10);

	tb.Daq_Select_ADC(nSamples, 1, 1);
	tb.uDelay(1000);
	tb.Daq_Open(1024);
	for (i=0; i<20; i++)
	{
		tb.Sig_SetDelay(SIG_CLK, 20-i);
		tb.Sig_SetDelay(SIG_CTR, 20-i);
		tb.Sig_SetDelay(SIG_TIN, 25-i);
		tb.uDelay(10);
		tb.Daq_Start();
		tb.Pg_Single();
		tb.uDelay(1000);
		tb.Daq_Stop();
		tb.Daq_Read(data[i], 1024);
		if (data[i].size() != nSamples)
		{
			printf("Data size %i: %i\n", i, int(data[i].size()));
			return;
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
	Scope("SDATA1", values);
}



// =======================================================================
//  experimental ROC test commands
// =======================================================================

CMD_PROC(vectortest)
{
	int length;
	PAR_INT(length, 0, 1048576);
	
	// test vectors
	vector<uint16_t> in;
	vector<uint16_t> out;

	// fill in vector
	in.reserve(length);
	for (int i=0; i<length; i++) in.push_back(i);

	tb.VectorTest(in, out);

	// compare vectors
}



CMD_PROC(daqtest)
{
	const int blklen = 220;
	// setup pg data sequence (0, 1, 2, ... 99)
	tb.SignalProbeD1(PROBE_PG_SYNC);
	tb.Pg_SetCmd(0, PG_SYNC + PG_RESR + 1);
	for (int k=1; k<blklen; k++) tb.Pg_SetCmd(k, PG_TOK + 1);
	tb.Pg_SetCmd(blklen, PG_TOK);

	tb.Daq_Open(80000000, 0);
	tb.Daq_Select_Datagenerator(0);
	tb.Daq_Start(0);

	int errors = 0;
	int i = 0;
	do
	{
		// create data
//		tb.Daq_Start(0);
		for (int k=0; k<180000; k++)
		{
			tb.Pg_Single();
			tb.uDelay(8); // 250
		}
//		tb.Daq_Stop(0);
		printf("%4i: created %u -> ", i, tb.Daq_GetSize());

		uint32_t n;
		uint32_t cnt = 0;
		uint32_t error_cnt = 0;
		vector<uint16_t> data;
		try
		{
			int dvalue = 0;
			do
			{
				tb.Daq_Read(data, 32768, n, 0);
				// check data
				for (unsigned int k=0; k<data.size(); k++)
				{
					if (data[k] != dvalue) error_cnt++;
					dvalue = (dvalue+1)%blklen;
				}
				cnt += data.size();
			} while (n != 0);
		}
		catch (DataPipeException e)
		{
			printf("\nERROR: %s\n", e.what());
			errors++;
		}
		
		printf("#samples=%u; #errors=%u\n", cnt, error_cnt);
		if (error_cnt) errors++;
		i++;
	} while (!keypressed());

	printf("#block errors %i\n", errors);

	tb.Daq_Close(0);
}


CMD_PROC(daqtest2)
{
	const int blklen = 100;
	// setup pg data sequence (0, 1, 2, ... 99)
	tb.SignalProbeD1(PROBE_PG_SYNC);
	tb.Pg_SetCmd(0, PG_SYNC + PG_RESR + 1);
	for (int k=1; k<blklen; k++) tb.Pg_SetCmd(k, PG_TOK + 1);
	tb.Pg_SetCmd(blklen, PG_TOK);

	tb.Daq_Open(80000000, 0);
	tb.Daq_Select_Datagenerator(0);
	tb.Daq_Start(0);

	// create continous data
	tb.Pg_Loop(520);

	uint32_t cnt = 0;
	uint32_t error_sum = 0;
	int dvalue = 0;
	vector<uint16_t> data;
	do
	{
		uint32_t error_cnt = 0;
		uint32_t n, n_remaining;
		try
		{
			int ret;
			ret = tb.Daq_Read(data, 1000000, n_remaining, 0);
			if (ret) { printf("overflow %i\n", ret); break; }
			// check data
			n = data.size();
			for (unsigned int k=0; k<n; k++)
			{
				if (data[k] != dvalue) error_cnt++;
				dvalue = (dvalue+1)%blklen;
			}
			error_sum += error_cnt;
			cnt += n;
		}
		catch (DataPipeException e)
		{
			printf("\nERROR: %s\n", e.what());
		}
		
		printf("total=%8u;  remaining=%8u;  block=%8u;  #errors=%u\n", cnt, n_remaining, n, error_cnt);
	} while (!keypressed());

	tb.Pg_Stop();
	tb.Daq_Stop(0);
	tb.Daq_Close(0);
	printf("\ntotal=%8u;  total errors=%u\n", cnt, error_sum);
}



// === Module error rate test

class CEventCounter : public CAnalyzer
{
	CEvent* Read();
public:
	unsigned int nEvents;
	unsigned int nPixels;
	unsigned int nErrors;
	void Reset() { nEvents = nPixels = nErrors = 0; }
	CEventCounter() { Reset(); }
	void Print();
};

CEvent* CEventCounter::Read()
{
	x = Get();
	nEvents++;
	if (x->error) nErrors++;
	for (unsigned int r = 0; r < x->roc.size(); r++)
		nPixels += x->roc[r].pixel.size();
	return x;
}


void CEventCounter::Print()
{
	int xorbyte = tb.Deser400_GetXor(0);
	int ph  = tb.Deser400_GetPhase(0);
	printf("nEvents: %u;  nPixels: %u;  nErrors: %u  xor=%02X, ph=%i\n", nEvents, nPixels, nErrors, xorbyte, ph);
}


class CEventMap : public CAnalyzer
{
	CEvent* Read();
public:
	unsigned int nWrongRocCount;
	unsigned int nWrongAddress;
	unsigned int map[8][52][80];
	void Reset();
	void Report();
	CEventMap() { Reset(); }
};

void CEventMap::Reset()
{
	unsigned int r, x, y;
	for (r=0; r<8; r++) for (x=0; x<52; x++) for (y=0; y<80; y++) map[r][x][y] = 0;
	nWrongRocCount = nWrongAddress = 0;
}

void CEventMap::Report()
{
	Log.section("PIXELMAP");
	Log.printf("Errors: RocCount=%u, Address=%u\n", nWrongRocCount, nWrongAddress);
	int r, x, y;
	for (r=0; r<8; r++)
	{
		Log.printf("ROC %i\n", r);
		for (y=51; y>=0; y--)
		{
			Log.printf("%2i: ", y);
			for (x=0; x<52; x++)
			{
				unsigned int n = map[r][x][y];
				if (n == 0)           Log.printf("   .");
				else if (n < 1000)    Log.printf(" %3u", n);
				else if (n < 1000000) Log.printf("%3uk", n/1000);
				else                  Log.printf("%3uM", n/10000000);
			}
			Log.printf("\n");
		}
	}
}

CEvent* CEventMap::Read()
{
	x = Get();
	bool error = false;
	if (x->roc.size() == 8)
	{
		for (unsigned int r = 0; r < x->roc.size(); r++)
		{
			unsigned int nP = x->roc[r].pixel.size();
			for (unsigned int p = 0; p < nP; p++)
			{
				unsigned int px = x->roc[r].pixel[p].x;
				unsigned int py = x->roc[r].pixel[p].y;
				if (px < 52 && py < 80) map[r][px][py]++;
				else { nWrongAddress++; error = true; }
			}
		}
	}
	else { nWrongRocCount++; error = true; }

	if (error) x->error |= 0x0800;
	return x;
}


CMD_PROC(daqerrorcheckm)
{ PROFILING
	int period;
	int channel;
	PAR_INT(channel, 0, 8);
	PAR_INT(period,0,0xffffff)

	CDtbSource src;  src.Logging(false);
	CDataRecordScannerMODD rec;
	CModDigDecoder decoder;
	CEventPrinter evList("error_report.txt");
	 evList.ListOnlyErrors(true);
	CEventCounter counter;
	CSink<CEvent*> pump;

	src >> rec >> decoder >> evList >> counter >> pump;

	src.OpenModDig(tb, channel, true, 20000000);
	src.Enable();
	tb.uDelay(100);

	tb.Pg_Loop(period);
//	tb.Trigger_SetGenPeriodic(period);
//	tb.Trigger_SetGenRandom(period);
//	tb.Trigger_Select(TRG_SEL_GEN_DIR);

	try
	{
		int i=0;
		while (!keypressed())
		{
			pump.Get();
			if (i++ % 1000 == 0)
			{
				counter.Print();
				printf(" Buffer: %u\n", src.GetRemainingSize());
			}
		}
		tb.Pg_Stop();
//		pxmap.Report();
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

//	printf("Bytes Transfered: %u\n", srcdump.ByteCount());
	printf("\n");
	src.Disable();
}


CMD_PROC(desergatescan)
{ PROFILING

	tb.Deser400_GateStop();
	tb.Deser400_Enable(0);

	Log.section("GATE_SCAN");

	for (int i=0; i<8; i++)
	{
		Log.printf("%i: ", i);
		for (int k=0; k<40; k++)
		{
			tb.Deser400_GateSingle(i);
			tb.mDelay(10);
			unsigned int xorbyte = tb.Deser400_GetXor(0);
			Log.printf(" %02X", xorbyte);
		}
		Log.printf("\n");
	}
	Log.flush();
}


CMD_PROC(daqreadm)
{ PROFILING
	int period;
	int channel;
	PAR_INT(channel, 0, 8);
	PAR_INT(period,0,65535)

	CDtbSource src;  src.Logging(false);
	CStreamDump srcdump("xxx_stream.txt");
	CDataRecordScannerMODD rec;
	CRocRawDataPrinter rawList("xxx_raw.txt");
	CModDigDecoder decoder;
//	CEventMap pxmap;
	CEventPrinter evList("xxx_event.txt");
//	evList.ListOnlyErrors(true);
	CReadbackLogger rdb("xxx_readback.txt");
	CEventCounter counter;
	CSink<CEvent*> pump;

	src >> srcdump >> rec >> rawList >> decoder >> /* pxmap >> */ evList >> rdb >> counter >> pump;

	src.OpenModDig(tb, channel, true, 20000000);
	src.Enable();
	tb.uDelay(100);
	tb.Pg_Loop(period);

	try
	{
		int i=0;
		while (i++ < 500000 && !keypressed())
		{
			pump.Get();
			if (i % 1000 == 0) counter.Print();
		}
		tb.Pg_Stop();
//		pxmap.Report();
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

//	printf("Bytes Transfered: %u\n", srcdump.ByteCount());
	printf("\n");
	src.Disable();
}


CMD_PROC(daqreadt)
{ PROFILING
	int period;
	PAR_INT(period,0,65535)

	CDtbSource src;  src.Logging(false);
	CStreamDump srcdump("stream_soft_tbm.txt");
	CSink<uint16_t> pump;

	src >> srcdump >> pump;
	src.OpenRocDig(tb, settings.deser160_tinDelay, true, 1000000);
	src.Enable();
	tb.uDelay(100);
	tb.Pg_Loop(period);
	tb.Trigger_SetGenPeriodic(period);
	tb.Trigger_Select(TRG_SEL_ASYNC);

	try
	{
		int i=0;
		while (i++ < 50000 && !keypressed())
		{
			pump.Get();
		}
		tb.Trigger_Select(0);
//		pxmap.Report();
	}
	catch (DS_empty &) { printf("finished\n"); }
	catch (DataPipeException &e) { printf("%s\n", e.what()); }

//	printf("Bytes Transfered: %u\n", srcdump.ByteCount());
	printf("\n");
	src.Disable();
}


/*
class CDemoAnalyzer : public CAnalyzer
{
	CEvent* Read();
};

CEvent* CDemoAnalyzer::Read()
{
	CEvent* ev = Get();

	Log.printf("Header: %03X\n", int(ev->header));
	for (unsigned int i=0; i<ev->roc[0].pixel.size(); i++)
		Log.printf("[%3i %3i %3i]\n", ev->roc[0].pixel[i].x, ev->roc[0].pixel[i].y, ev->roc[0].pixel[i].ph);
	Log.printf("\n");
	return ev;
};


class CDemoAnalyzer2 : public CAnalyzer
{
	CEvent* Read();
};


CEvent* CDemoAnalyzer2::Read()
{
	CEvent* ev = Get();

	Log.printf("%03X ", int(ev->header));
	for (unsigned int i=0; i<ev->roc[0].pixel.size(); i++)
		Log.printf(" %3i", ev->roc[0].pixel[i].ph);
	Log.printf("\n");
	return ev;
};
*/

// --- Level Histo

class CLevelHisto : public CAnalyzer
{
	unsigned int pos;
	unsigned int h[256];
	CEvent* Read();
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


CEvent* CLevelHisto::Read()
{
	CEvent* ev = Get();
	if (ev->roc[0].pixel.size() > pos)
	{
		unsigned int x = ev->roc[0].pixel[pos].ph;
		if (x < 256) h[x]++;
	}
	return ev;
};


// ---
class CPrint : public CAnalyzer
{
	CEvent* Read();
};


CEvent* CPrint::Read()
{
	CEvent* ev = Get();
	for (unsigned int i = 0; i < ev->roc[0].pixel.size(); i++)
	{
		printf(" %3u", ev->roc[0].pixel[i].ph);
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
	CDataRecordScannerROC rec;
	CRocRawDataPrinter rawList("raw.txt");

//	CSink<CDataRecord*> pump;

	CRocDigDecoder decoder;
	CEventPrinter evList("eventlist.txt");
	CSink<CEvent*> pump;
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
}


CMD_PROC(ethsend)
{
	char msg[45];
	PAR_STRINGEOL(msg,45);
	string message = msg;
	tb.Ethernet_Send(message);
	DO_FLUSH
}

CMD_PROC(ethrx)
{
	unsigned int n = tb.Ethernet_RecvPackets();
	printf("%u packets received\n", n);
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


void Scan1D(CDtbSource &src, CSink<CEvent*> &data, int vx, int xmin, int xmax, int xstep)
{
	int x, k;

	// --- take data
	src.Enable();
	for (x = xmin; x<=xmax; x++)
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
				CEvent *ev = data.Get();
				if (ev->roc[0].pixel.size() != 0) count++;
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
	if (count < 1 || count > 256) return;

	Log.section("SHMOO",false);
	Log.printf("regX(%i)=%i:%i;  regY(%i)=%i:%i\n",
		vx, xmin, xmax, vy, ymin, ymax);

	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigDecoder dec;
	CSink<CEvent*> data;
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
}


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
}
*/





// === DROC600 Test =========================================================

CMD_PROC(phscan)
{ PROFILING

	int col;
	int row;
	int vcalmin;
	int vcalmax;

	PAR_INT(col, 0, 51)
	PAR_INT(row, 0, 79)
	PAR_RANGE(vcalmin, vcalmax, 0, 255)

	Log.section("PHSCAN1", false);
	Log.printf("(%2i/%2i)\n", col, row);

	tb.Pg_Stop();
	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 100);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);

	CDtbSource src;
	CDataRecordScannerROC raw;
//	CRocRawDataPrinter rawPrint("raw.txt");
	CRocDigDecoder dec;
//	CEventPrinter evPrint("event.txt");
	CSink<CEvent*> data;
	src >> raw /* >> rawPrint */ >> dec /* >> evPrint */ >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 400000);
	src.Enable();
	tb.uDelay(100);

	// --- scan vcal
	tb.roc_ClrCal();
	tb.roc_Col_Enable(col, true);
	tb.roc_Pix_Trim(col, row, 15);
	tb.roc_Pix_Cal (col, row, false);

	for (int cal = vcalmin; cal < vcalmax; cal++)
	{
		tb.roc_SetDAC(Vcal, cal);
		tb.uDelay(100);
		for (int k=0; k<20; k++)
		{
			tb.Pg_Single();
			tb.uDelay(40);
		}
	}

	tb.roc_Pix_Mask(col, row);
	tb.roc_Col_Enable(col, false);
	tb.roc_ClrCal();

	src.Disable();

	// --- plot data
	try
	{
		for (int cal = vcalmin; cal < vcalmax; cal++)
		{
			int cnt = 0;
			double yi = 0.0;
			double yi2 = 0.0;
			for (int k=0; k<20; k++)
			{
				CEvent *ev = data.Get();
				if (ev->roc[0].pixel.size() > 0)
				{
					double ph = ev->roc[0].pixel[0].ph;
					cnt++;
					yi  += ph;
					yi2 += ph*ph;
				}
			}
			if (cnt > 0)
			{
				Log.printf("%3i %5.1f %5.1f\n", cal, yi/cnt, sqrt((yi2-yi*yi/cnt)/cnt));
			}
			else Log.printf("%3i\n", cal);
		}
	} catch (DataPipeException e) { printf("\nERROR test_pulseheight: %s\n", e.what()); }

	Log.flush();
}


const char *i2bin(int x)
{
	static char s[18];

	for (int i=0; i<7; i++)
	{
		s[i] = (x & 0x40) ? '1' : '0';
		x <<= 1;
	}
	s[7] = 0;
	return s;
}


CMD_PROC(scanaddr)
{ PROFILING
	Log.section("SCANADDR");
	tb.Pg_Stop();
	tb.roc_Chip_Mask();

	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 100000);
	src.Enable();

	// --- scan all pixel ------------------------------------------------------
	unsigned char col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.roc_Col_Enable(col, true);
		tb.uDelay(10);
		for (row=0; row<ROC_NUMROWS; row++)
		{
			tb.roc_Pix_Cal (col, row,   false);  // cluster lower
//			tb.roc_Pix_Cal (col, row+1, false);  // cluster upper
			tb.roc_Pix_Trim(col, row,   15);
//			tb.roc_Pix_Trim(col, row+1, 15);
			tb.uDelay(20);
			tb.Pg_Single();
			tb.uDelay(20);

			tb.roc_Pix_Mask(col, row);
			tb.roc_ClrCal();
		}
		tb.roc_Col_Enable(col, false);
		tb.uDelay(10);
	}
	src.Disable();


	// --- analyze data --------------------------------------------------------
	// for each col, for each row
	int r[ROC_NUMCOLS][ROC_NUMROWS];
	int x[ROC_NUMCOLS][ROC_NUMROWS];
	int y[ROC_NUMCOLS][ROC_NUMROWS];

	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)
		{
			for (row=0; row<ROC_NUMROWS; row++)
			{
				CEvent *ev = data.Get();
				if (ev->roc[0].pixel.size() >= 1)
				{
					r[col][row] = ev->roc[0].pixel[0].raw;
					x[col][row] = ev->roc[0].pixel[0].x;
					y[col][row] = ev->roc[0].pixel[0].y;
				} else x[col][row] = -1;
			}
		}
	} catch (DataPipeException e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	src.Close();

	int addrDefect = 0;
	for (row=0; row<ROC_NUMROWS; row++)
	{
		Log.printf("%2i", row);
		for (col=0; col<ROC_NUMCOLS; col++)
		{
			if (x[col][row] >= 0)
			{
//				Log.printf(" %06X(%2i/%s)", r[col][row], x[col][row], i2bin(y[col][row]));
//				Log.printf(" (%2i/%s)", x[col][row], i2bin(y[col][row]));
				bool addrOk = (x[col][row] == col) && (y[col][row] == row);
				if (!addrOk) addrDefect++;
				Log.printf("(%2i%c%2i)", x[col][row], addrOk ? '/' : '*',  y[col][row]);
			}
			else Log.printf(" ------");
		}
		Log.printf("\n");
	}
	Log.printf("%i addresses defect\n", addrDefect);
	Log.flush();
}


CMD_PROC(multiread)
{ PROFILING
	int nReadouts;
	PAR_INT(nReadouts, 0, 100000);

	Log.section("MULTIREAD");

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);
	src.Enable();

	// --- scan all pixel ---------------------------------------------------
	int i;
	for (i=0; i<nReadouts; i++)
	{
		tb.Pg_Single();
		tb.uDelay(1000);
	}
	src.Disable();


	// --- analyze data -----------------------------------------------------

	int evCnt  = 0;
	int errCnt = 0;

	try
	{
		while (!keypressed())
		{
			CEvent *ev = data.Get(); evCnt++;
			Log.printf("%5i: <%3u>", evCnt, ev->roc[0].pixel.size());
			for (unsigned int k = 0; k < ev->roc[0].pixel.size(); k++)
			{
				int x = ev->roc[0].pixel[k].x;
				int y = ev->roc[0].pixel[k].y;
				int a = ev->roc[0].pixel[k].ph;
//				Log.printf (" (%2i/%s%c%3i)", x, i2bin(y), ev->roc[0].error ? '*' : '/', a);
				Log.printf (" (%2i%c%2i/%3i)", x, ev->roc[0].error ? '*' : '/', y, a);
//				Log.printf ("  %3i %3i", y, a); 
				if (ev->roc[0].error) errCnt++;
			}
			Log.puts("\n");
		}
	} 
	catch (DS_empty &) { Log.printf("--- end of data ---\n"); }
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }

	printf("%i events read with %i errors.\n", evCnt, errCnt);

	Log.flush();

	src.Close();
}


CMD_PROC(cluster)
{ PROFILING
	int nReadouts;
	PAR_INT(nReadouts, 0, 100000);
	const int offset = 2;

	Log.section("MULTIREAD");

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);
	
	unsigned int evCnt = 0;
	unsigned int j, k;

	for (j=0; j<78; j++)
	{
		tb.roc_Pix_Trim(0, j,   15);
		tb.roc_Pix_Trim(1, j+offset, 15);
		tb.roc_Pix_Cal(0, j);
		tb.roc_Pix_Cal(1, j+offset);
		tb.uDelay(500);
		Log.printf("----- (0/%i) (1/%i)\n", j, j+offset);

		// --- readout ---------------------------------------------------
		src.Enable();
		for (int i=0; i<nReadouts; i++)
		{
			tb.Pg_Single();
			tb.uDelay(100);
		}
		src.Disable();

		// --- analyze data -----------------------------------------------------
		try
		{
			while (!keypressed())
			{
				CEvent *ev = data.Get(); evCnt++;
				Log.printf("%5i:", evCnt);
				for (k = 0; k < ev->roc[0].pixel.size(); k++)
				{
					int x = ev->roc[0].pixel[k].x;
					int y = ev->roc[0].pixel[k].y;
					int a = ev->roc[0].pixel[k].ph;
					Log.printf (" (%2i/%s/%3i)", x, i2bin(y), a);
//					Log.printf ("  %3i %3i", y, a); 
				}
				Log.puts("\n");
			}
		} 
		catch (DS_empty &) { Log.printf("--- end of data ---\n"); }
		catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	
		tb.roc_Pix_Mask(0, j);
		tb.roc_Pix_Mask(1, j+offset);
		tb.roc_ClrCal();
	}

	Log.flush();

	src.Close();
	printf("%i events read.\n", evCnt);
}


CMD_PROC(cluster2)
{ PROFILING
	int nReadouts;
	PAR_INT(nReadouts, 0, 100000);
	const int offset = 0;

	Log.section("MULTIREAD");

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);
	
	unsigned int evCnt = 0;
	unsigned int j, k;

	for (j=0; j<78; j++)
	{
		tb.roc_Pix_Trim(0, j,   15);
		tb.roc_Pix_Trim(1, offset, 15);
		tb.roc_Pix_Cal(0, j);
		tb.roc_Pix_Cal(1, offset);
		tb.uDelay(500);
		Log.printf("----- (0/%i) (1/%i)\n", j, offset);

		// --- readout ---------------------------------------------------
		src.Enable();
		for (int i=0; i<nReadouts; i++)
		{
			tb.Pg_Single();
			tb.uDelay(100);
		}
		src.Disable();

		// --- analyze data -----------------------------------------------------
		try
		{
			while (!keypressed())
			{
				CEvent *ev = data.Get(); evCnt++;
				Log.printf("%5i:", evCnt);
				for (k = 0; k < ev->roc[0].pixel.size(); k++)
				{
					int x = ev->roc[0].pixel[k].x;
					int y = ev->roc[0].pixel[k].y;
					int a = ev->roc[0].pixel[k].ph;
					Log.printf (" (%2i/%s/%3i)", x, i2bin(y), a);
//					Log.printf ("  %3i %3i", y, a); 
				}
				Log.puts("\n");
			}
		} 
		catch (DS_empty &) { Log.printf("--- end of data ---\n"); }
		catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	
		tb.roc_Pix_Mask(0, j);
		tb.roc_Pix_Mask(1, offset);
		tb.roc_ClrCal();
	}

	Log.flush();

	src.Close();
	printf("%i events read.\n", evCnt);
}


CMD_PROC(db1)
{ PROFILING
	Log.section("DB1");

	int evCnt = 0;
	int k;

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 10000000);
	
	// --- readout ---------------------------------------------------
	src.Enable();
	tb.Pg_Single();
	tb.uDelay(1000);
	src.Disable();

	// --- analyze data -----------------------------------------------------
	try
	{
		while (!keypressed())
		{
			CEvent *ev = data.Get(); evCnt++;

			int nPx = ev->roc[0].pixel.size();
			Log.printf("%5i<%3i>:", evCnt, nPx);

			for (k = 0; k < nPx; k++)
			{
				int x = ev->roc[0].pixel[k].x;
				int y = ev->roc[0].pixel[k].y;
				int a = ev->roc[0].pixel[k].ph;
//				Log.printf (" (%2i/%s/%3i)", x, i2bin(y), a);
				Log.printf(" (%2i/%2i)", x, y /* & 0x3f */ );
				if ((k % 14) == 13) Log.puts("\n           ");
			}
			Log.puts("\n");
		}
	} 
	catch (DS_empty &) { Log.printf("--- end of data ---\n"); }
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	
	Log.flush();

	src.Close();
	printf("%i events read.\n", evCnt);
}


// =================================


class CPulse
{
	unsigned int pn;
	double px;
	double px2;
	double mean;
	double stdev;
public:
	void Clear() { pn = 0; px = px2 = 0.0; }
	void Add(unsigned int ph) { px += ph; px2 += ph*ph; pn++; }
	void Update();
	double GetCount() { return pn; }
	double GetMean() { return mean; }
	double GetStdev() { return stdev; }
	void Print();
};


void CPulse::Update()
{
	if (pn)
	{
		mean = px/pn;
		stdev = sqrt(px2/pn - mean*mean);
	}
}

void CPulse::Print()
{
	if (pn) Log.printf(" (%3u %5.1f %5.1f)", pn, mean, stdev);
	else    Log.printf(" (  0            )");
}


class CPulseMap
{
	unsigned int pn;
public:
	CPulse ph[52][80];
	void Clear();
	void Add(CRocPixel &p);
	void Update();
	void Print();
	void Print(int xmin, int xmax, int ymin=0, int ymax=79);
	void List();
};


void CPulseMap::Clear()
{
	unsigned int x, y;
	for (x=0; x<52; x++) for (y=0; y<80; y++) ph[x][y].Clear();
	pn = 0;
}

void CPulseMap::Add(CRocPixel &p)
{
	if (p.x < 52 && p.y < 80)
	{
		ph[p.x][p.y].Add(p.ph);
		pn++;
	}
}

void CPulseMap::Update()
{
	unsigned int x, y;
	for (x=0; x<52; x++) for (y=0; y<80; y++) ph[x][y].Update();
}

void CPulseMap::Print()
{
	int x, y;
	for (y=79; y>=0; y--)
	{
		for (x=0; x<52; x++) ph[x][y].Print();
		Log.puts("\n");
	}
}

void CPulseMap::Print(int xmin, int xmax, int ymin, int ymax)
{
	int x, y;
	for (y=ymax; y>=ymin; y--)
	{
		for (x=xmin; x<=xmax; x++) ph[x][y].Print();
		Log.puts("\n");
	}
}

void CPulseMap::List()
{
	int x, y;
	for (x=0; x<52; x++)
	{
		for (y=0; y<80; y++) if (ph[x][y].GetCount())
		{
			Log.printf("(%2u/%3u):", x, y);
			ph[x][y].Print();
			Log.puts("\n");
		}
	}
	Log.printf("%u pixel hits\n", pn);
}


CMD_PROC(scanphxy)
{ PROFILING
	int nReadouts;
	int xmin, xmax, ymin, ymax;
	PAR_RANGE(xmin, xmax, 0, 51)
	PAR_RANGE(ymin, ymax, 0, 79)
	if (!PAR_IS_INT(nReadouts, 0, 10000)) nReadouts = 1;

	Log.section("SCANPHXY");

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);

	// --- scan all pixel ---------------------------------------------------
	int x, y, k;
	src.Enable();
	for (x=xmin; x<=xmax; x++)
	{
		tb.roc_Col_Enable(x, true);
		for (y=ymin; y <=ymax; y++)
		{
			tb.roc_Pix_Trim(x, y, 15);
			tb.roc_Pix_Cal(x, y);
			tb.uDelay(50);
			for (k=0; k<nReadouts; k++)
			{
				tb.Pg_Single();
				tb.uDelay(50);
			}
			tb.roc_Pix_Mask(x, y);
			tb.roc_ClrCal();
		}
	}
	src.Disable();


	// --- analyze data -----------------------------------------------------
	CPulseMap *map = new CPulseMap;
	map->Clear();

	try
	{
		while (true)
		{
			CEvent *ev = data.Get();
			for (k = 0; k < int(ev->roc[0].pixel.size()); k++)
			{
				map->Add(ev->roc[0].pixel[k]);
			}
		}
	} 
	catch (DS_empty &) {}
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }

	map->Update();
	map->Print(xmin, xmax, ymin, ymax);
	
	// --- statistics
	int channel_cnt[4];
	double channel[4];
	for (k=0; k<4; k++) { channel_cnt[k] = 0; channel[k] = 0.0; }
	for (x=xmin; x<= xmax; x++) for (y=ymin; y<=ymax; y++)
	{
		int grp = (x%2) + 2*(y%2);
		channel[grp] += map->ph[x][y].GetMean();
		channel_cnt[grp]++;
	}
	for (k=0; k<4; k++)
	{
		if (channel_cnt[k]) Log.printf("channel %i: %5.1f  %i\n", k, channel[k]/channel_cnt[k], channel_cnt[k]);
		else Log.printf("channel %i:\n", k);
	}


	delete map;
	Log.flush();
	src.Close();
}

CMD_PROC(scanph)
{ PROFILING
	int nReadouts;
	PAR_INT(nReadouts, 1, 100000)

	Log.section("SCANPH");

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);

	// --- scan pixel -------------------------------------------------------
	int k;
	src.Enable();
	for (k=0; k<nReadouts; k++)
	{
		tb.Pg_Single();
		tb.uDelay(500);
	}
	src.Disable();

	// --- analyze data -----------------------------------------------------
	CPulseMap *map = new CPulseMap;
	map->Clear();

	try
	{
		while (true)
		{
			CEvent *ev = data.Get();
			for (k = 0; k < int(ev->roc[0].pixel.size()); k++)
			{
				 map->Add(ev->roc[0].pixel[k]);
			}
		}
	} 
	catch (DS_empty &) {}
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }

	map->Update();
	map->List();
	
	delete map;
	Log.flush();
	src.Close();
}


CMD_PROC(dbmatch)
{ PROFILING
	const int DBSIZE = 56;

	int nCycles;
	PAR_INT(nCycles, 1, 1000)

	Log.section("DBMATCH");

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);

	// --- scan pixel -------------------------------------------------------
	int n = nCycles*DBSIZE;
	int j, k;
	src.Enable();
	for (k=0; k<n; k++)
	{
		tb.Pg_Single();
		tb.uDelay(500);
	}
	src.Disable();

	// --- analyze data -----------------------------------------------------

	unsigned int db_cnt[DBSIZE];
	double db_ph[DBSIZE];
	double db_ph2[DBSIZE];
	for (j=0; j<DBSIZE; j++) { db_ph[j] = db_ph2[j] = 0.0; db_cnt[j] = 0; }

	try
	{
		for (k=0; k<nCycles; k++)
		{
			for (j=0; j<DBSIZE; j++)
			{
				CEvent *ev = data.Get();
				if (ev->roc.size() && ev->roc[0].pixel.size())
				{
					int ph = ev->roc[0].pixel[0].ph;
					db_ph[j] += ph;
					db_ph2[j] += ph*ph;
					db_cnt[j]++;
				}
			}
		}
	} 
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }

	for (j=0; j<DBSIZE; j++)
	{
		if (db_cnt[j])
		{
			db_ph[j] /= db_cnt[j];
			db_ph2[j] = sqrt(db_ph2[j]/db_cnt[j] - db_ph[j]*db_ph[j]);
		}
		else db_ph[j] = db_ph2[j] = 0.0;
	}

	for (j=0; j < DBSIZE; j++)
	{
		Log.printf("%2i  %5.1f  %5.1f %3u\n", j, db_ph[j], db_ph2[j], db_cnt[j]);
	}

	Log.flush();
	src.Close();
}


CMD_PROC(dbmatch2)
{ PROFILING
	const int DBSIZE = 56;

	int nCycles;
	PAR_INT(nCycles, 1, 1000)

	Log.section("DBMATCH2");

	int j, k;

	// --- construct PG sequence -------------------------------------------
	vector<uint16_t>sequence;
	sequence.push_back(PG_RESR|PG_SYNC + 20);
	for (j = 0; j < DBSIZE; j++)
	{
		sequence.push_back(PG_CAL + 100);
		sequence.push_back(PG_TRG +  20);
		sequence.push_back(PG_TOK + 100);
	}
	sequence.push_back(0);
	tb.Pg_SetCmdAll(sequence);

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);

	// --- scan pixel -------------------------------------------------------
	unsigned int db_cnt[DBSIZE];
	double db_ph[DBSIZE];
	double db_ph2[DBSIZE];
	for (j=0; j<DBSIZE; j++) { db_ph[j] = db_ph2[j] = 0.0; db_cnt[j] = 0; }

	src.Enable();
	try
	{
		for (k=0; k<nCycles; k++)
		{
			tb.Pg_Single();
			tb.uDelay(1000);
			for (j=0; j<DBSIZE; j++)
			{
				CEvent *ev = data.Get();
				if (ev->roc.size() && ev->roc[0].pixel.size())
				{
					int ph = ev->roc[0].pixel[0].ph;
					db_ph[j] += ph;
					db_ph2[j] += ph*ph;
					db_cnt[j]++;
				}
			}
		}
	}
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	src.Disable();
	src.Close();

	// --- analyze and print data -------------------------------------------
	for (j=0; j<DBSIZE; j++)
	{
		if (db_cnt[j])
		{
			db_ph[j] /= db_cnt[j];
			db_ph2[j] = sqrt(db_ph2[j]/db_cnt[j] - db_ph[j]*db_ph[j]);
		}
		else db_ph[j] = db_ph2[j] = 0.0;
	}

	for (j=0; j < DBSIZE; j++)
	{
		Log.printf("%2i  %5.1f  %5.1f %3u\n", j, db_ph[j], db_ph2[j], db_cnt[j]);
	}

	double db_even = 0.0;
	double db_odd  = 0.0;
	for (j=0; j<DBSIZE; j++)
	{
		if (j & 1) db_odd += db_ph[j]; else db_even += db_ph[j];
	}
	db_even /= DBSIZE/2;
	db_odd  /= DBSIZE/2;
	Log.printf("even/odd/diff: %5.1f  %5.1f  %5.1f\n", db_even, db_odd, db_even-db_odd);

	Log.flush();
}


CMD_PROC(evenodd)
{ PROFILING
	const int DBSIZE = 56;

	int nCycles = 10;
	int x0;
	int y0;
	PAR_INT(x0, 0, 51);
	PAR_INT(y0, 0, 1);

	Log.section("EVENODD", false);
	Log.printf(" (%i/%i)\n", x0, y0);

	int j, k;

	// --- construct PG sequence -------------------------------------------
	vector<uint16_t>sequence;
	sequence.push_back(PG_RESR|PG_SYNC + 20);
	for (j = 0; j < DBSIZE; j++)
	{
		sequence.push_back(PG_CAL + 100);
		sequence.push_back(PG_TRG +  20);
		sequence.push_back(PG_TOK + 100);
	}
	sequence.push_back(0);
	tb.Pg_SetCmdAll(sequence);

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);

	// --- scan pixel -------------------------------------------------------
	src.Enable();
	try
	{
		for (int y=y0; y<80; y+=2) // all pixel on same channel
		{
			unsigned int db_neven = 0;
			unsigned int db_nodd  = 0;
			double db_even = 0.0;
			double db_odd = 0.0;
			tb.roc_Pix_Cal(x0, y);
			tb.roc_Pix_Trim(x0, y, 15);
			tb.roc_Col_Enable(x0, true);
			tb.uDelay(100);

			for (k=0; k<nCycles; k++) // mean value over nCycles
			{
				tb.Pg_Single();
				tb.uDelay(1000);
				for (j=0; j<DBSIZE; j++) // all DB cells
				{
					CEvent *ev = data.Get();
					if (ev->roc.size() && ev->roc[0].pixel.size())
					{
						int ph = ev->roc[0].pixel[0].ph;
						if (j & 1)
						{
							db_odd += ph;
							db_nodd++;
						}
						else
						{
							db_even += ph;
							db_neven++;
						}
					}
				}
			}

			tb.roc_ClrCal();
			tb.roc_Pix_Mask(x0, y);
			tb.roc_Col_Enable(x0, false);

			if (db_nodd && db_neven)
			{
				db_even /= db_neven;
				db_odd  /= db_nodd;
				Log.printf("%2i: %5.1f  %5.1f  %5.1f %4u\n", y, db_even, db_odd, db_even-db_odd, db_neven+db_nodd);
			} else Log.printf("%2i:\n", y);
		}
	}
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	src.Disable();
	src.Close();

	Log.flush();
}


CMD_PROC(enapx)
{
	int x, y;
	tb.roc_Chip_Mask();
	while (PAR_IS_INT(x, 0, 55))
	{
		PAR_INT(y, 0, 79);
		tb.roc_Col_Enable(x, true);
		tb.roc_Pix_Trim(x, y, 15);
		tb.roc_Pix_Cal(x, y);
	}
}


// --- DB & readout test ----------

class CPgEvent
{
public:
	int type;
	int time;

	CPgEvent() {};
	CPgEvent(int t, int ev) : type(ev), time(t) {}
	bool operator <(const CPgEvent &b) { return time < b.time; }
};


class CPgEventList
{
	bool sorted, created;
	unsigned int nToken;
	list<CPgEvent> el;
	vector<uint16_t> pg_sequence;
	void Sort() { el.sort(); sorted = true; }
	void Add(const CPgEvent &ev);
	bool ReadLine(const char *s);
	void Clear();
public:
	CPgEventList() : sorted(false), created(false), nToken(0) {}

	void Add_RESR(int t) { Add(CPgEvent(t, PG_RESR)); }
	void Add_REST(int t) { Add(CPgEvent(t, PG_REST)); }
	void Add_SYNC(int t)  { Add(CPgEvent(t, PG_SYNC)); }
	void Add_CAL(int t, int wbc = 0)
	{ Add(CPgEvent(t, PG_CAL)); if (wbc) Add_TRG(t+wbc); }
	void Add_TRG(int t)  { Add(CPgEvent(t, PG_TRG)); }
	void Add_TOK(int t)  { Add(CPgEvent(t, PG_TOK)); }
	bool Read(const char *filename);
	bool Check();
	unsigned int GetTokenCnt() { return nToken; }
	void Load();
	void Print();
};

void CPgEventList::Clear()
{
	sorted = created = false;
	nToken = 0;
	el.clear();
	pg_sequence.clear();
}

void CPgEventList::Add(const CPgEvent &ev)
{
	sorted = created = false;
	list<CPgEvent>::iterator i = el.begin();
	while (i != el.end())
	{
		if (ev.time == i->time)
		{
			i->type |= ev.type;
			return;
		}
		i++;
	}
	el.push_back(ev);
}

bool CPgEventList::ReadLine(const char *s)
{
/*
	RESR 100
	REST 110
	SYNC 110
	CAL  200
	CAL  200 100
	TRG  210
	TOK  2
*/
	char ev[16];
	int t, t2;
	int n = sscanf(s, "%14s %i %i", ev, &t, &t2);

	if (n <= 0) return true; // empty line
	if (ev[0] == '-') return true; // comment
	if (n < 2) return false; // missing parameter
	if (n < 3) t2 = 0; // 0 if 2nd parameter not exists

	if      (!strcmp(ev, "CAL" )) Add_CAL(t,t2);
	else if (!strcmp(ev, "TRG" )) Add_TRG(t);
	else if (!strcmp(ev, "TOK" )) Add_TOK(t);
	else if (!strcmp(ev, "SYNC")) Add_SYNC(t);
	else if (!strcmp(ev, "RESR")) Add_RESR(t);
	else if (!strcmp(ev, "REST")) Add_REST(t);
	else return false;

	return true;
}


bool CPgEventList::Read(const char *filename)
{
	Clear();
	FILE *f = fopen(filename, "rt");
	if (!f) return false;

	char s[256];

	while (!feof(f))
	{
		if (fgets(s, 254, f))
		{
			if (!ReadLine(s)) { Clear(); fclose(f);  return false; }
		}
	}
	fclose(f);
	return Check();
}


bool CPgEventList::Check()
{
	if (created) return true;
	if (!sorted) Sort();
	pg_sequence.clear();
	created = false;
	nToken  = 0;

	unsigned int tk = 0;
	list<CPgEvent>::iterator i = el.begin();
	while (i != el.end())
	{
		list<CPgEvent>::iterator k = i; k++;
		if (k == el.end())
		{
			pg_sequence.push_back(i->type);
			created = true;
			break;
		}

		int t = k->time - i->time;
		if (t > 1)
		{
			int dt = (t <= 255)? t : 255;
			pg_sequence.push_back(i->type + dt-1); t -= dt;
			while (t > 0)
			{
				dt = (t <= 255)? t : 255;
				pg_sequence.push_back(dt); t -= dt;
				if (pg_sequence.size() > 256) break;
			}
		}
		else
		{
			pg_sequence.clear();
			created = false;
			break;
		}
		i = k;
	}
	if (!created || pg_sequence.size() > 256) { pg_sequence.clear(); created = false; }
	
	if (created)
	{
		for (unsigned int j = 0; j < pg_sequence.size(); j++)
			if (pg_sequence[j] & PG_TOK) nToken++;
	}
	
	return created;
}


void CPgEventList::Load()
{
	if (created && pg_sequence.size()) tb.Pg_SetCmdAll(pg_sequence);
	else tb.Pg_SetCmd(0, 0);
}

void CPgEventList::Print()
{
	unsigned int i;
	if (created && pg_sequence.size())
	{
		for (i = 0; i < pg_sequence.size(); i++)
		{
			int x = pg_sequence[i];
			printf(" 0x%c%c%c%c%c%c %3i\n",
				(x&PG_SYNC)?'1':'0', (x&PG_REST)?'1':'0', (x&PG_RESR)?'1':'0',
				(x&PG_CAL)?'1':'0', (x&PG_TRG)?'1':'0', (x&PG_TOK)?'1':'0', int(x & 0xff));
		}
	}
	else printf("sequence not valid!\n");
}


#define ENAPX(x,y) tb.roc_Pix_Trim(x, y, 15); tb.roc_Pix_Cal(x, y);

CMD_PROC(rotest0)
{
	CPgEventList seq_fill;
	seq_fill.Add_SYNC( 0);
	seq_fill.Add_RESR( 0);
	seq_fill.Add_CAL( 20, 201);
	seq_fill.Add_CAL( 40, 201);
	seq_fill.Add_CAL( 60, 201);
	seq_fill.Add_CAL( 80, 201);
	if (!seq_fill.Check()) { printf("ERROR: seq_fill\n"); return; }

	seq_fill.Print();
	seq_fill.Load();
	tb.Pg_Loop(10000);
	DO_FLUSH
}


CMD_PROC(rotest1)
{
	Log.section("ROTEST1");

	const int trgDel = 210;
	int x, k;

	// --- create test sequences --------------------------------------------
	// --- RB fill sequence
	CPgEventList seq_fill;
	seq_fill.Add_SYNC( 0);
	seq_fill.Add_RESR( 0);
	seq_fill.Add_CAL( 20, trgDel);
	seq_fill.Add_CAL( 40, trgDel);
	seq_fill.Add_CAL( 60, trgDel);
	seq_fill.Add_CAL( 80, trgDel);
	if (!seq_fill.Check()) { printf("ERROR: seq_fill\n"); return; }

	// --- test sequence
	CPgEventList seq_test;
	seq_test.Add_CAL(   0);  //  1
	seq_test.Add_CAL( 100);  //  2
	seq_test.Add_CAL( 200);  //  3
	seq_test.Add_CAL( 300);  //  4
	seq_test.Add_CAL( 400, trgDel);
	seq_test.Add_CAL( 500);  //  6
	seq_test.Add_CAL( 600);  //  7
	seq_test.Add_CAL( 700);  //  8
	seq_test.Add_CAL( 800);  //  9
	seq_test.Add_CAL( 900);  // 10
	seq_test.Add_CAL(1000);  // 11
	seq_test.Add_CAL(1100);  // 12
	seq_test.Add_CAL(1200);  // 13
	seq_test.Add_CAL(1300, trgDel);
	seq_test.Add_CAL(1400);  // 14
	seq_test.Add_CAL(1500);  // 15
	seq_test.Add_CAL(1600);  // 16
	seq_test.Add_CAL(1700, trgDel);
	seq_test.Add_CAL(1800);  // 18
	seq_test.Add_CAL(1850);  // 19
	seq_test.Add_CAL(1900);  // 20
	seq_test.Add_CAL(1950);  // 21
	seq_test.Add_CAL(2000);  // 22
	seq_test.Add_CAL(2050);  // 23
	seq_test.Add_CAL(2100);  // 24
	seq_test.Add_CAL(2150);  // 25

	if (!seq_test.Check()) { printf("ERROR: seq_test\n"); return; }

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);
	src.Enable();

	// --- fill RB -----------------------------------------------------------
	tb.Pg_Stop();
	tb.roc_Chip_Mask();
	// load PG
	seq_fill.Load();
	// tb.Pg_SetCmdAll(seq_fill);

	// enable pixels
	for (x = 0; x < 52; x++) tb.roc_Col_Enable(x, true);
	for (x = 12; x < 28; x++)
	{
		tb.roc_Pix_Trim(x, 0, 15);
		tb.roc_Pix_Cal (x, 0);
	}

	tb.roc_SetDAC(0xfe, trgDel-5); // WBC

	// send CAL TRG
	tb.uDelay(10);
	tb.Pg_Single();
	tb.uDelay(10);

	// mask pixel
	for (x = 10; x < 26; x++) tb.roc_Pix_Mask(x, 0);
	tb.roc_ClrCal();

	// --- test -------------------------------------------------------------
	seq_test.Load();
	// tb.Pg_SetCmdAll(seq_test);

	ENAPX(10,  0)
	ENAPX(10,  2)
	ENAPX(10,  4)
	ENAPX(10,  6)
	ENAPX(10,  8)
	ENAPX(10, 10)
	ENAPX(10, 12)
	ENAPX(10, 14)
	ENAPX(10, 16)
	ENAPX(10, 18)

	tb.uDelay(10);
	tb.Pg_Single();
	tb.uDelay(50);

	// --- readout ----------------------------------------------------------
	tb.Pg_SetCmd(0, PG_TOK);
	int evCnt = 0;
	try
	{
		while (!keypressed())
		{
			tb.Pg_Single();
			tb.uDelay(100);
			CEvent *ev = data.Get();
			if (ev->roc.size() && ev->roc[0].pixel.size())
			{
				evCnt++;
				Log.printf("%3i:<%2i>", evCnt, int(ev->roc[0].pixel.size()));
				for (k = 0; k < int(ev->roc[0].pixel.size()); k++)
				{
//					int r = ev->roc[0].pixel[k].raw;
					int f = ev->roc[0].pixel[k].error;
					int x = ev->roc[0].pixel[k].x;
					int y = ev->roc[0].pixel[k].y;
					int a = ev->roc[0].pixel[k].ph;
					Log.printf (" (%2i%c%2i)", x, f ? '*' : '/', y);
//					Log.printf (" (%2i%c%2i/%3i)", x, f ? '*' : '/', y, a);
				}
				Log.puts("\n");
			} else break;
		}
	} 
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	src.Disable();
	src.Close();

	Log.printf("\n", evCnt);
	Log.flush();

	printf("%i events read.\n", evCnt);
	DO_FLUSH
}


CMD_PROC(rotest2)
{
	Log.section("ROTEST2");

	const int trgDel = 210;
	int x, y, k;

	// --- create test sequences --------------------------------------------
	// --- RB fill sequence
	CPgEventList seq_fill;
//	seq_fill.Add_SYNC( 0);
	seq_fill.Add_RESR( 0);
	seq_fill.Add_CAL( 20, trgDel);
	seq_fill.Add_CAL( 40, trgDel);
	seq_fill.Add_CAL( 60, trgDel);
	seq_fill.Add_CAL( 80, trgDel);
	if (!seq_fill.Check()) { printf("ERROR: seq_fill\n"); return; }

	// --- test sequence
	CPgEventList seq_test;
	seq_test.Add_SYNC(  0);
	seq_test.Add_CAL(   0);  //  1
	seq_test.Add_CAL( 100);  //  2
	seq_test.Add_CAL( 200);  //  3
	seq_test.Add_CAL( 300, trgDel);


	if (!seq_test.Check()) { printf("ERROR: seq_test\n"); return; }

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);
	src.Enable();

	// --- fill RB -----------------------------------------------------------
	tb.Pg_Stop();
	tb.roc_Chip_Mask();
	// load PG
	seq_fill.Load();
	// tb.Pg_SetCmdAll(seq_fill);

	// enable pixels
	for (x = 0; x < 52; x++) tb.roc_Col_Enable(x, true);
	for (x = 12; x < 28; x++)
	{
		tb.roc_Pix_Trim(x, 0, 15);
		tb.roc_Pix_Cal (x, 0);
	}

	tb.roc_SetDAC(0xfe, trgDel-5); // WBC

	// send CAL TRG
	tb.uDelay(10);
	tb.Pg_Single();
	tb.uDelay(10);

	// mask pixel
	for (x = 10; x < 26; x++) tb.roc_Pix_Mask(x, 0);
	tb.roc_ClrCal();

	// --- test -------------------------------------------------------------
	seq_test.Load();
	// tb.Pg_SetCmdAll(seq_test);

	for (y = 0; y < 80; y++) tb.roc_Pix_Trim(10, y, 0);
	
	tb.roc_Pix_Cal(10,  0);
	tb.roc_Pix_Cal(10,  2);
	tb.roc_Pix_Cal(10,  4);
	tb.roc_Pix_Cal(10,  6);
	tb.roc_Pix_Cal(10,  8);
	tb.uDelay(30);
	tb.Pg_Single();
	tb.uDelay(50);

	tb.roc_ClrCal();
	tb.roc_Pix_Cal(10, 18);
	tb.roc_Pix_Cal(10, 20);
	tb.roc_Pix_Cal(10, 22);
	tb.roc_Pix_Cal(10, 24);
	tb.uDelay(30);
	tb.Pg_Single();
	tb.uDelay(10);

	tb.roc_ClrCal();
	tb.roc_Pix_Cal(10, 36);
	tb.roc_Pix_Cal(10, 38);
	tb.roc_Pix_Cal(10, 40);
	tb.roc_Pix_Cal(10, 42);
	tb.roc_Pix_Cal(10, 44);
	tb.uDelay(30);
	tb.Pg_Single();
	tb.uDelay(50);

/*	tb.roc_ClrCal();
	tb.roc_Pix_Cal(10, 54);
	tb.roc_Pix_Cal(10, 56);
	tb.roc_Pix_Cal(10, 58);
	tb.uDelay(5);
	tb.Pg_Single();
	tb.uDelay(100);
*/

	// --- readout ----------------------------------------------------------
	tb.Pg_SetCmd(0, PG_TOK);
	int evCnt = 0;
	try
	{
		while (!keypressed())
		{
			tb.Pg_Single();
			tb.uDelay(100);
			CEvent *ev = data.Get();
			if (ev->roc.size() && ev->roc[0].pixel.size())
			{
				evCnt++;
				Log.printf("%3i:<%2i>", evCnt, int(ev->roc[0].pixel.size()));
				for (k = 0; k < int(ev->roc[0].pixel.size()); k++)
				{
//					int r = ev->roc[0].pixel[k].raw;
					int f = ev->roc[0].pixel[k].error;
					int x = ev->roc[0].pixel[k].x;
					int y = ev->roc[0].pixel[k].y;
					int a = ev->roc[0].pixel[k].ph;
					Log.printf (" (%2i%c%2i)", x, f ? '*' : '/', y);
//					Log.printf (" (%2i%c%2i/%3i)", x, f ? '*' : '/', y, a);
				}
				Log.puts("\n");
			} else break;
		}
	} 
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	src.Disable();
	src.Close();

	Log.printf("\n", evCnt);
	Log.flush();

	printf("%i events read.\n", evCnt);
	DO_FLUSH
}

CMD_PROC(rotest3)
{

	int trgDel;
	PAR_INT(trgDel, 10, 255)
	Log.section("ROTEST3", false);
	Log.printf("trgDel = %i\n", trgDel);

	unsigned int x, k;

	// --- create test sequences --------------------------------------------

	// --- RB fill sequence
	CPgEventList seq_fill;
	seq_fill.Add_RESR( 0);
	seq_fill.Add_CAL( 20, trgDel);
	seq_fill.Add_CAL( 40, trgDel);
	seq_fill.Add_CAL( 60, trgDel);
	seq_fill.Add_CAL( 80, trgDel);
	if (!seq_fill.Check()) { printf("ERROR: seq_fill\n"); return; }

	// --- test sequence
	CPgEventList seq_test;
	if (!seq_test.Read("seq.txt"))
	{
		printf("Error loading test sequence!\n");
		return;
	}

	// --- setup decoding chain ---------------------------------------------
	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigLinearDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 1000000);
	src.Enable();

		// --- fill RB -----------------------------------------------------------
	tb.Pg_Stop();
	tb.roc_Chip_Mask();
	// load PG
	seq_fill.Load();

	// enable pixels
	for (x = 12; x < 28; x++) tb.roc_Col_Enable(x, true);
	for (x = 12; x < 28; x++)
	{
		tb.roc_Pix_Trim(x, 0, 15);
		tb.roc_Pix_Cal (x, 0);
	}

	tb.roc_SetDAC(0xfe, trgDel-5); // WBC

	// send CAL TRG
	tb.uDelay(10);
	tb.Pg_Single();
	tb.uDelay(10);

	// mask pixel
	for (x = 10; x < 26; x++) tb.roc_Pix_Mask(x, 0);
	tb.roc_ClrCal();

	// --- test -------------------------------------------------------------
	for (int y = 0; y < 80; y++) tb.roc_Pix_Trim(10, y, 0);
	
	tb.roc_Pix_Cal(10,  0);
	tb.roc_Pix_Cal(10,  2);
	tb.roc_Pix_Cal(10,  4);
	tb.roc_Pix_Cal(10,  6);
	tb.roc_Pix_Cal(10,  8);
	tb.roc_Pix_Cal(10, 10);
	tb.roc_Pix_Cal(10, 12);
	tb.roc_Pix_Cal(10, 14);
	tb.roc_Pix_Cal(10, 16);
	tb.roc_Pix_Cal(10, 18);

	tb.uDelay(10);
	tb.roc_Col_Enable(10, true);
	tb.uDelay(50);
	seq_test.Load();
	tb.Pg_Single();
	tb.uDelay(100);

	// --- readout ----------------------------------------------------------
	int evCnt = 0;
	try
	{
		for (unsigned int j=0; j<seq_test.GetTokenCnt(); j++)
		{
			CEvent *ev = data.Get();
			if (ev->roc.size())
			{
				evCnt++;
				Log.printf("%3i:<%2i>", evCnt, int(ev->roc[0].pixel.size()));
				for (k = 0; k < ev->roc[0].pixel.size(); k++)
				{
//					int r = ev->roc[0].pixel[k].raw;
					int f = ev->roc[0].pixel[k].error;
					int x = ev->roc[0].pixel[k].x;
					int y = ev->roc[0].pixel[k].y;
					int a = ev->roc[0].pixel[k].ph;
					Log.printf (" (%2i%c%2i)", x, f ? '*' : '/', y);
//					Log.printf (" (%2i%c%2i/%3i)", x, f ? '*' : '/', y, a);
				}
				Log.puts("\n");
			} else break;
		}
	} 
	catch (DataPipeException &e) { printf("\nERROR TestPixel: %s\n", e.what()); }
	src.Disable();
	src.Close();

	Log.printf("\n", evCnt);
	Log.flush();

	printf("%i events read.\n", evCnt);
	DO_FLUSH
}


double GetSlopeId(int t = 0)
{
	if (t) tb.Pg_Loop(t); else tb.Pg_Stop();
	tb.mDelay(100);

	double id = 0.0;
	for (int i=0; i<10; i++)
	{
		id += tb.GetID();
		tb.mDelay(100);
	}
	id *= 100; // mA
	printf("%4i  %5.2f\n", t, id);
	return id;
}

CMD_PROC(idslope)
{
	Log.section("IDSLOPE");
	tb.Pg_Stop();

	Log.printf("inf  %5.2f\n", GetSlopeId());
	Log.printf("1000 %5.2f\n", GetSlopeId(1000));
	Log.printf(" 300 %5.2f\n", GetSlopeId( 300));
	Log.printf(" 150 %5.2f\n", GetSlopeId( 150));
	Log.printf(" 100 %5.2f\n", GetSlopeId( 100));
	Log.printf("  80 %5.2f\n", GetSlopeId(  80));
	Log.printf("  65 %5.2f\n", GetSlopeId(  65));
	Log.printf("  55 %5.2f\n", GetSlopeId(  55));
	Log.printf("  45 %5.2f\n", GetSlopeId(  45));
	Log.printf("  40 %5.2f\n", GetSlopeId(  40));
	Log.printf("  35 %5.2f\n", GetSlopeId(  35));
	Log.printf("  32 %5.2f\n", GetSlopeId(  32));
	Log.printf("  29 %5.2f\n", GetSlopeId(  29));
	Log.printf("  27 %5.2f\n", GetSlopeId(  27));
	Log.printf("  25 %5.2f\n", GetSlopeId(  25));
	Log.printf("  23 %5.2f\n", GetSlopeId(  23));
	Log.printf("  20 %5.2f\n", GetSlopeId(  20));

	tb.Pg_Loop(10000);
	tb.Flush();
	Log.flush();
}