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
	printf("nEvents: %u;  nPixels: %u;  nErrors: %u  xor=%02X, ph=%i", nEvents, nPixels, nErrors, xorbyte, ph);
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
