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
	Scope("SDATA1", values);

	return true;
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
	for (unsigned int i=0; i<length; i++) in.push_back(int(i));

	tb.VectorTest(in, out);

	// compare vectors
}


CMD_PROC(daqtest)
{
	// setup pg data sequence (0, 1, 2, ... 99)
	tb.Pg_SetCmd(0, PG_RESR + 4);
	for (int k=1; k<100; k++) tb.Pg_SetCmd(k, PG_TOK + 2);
	tb.Pg_SetCmd(100, PG_TOK);

	tb.Daq_Open(60000000, 0);
	tb.Daq_Select_Datagenerator(0);
	tb.Daq_Start(0);

	int errors = 0;
	int i = 0;
	do
	{
		// create data
//		tb.Daq_Start(0);
		for (int k=0; k<10000; k++)
		{
			tb.Pg_Single();
			tb.uDelay(250); // 250
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
					dvalue = (dvalue+1)%100;
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
