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

// =======================================================================
//  connection, communication, startup commands
// =======================================================================

CMD_PROC(scan)
{
	CTestboard *tb = new CTestboard;
	string name;
	vector<string> devList;
	unsigned int nDev, nr;

	try
	{
		if (!tb->EnumFirst(nDev)) throw int(1);
		for (nr=0; nr<nDev; nr++)
		{
			if (!tb->EnumNext(name)) throw int(2);
			if (name.size() < 4) continue;
			if (name.compare(0, 4, "DTB_") == 0) devList.push_back(name);
		}
	}
	catch (int e)
	{
		switch (e)
		{
		case 1: printf("Cannot access the USB driver\n"); break;
		case 2: printf("Cannot read name of connected device\n"); break;
		}
		delete tb;
		return true;
	}

	if (devList.size() == 0)
	{
		printf("no DTB connected\n");
		return true;
	}

	for (nr = 0; nr < devList.size(); nr++)
	try
	{
		printf("%10s: ", devList[nr].c_str());
		if (!tb->Open(devList[nr],false))
		{
			printf("DTB in use\n");
			continue;
		}

		unsigned int bid = tb->GetBoardId();
		printf("DTB Id %u\n", bid);
		tb->Close();
	}
	catch (...)
	{
		printf("DTB not identifiable\n");
		tb->Close();
	}

	delete tb;

	return true;
}

CMD_PROC(open)
{
	if (tb.IsConnected())
	{
		printf("Already connected to DTB.\n");
		return true;
	}

	string usbId;
	char name[80];
	if (PAR_IS_STRING(name,79)) usbId = name;
	else if (!tb.FindDTB(usbId)) return true;

	bool status = tb.Open(usbId, false);

	if (!status)
	{
		printf("USB error: %s\nCould not connect to DTB %s\n", tb.ConnectionError(), usbId.c_str());
		return true;
	}
	printf("DTB %s opened\n", usbId.c_str());

	string info;
	tb.GetInfo(info);
	printf("--- DTB info-------------------------------------\n"
		   "%s"
		   "-------------------------------------------------\n", info.c_str());

	return true;
}

CMD_PROC(close)
{
	tb.Close();
	return true;
}


CMD_PROC(rpclink)
{
	if (tb.RpcLink()) printf("ok\n");
	return true;
}


CMD_PROC(welcome)
{
	tb.Welcome();
	DO_FLUSH
	return true;
}


CMD_PROC(setled)
{
	int value;
	PAR_INT(value, 0, 0x3f);
	tb.SetLed(value);
	DO_FLUSH
	return true;
}

CMD_PROC(log)
{
	char s[256];
	PAR_STRINGEOL(s,255);
	Log.printf("%s\n", s);
	return true;
}

bool UpdateDTB(const char *filename)
{
	fstream src;

	if (tb.UpgradeGetVersion() == 0x0100)
	{
		// open file
		src.open(filename);
		if (!src.is_open())
		{
			printf("ERROR UPGRADE: Could not open \"%s\"!\n", filename);
			return false;
		}

		// check if upgrade is possible
		printf("Start upgrading DTB.\n");
		if (tb.UpgradeStart(0x0100) != 0)
		{
			string msg;
			tb.UpgradeErrorMsg(msg);
			printf("ERROR UPGRADE: %s!\n", msg.data());
			return false;
		}

		// download data
		printf("Download running ...\n");
		string rec;
		uint16_t recordCount = 0;
		while (true)
		{
			getline(src, rec);
			if (src.good())
			{
				if (rec.size() == 0) continue;
				recordCount++;
				if (tb.UpgradeData(rec) != 0)
				{
					string msg;
					tb.UpgradeErrorMsg(msg);
					printf("ERROR UPGRADE: %s!\n", msg.data());
					return false;
				}
			}
			else if (src.eof()) break;
			else
			{
				printf("ERROR UPGRADE: Error reading \"%s\"!\n", filename);
				return false;
			}
		}

		if (tb.UpgradeError() != 0)
		{
			string msg;
			tb.UpgradeErrorMsg(msg);
			printf("ERROR UPGRADE: %s!\n", msg.data());
			return false;
		}

		// write EPCS FLASH
		printf("DTB download complete.\n");
		tb.mDelay(200);
		printf("FLASH write start (LED 1..4 on)\n"
			   "DO NOT INTERUPT DTB POWER !\n"
			   "Wait till LEDs goes off.\n"
		       "Restart the DTB.\n");
		tb.UpgradeExec(recordCount);
		tb.Flush();

		return true;
	}

	printf("ERROR UPGRADE: Could not upgrade this DTB version!\n");
	return false;
}


CMD_PROC(upgrade)
{
	char filename[256];
	PAR_STRING(filename, 255);
	UpdateDTB(filename);
	return true;
}

CMD_PROC(rpcinfo)
{
	string name, call, ts;

	tb.GetRpcTimestamp(ts);
	int version = tb.GetRpcVersion();
	int n = tb.GetRpcCallCount();

	printf("--- DTB RPC info ----------------------------------------\n");
	printf("RPC version:     %i.%i\n", version/256, version & 0xff);
	printf("RPC timestamp:   %s\n", ts.c_str());
	printf("Number of calls: %i\n", n);
	printf("Function calls:\n");
	for (int i = 0; i < n; i++)
	{
		tb.GetRpcCallName(i, name);
		rpc_TranslateCallName(name, call);
		printf("%5i: %s\n", i, call.c_str());
	}
	return true;
}

CMD_PROC(info)
{
	string s;
	tb.GetInfo(s);
	printf("--- DTB info ------------------------------------\n%s"
		   "-------------------------------------------------\n", s.c_str());
	return true;
}

CMD_PROC(ver)
{
	string hw;
	tb.GetHWVersion(hw);
	int fw = tb.GetFWVersion();
	int sw = tb.GetSWVersion();
	printf("%s: FW=%i.%02i SW=%i.%02i\n", hw.c_str(), fw/256, fw%256, sw/256, sw%256);
	return true;
}

CMD_PROC(version)
{
	string hw;
	tb.GetHWVersion(hw);
	int fw = tb.GetFWVersion();
	int sw = tb.GetSWVersion();
	printf("%s: FW=%i.%02i SW=%i.%02i\n", hw.c_str(), fw/256, fw%256, sw/256, sw%256);
	return true;
}

CMD_PROC(boardid)
{
	int id = tb.GetBoardId();
	printf("\nBoard Id = %i\n", id);
	return true;
}

CMD_PROC(init)
{
	tb.Init();
	DO_FLUSH
	return true;
}

CMD_PROC(flush)
{
	tb.Flush();
	return true;
}

CMD_PROC(clear)
{
	tb.Clear();
	return true;
}




// =======================================================================
//  delay commands
// =======================================================================

CMD_PROC(udelay)
{
	int del;
	PAR_INT(del, 0, 1000);
	if (del) tb.uDelay(del);
	DO_FLUSH
	return true;
}

CMD_PROC(mdelay)
{
	int ms;
	PAR_INT(ms,1,10000)
	tb.mDelay(ms);
	return true;
}


// =======================================================================
//  test board commands
// =======================================================================

CMD_PROC(clksrc)
{
	int source;
	PAR_INT(source, 0, 1);
	tb.SetClockSource(source);
	DO_FLUSH
	return true;
}

CMD_PROC(clkok)
{
	if (tb.IsClockPresent())
		printf("clock ok\n");
	else
		printf("clock missing\n");
	return true;
}

CMD_PROC(fsel)
{
	int div;
	PAR_INT(div, 0, 5)

	tb.SetClock(div);
	DO_FLUSH
	return true;
}

CMD_PROC(stretch)
{
	int src, delay, width;
	PAR_INT(src,   0,      3)
	PAR_INT(delay, 0,   1023);
	PAR_INT(width, 0, 0xffff);
	tb.SetClockStretch(src,delay,width);
	DO_FLUSH
	return true;
}


CMD_PROC(clk)
{
	int ns, duty;
	PAR_INT(ns,0,400);
	if (!PAR_IS_INT(duty, -8, 8)) duty = 0;
	tb.Sig_SetDelay(SIG_CLK, ns, duty);
	DO_FLUSH
	return true;
}

CMD_PROC(sda)
{
	int ns, duty;
	PAR_INT(ns,0,400);
	if (!PAR_IS_INT(duty, -8, 8)) duty = 0;
	tb.Sig_SetDelay(SIG_SDA, ns, duty);
	DO_FLUSH
	return true;
}

/*
CMD_PROC(rda)
{
	int ns;
	PAR_INT(ns,0,400);
	tb.SetDelay(DELAYSIG_RDA, ns);
	DO_FLUSH
	return true;
}
*/

CMD_PROC(ctr)
{
	int ns, duty;
	PAR_INT(ns,0,400);
	if (!PAR_IS_INT(duty, -8, 8)) duty = 0;
	tb.Sig_SetDelay(SIG_CTR, ns, duty);
	DO_FLUSH
	return true;
}

CMD_PROC(tin)
{
	int ns, duty;
	PAR_INT(ns,0,400);
	if (!PAR_IS_INT(duty, -8, 8)) duty = 0;
	tb.Sig_SetDelay(SIG_TIN, ns, duty);
	DO_FLUSH
	return true;
}


CMD_PROC(clklvl)
{
	int lvl;
	PAR_INT(lvl,0,15);
	tb.Sig_SetLevel(SIG_CLK, lvl);
	DO_FLUSH
	return true;
}

CMD_PROC(sdalvl)
{
	int lvl;
	PAR_INT(lvl,0,15);
	tb.Sig_SetLevel(SIG_SDA, lvl);
	DO_FLUSH
	return true;
}

CMD_PROC(ctrlvl)
{
	int lvl;
	PAR_INT(lvl,0,15);
	tb.Sig_SetLevel(SIG_CTR, lvl);
	DO_FLUSH
	return true;
}

CMD_PROC(tinlvl)
{
	int lvl;
	PAR_INT(lvl,0,15);
	tb.Sig_SetLevel(SIG_TIN, lvl);
	DO_FLUSH
	return true;
}


CMD_PROC(clkmode)
{
	int mode;
	PAR_INT(mode,0,3);
	if (mode == 3)
	{
		int speed;
		PAR_INT(speed,0,31);
		tb.Sig_SetPRBS(SIG_CLK, speed);
	}
	else
		tb.Sig_SetMode(SIG_CLK, mode);
	DO_FLUSH
	return true;
}

CMD_PROC(sdamode)
{
	int mode;
	PAR_INT(mode,0,3);
	if (mode == 3)
	{
		int speed;
		PAR_INT(speed,0,31);
		tb.Sig_SetPRBS(SIG_SDA, speed);
	}
	else
		tb.Sig_SetMode(SIG_SDA, mode);
	DO_FLUSH
	return true;
}

CMD_PROC(ctrmode)
{
	int mode;
	PAR_INT(mode,0,3);
	if (mode == 3)
	{
		int speed;
		PAR_INT(speed,0,31);
		tb.Sig_SetPRBS(SIG_CTR, speed);
	}
	else
		tb.Sig_SetMode(SIG_CTR, mode);
	DO_FLUSH
	return true;
}

CMD_PROC(tinmode)
{
	int mode;
	PAR_INT(mode,0,3);
	if (mode == 3)
	{
		int speed;
		PAR_INT(speed,0,31);
		tb.Sig_SetPRBS(SIG_TIN, speed);
	}
	else
		tb.Sig_SetMode(SIG_TIN, mode);
	DO_FLUSH
	return true;
}


CMD_PROC(sigoffset)
{
	int offset;
	PAR_INT(offset, 0, 15);
	tb.Sig_SetOffset(offset);
	DO_FLUSH
	return true;
}

CMD_PROC(lvds)
{
	tb.Sig_SetLVDS();
	DO_FLUSH
	return true;
}

CMD_PROC(lcds)
{
	tb.Sig_SetLCDS();
	DO_FLUSH
	return true;
}

/*
CMD_PROC(tout)
{
	int ns;
	PAR_INT(ns,0,450);
	tb.SetDelay(SIGNAL_TOUT, ns);
	DO_FLUSH
	return true;
}

CMD_PROC(trigout)
{
	int ns;
	PAR_INT(ns,0,450);
	tb.SetDelay(SIGNAL_TRGOUT, ns);
	DO_FLUSH
	return true;
}
*/


CMD_PROC(pon)
{
	tb.Pon();
	DO_FLUSH
	return true;
}

CMD_PROC(poff)
{
	tb.Poff();
	DO_FLUSH
	return true;
}

CMD_PROC(va)
{
	int value;
	PAR_INT(value, 0, 4000);
	tb._SetVA(value);
	DO_FLUSH
	return true;
}

CMD_PROC(vd)
{
	int value;
	PAR_INT(value, 0, 4000);
	tb._SetVD(value);
	DO_FLUSH
	return true;
}


CMD_PROC(ia)
{
	int value;
	PAR_INT(value, 0, 1200);
	tb._SetIA(value*10);
	DO_FLUSH
	return true;
}

CMD_PROC(id)
{
	int value;
	PAR_INT(value, 0, 1200);
	tb._SetID(value*10);
	DO_FLUSH
	return true;
}

CMD_PROC(getva)
{
	double v = tb.GetVA();
	printf("\n VA = %1.3fV\n", v);
	return true;
}

CMD_PROC(getvd)
{
	double v = tb.GetVD();
	printf("\n VD = %1.3fV\n", v);
	return true;
}

CMD_PROC(getia)
{
	double i = tb.GetIA();
	printf("\n IA = %1.1fmA\n", i*1000.0);
	return true;
}

CMD_PROC(getid)
{
	double i = tb.GetID();
	printf("\n ID = %1.1fmA\n", i*1000.0);
	return true;
}


CMD_PROC(hvon)
{
	tb.HVon();
	DO_FLUSH
	return true;
}

CMD_PROC(hvoff)
{
	tb.HVoff();
	DO_FLUSH
	return true;
}

CMD_PROC(reson)
{
	tb.ResetOn();
	DO_FLUSH
	return true;
}

CMD_PROC(resoff)
{
	tb.ResetOff();
	DO_FLUSH
	return true;
}


CMD_PROC(status)
{
	uint8_t status = tb.GetStatus();
	printf("SD card detect: %c\n", (status&8) ? '1' : '0');
	printf("CRC error:      %c\n", (status&4) ? '1' : '0');
	printf("Clock good:     %c\n", (status&2) ? '1' : '0');
	printf("CLock present:  %c\n", (status&1) ? '1' : '0');
	return true;
}

CMD_PROC(rocaddr)
{
	int addr;
	PAR_INT(addr, 0, 15)
	tb.SetRocAddress(addr);
	DO_FLUSH
	return true;
}

CMD_PROC(d1)
{
	int sig;
	PAR_INT(sig, 0, 31)
	tb.SignalProbeD1(sig);
	DO_FLUSH
	return true;
}


CMD_PROC(d2)
{
	int sig;
	PAR_INT(sig, 0, 31)
	tb.SignalProbeD2(sig);
	DO_FLUSH
	return true;
}

CMD_PROC(a1)
{
	int sig;
	PAR_INT(sig, 0, 7)
	tb.SignalProbeA1(sig);
	DO_FLUSH
	return true;
}

CMD_PROC(a2)
{
	int sig;
	PAR_INT(sig, 0, 7)
	tb.SignalProbeA2(sig);
	DO_FLUSH
	return true;
}

CMD_PROC(probeadc)
{
	int sig;
	PAR_INT(sig, 0, 7)
	tb.SignalProbeADC(sig);
	DO_FLUSH
	return true;
}

CMD_PROC(pgset)
{
	int addr, delay, bits;
	PAR_INT(addr,0,255)
	PAR_INT(bits,0,63)
	PAR_INT(delay,0,255)
	tb.Pg_SetCmd(addr, (bits<<8) + delay);
	DO_FLUSH
	return true;
}

CMD_PROC(pgstop)
{
	tb.Pg_Stop();
	DO_FLUSH
	return true;
}

CMD_PROC(pgsingle)
{
	tb.Pg_Single();
	DO_FLUSH
	return true;
}

CMD_PROC(pgtrig)
{
	tb.Pg_Trigger();
	DO_FLUSH
	return true;
}

CMD_PROC(pgloop)
{
	int period;
	PAR_INT(period,0,65535)
	tb.Pg_Loop(period);
	DO_FLUSH
	return true;
}


// === DAQ ==================================================================

CMD_PROC(dopen)
{
	int buffersize;
	int channel;
	PAR_INT(buffersize, 0, 60000000);
	if (!PAR_IS_INT(channel, 0, 7)) channel = 0;
	buffersize = tb.Daq_Open(buffersize, channel);
	printf("%i words allocated for data buffer %i\n", buffersize, channel);
	if (buffersize == 0) printf("error\n");
	return true;
}

CMD_PROC(dclose)
{
	int channel;
	if (!PAR_IS_INT(channel, 0, 7)) channel = 0;
	tb.Daq_Close(channel);
	DO_FLUSH
	return true;
}


CMD_PROC(dstart)
{
	int channel;
	if (!PAR_IS_INT(channel, 0, 7)) channel = 0;
	tb.Daq_Start(channel);
	DO_FLUSH
	return true;
}

CMD_PROC(dstop)
{
	int channel;
	if (!PAR_IS_INT(channel, 0, 7)) channel = 0;
	tb.Daq_Stop(channel);
	DO_FLUSH
	return true;
}


CMD_PROC(dsize)
{
	int channel;
	if (!PAR_IS_INT(channel, 0, 7)) channel = 0;
	unsigned int size = tb.Daq_GetSize(channel);
	printf("size = %u\n", size);
	return true;
}


void DecodeTbmHeader(unsigned int raw)
{
	int evNr = raw >> 8;
	int stkCnt = raw & 6;
	printf("  EV(%3i) STF(%c) PKR(%c) STKCNT(%2i)",
		evNr,
		(raw&0x0080)?'1':'0',
		(raw&0x0040)?'1':'0',
		stkCnt
		);
}

void DecodeTbmTrailer(unsigned int raw)
{
	int dataId = (raw >> 6) & 0x3;
	int data   = raw & 0x3f;
	printf("  NTP(%c) RST(%c) RSR(%c) SYE(%c) SYT(%c) CTC(%c) CAL(%c) SF(%c) D%i(%2i)",
		(raw&0x8000)?'1':'0',
		(raw&0x4000)?'1':'0',
		(raw&0x2000)?'1':'0',
		(raw&0x1000)?'1':'0',
		(raw&0x0800)?'1':'0',
		(raw&0x0400)?'1':'0',
		(raw&0x0200)?'1':'0',
		(raw&0x0100)?'1':'0',
		dataId,
		data
		);
}

void DecodePixel(unsigned int raw)
{
	unsigned int ph = (raw & 0x0f) + ((raw >> 1) & 0xf0);
	raw >>= 9;
	int c =    (raw >> 12) & 7;
	c = c*6 + ((raw >>  9) & 7);
	int r =    (raw >>  6) & 7;
	r = r*6 + ((raw >>  3) & 7);
	r = r*6 + ( raw        & 7);
	int y = 80 - r/2;
	int x = 2*c + (r&1);
	printf("   Pixel [%05o] %2i/%2i: %3u", raw, x, y, ph);
}


CMD_PROC(dread)
{
	uint32_t words_remaining = 0;
	vector<uint16_t> data;
	tb.Daq_Read(data, 256, words_remaining, 0);
	int size = data.size();
	printf("#samples: %i remaining: %i\n", size, int(words_remaining));

	for (int i=0; i<size; i++)
	{
		printf(" %04X", int(data[i]));
		if (i%10 == 9) printf("\n");
	}
	printf("\n");

	return true;
}

CMD_PROC(dreadr)
{
	uint32_t words_remaining = 0;
	vector<uint16_t> data;
	tb.Daq_Read(data, words_remaining);
	int size = data.size();
	printf("#samples: %i remaining: %i\n", size, int(words_remaining));

	for (int i=0; i<size; i++)
	{
		int x = data[i] & 0x0fff;
		if (x & 0x0800) x |= 0xfffff000;
		printf(" %6i", x);
		if (i%10 == 9) printf("\n");
	}
	printf("\n");

	for (int i=0; i<size; i++)
	{
		int x = data[i] & 0x0fff;
		if (x & 0x0800) x |= 0xfffff000;
		Log.printf("%6i", x);
		if (i%100 == 9) printf("\n");
	}
	printf("\n");

	return true;
}

CMD_PROC(dreadm)
{
	int channel;
	if (!PAR_IS_INT(channel, 0, 7)) channel = 0;

	uint32_t words_remaining = 0;
	vector<uint16_t> data;
//	int TBM_eventnr,TBM_stackinfo,ColAddr,RowAddr,PulseHeight,TBM_trailerBits,TBM_readbackData;

	tb.Daq_Read(data, 4096, words_remaining, channel);
	int size = data.size();
	printf("#samples: %i  remaining: %u\n", size, (unsigned int)(words_remaining));

	unsigned int hdr, trl;
	unsigned int raw;
	for (int i=0; i<size; i++)
	{
		int d = data[i] & 0xf;
		int q = (data[i]>>4) & 0xf;
		switch (q)
		{
		case  0: printf("  0(%1X)", d); break;

		case  1: printf("\n R1(%1X)", d); raw = d; break;
		case  2: printf(" R2(%1X)", d);   raw = (raw<<4) + d; break;
		case  3: printf(" R3(%1X)", d);   raw = (raw<<4) + d; break;
		case  4: printf(" R4(%1X)", d);   raw = (raw<<4) + d; break;
		case  5: printf(" R5(%1X)", d);   raw = (raw<<4) + d; break;
		case  6: printf(" R6(%1X)", d);   raw = (raw<<4) + d;
			     DecodePixel(raw);
				 break;

		case  7: printf("\nROC-HEADER(%1X): ", d); break;

		case  8: printf("\n\nTBM H1(%1X) ", d); hdr = d; break;
		case  9: printf("H2(%1X) ", d);       hdr = (hdr<<4) + d; break;
		case 10: printf("H3(%1X) ", d);       hdr = (hdr<<4) + d; break;
		case 11: printf("H4(%1X) ", d);       hdr = (hdr<<4) + d;
			     DecodeTbmHeader(hdr);
			     break;

		case 12: printf("\nTBM T1(%1X) ", d); trl = d; break;
		case 13: printf("T2(%1X) ", d);       trl = (trl<<4) + d; break;
		case 14: printf("T3(%1X) ", d);       trl = (trl<<4) + d; break;
		case 15: printf("T4(%1X) ", d);       trl = (trl<<4) + d;
			     DecodeTbmTrailer(trl);
			     break;
		}
	}

	for (int i=0; i<size; i++)
	{
		int x = data[i] & 0xffff;
		Log.printf("%04X", x);
		if (i%100 == 9) printf("\n");
	}
	printf("\n");
	Log.printf("\n");
	Log.flush();

	return true;
}

CMD_PROC(dreada)
{
	uint32_t words_remaining = 0;
	vector<uint16_t> data;
	tb.Daq_Read(data, words_remaining);
	int size = data.size();
	printf("#samples: %i remaining: %i\n", size, int(words_remaining));

	for (int i=0; i<size; i++)
	{
		int x = data[i] & 0x0fff;
		if (x & 0x0800) x |= 0xfffff000;
		printf(" %6i", x);
		if (i%10 == 9) printf("\n");
	}
	printf("\n");

	for (int i=0; i<size; i++)
	{
		int x = data[i] & 0x0fff;
		if (x & 0x0800) x |= 0xfffff000;
		Log.printf("%6i", x);
		if (i%100 == 9) printf("\n");
	}
	printf("\n");

	return true;
}



// =======================================================================
//  tbm commands
// =======================================================================

CMD_PROC(tbmdis)
{
	tb.tbm_Enable(false);
	DO_FLUSH
	return true;
}

CMD_PROC(tbmsel)
{
	int hub, port;
	PAR_INT(hub,0,31);
	PAR_INT(port,0,3);
	tb.tbm_Enable(true);
	tb.tbm_Addr(hub,port);
	DO_FLUSH
	return true;
}

CMD_PROC(modsel)
{
	int hub;
	PAR_INT(hub,0,31);
	tb.tbm_Enable(true);
	tb.mod_Addr(hub);
	DO_FLUSH
	return true;
}

CMD_PROC(tbmset)
{
	int reg, value;
	PAR_INT(reg,0,255);
	PAR_INT(value,0,255);
	tb.tbm_Set(reg,value);

	DO_FLUSH
	return true;
}

/*
CMD_PROC(tbmget)
{
	int reg;
	unsigned char value;
	PAR_INT(reg,0,255);
	if (tb.tbm_Get(reg,value))
	{
		printf(" reg 0x%02X = %3i (0x%02X)\n", reg, (int)value, (int)value);
	} else puts(" error\n");

	return true;
}

CMD_PROC(tbmgetraw)
{
	int reg;
	long value;
	PAR_INT(reg,0,255);
	if (tb.tbm_GetRaw(reg,value))
	{
		printf("value=0x%02X (Hub=%2i; Port=%i; Reg=0x%02X; inv=0x%X; stop=%c)\n",
			value & 0xff, (value>>19)&0x1f, (value>>16)&0x07,
			(value>>8)&0xff, (value>>25)&0x1f, (value&0x1000)?'1':'0');
	} else puts("error\n");

	return true;
}


void PrintTbmData(int reg, int value)
{
	if (value < 0)
	{
		printf("02X: ?|        |\n", reg);
		return;
	}
	printf("%02X:%02X", reg, value);
	switch (reg & 0x0e)
	{
	case 0:
		printf("|        |");
		if (value&0x40) printf("TrigOut |");   else printf("        |");
		if (value&0x20) printf("Stack rd|");   else printf("        |");
		if (value&0x20) printf("        |");   else printf("Trig In |");
		if (value&0x10) printf("Pause RO|");   else printf("        |");
		if (value&0x08) printf("Stack Fl|");   else printf("        |");
		if (value&0x02) printf("        |");   else printf("TBM Clk |");
		if (value&0x01) printf(" Full RO|\n"); else printf("Half RO |\n");
		break;

	case 2:
		if (value&0x80)                printf("| Mode:Sync       |");
		else if ((value&0xc0) == 0x80) printf("| Mode:Clear EvCtr|");
		else                           printf("| Mode:Pre-Cal    |");

		printf(" StackCount: %2i                                      |\n",
			value&0x3F);
		break;

	case 4:
		printf("| Event Number: %3i             "
			"                                        |\n", value&0xFF);
		break;

	case 6:
		if (value&0x80) printf("|        |");   else printf("|Tok Pass|");
		if (value&0x40) printf("TBM Res |");   else printf("        |");
		if (value&0x20) printf("ROC Res |");   else printf("        |");
		if (value&0x10) printf("Sync Err|");   else printf("        |");
		if (value&0x08) printf("Sync    |");   else printf("        |");
		if (value&0x04) printf("Clr TCnt|");   else printf("        |");
		if (value&0x02) printf("PreCalTr|");   else printf("        |");
		if (value&0x01) printf("Stack Fl|\n"); else printf("        |\n");
		break;

	case 8:
		printf("|        |        |        |        |        |");
		if (value&0x04) printf("Force RO|");   else printf("        |");
		if (value&0x02) printf("        |");   else printf("Tok Driv|");
		if (value&0x01) printf("        |\n"); else printf("Ana Driv|\n");
		break;
	}
}

const char regline[] =
"+--------+--------+--------+--------+--------+--------+--------+--------+";

CMD_PROC(tbmregs)
{
	const int reg[13] =
	{
		0xE1,0xE3,0xE5,0xE7,0xE9,
		0xEB,0xED,0xEF,
		0xF1,0xF3,0xF5,0xF7,0xF9
	};
	int i;
	int value[13];
	unsigned char data;

	for (i=0; i<13; i++)
		if (tb.tbm_Get(reg[i],data)) value[i] = data; else value[i] = -1;
	printf(" reg  TBMA   TBMB\n");
	printf("TBMA %s\n", regline);
	for (i=0; i<5;  i++) PrintTbmData(reg[i],value[i]);
	printf("TBMB %s\n", regline);
	for (i=8; i<13; i++) PrintTbmData(reg[i],value[i]);
	printf("     %s\n", regline);
	for (i=5; i<8; i++)
	{
		printf("%02X:", reg[i]);
		if (value[i]>=0) printf("%02X\n", value[i]);
		else printf(" ?\n");

	}
	return true;
}

CMD_PROC(modscan)
{
	int hub;
	unsigned char data;
	printf(" hub:");
	for (hub=0; hub<32; hub++)
	{
		tb.mod_Addr(hub);
		if (tb.tbm_Get(0xe1,data)) printf(" %2i", hub);
	}
	puts("\n");
	return true;
}
*/

// =======================================================================
//  roc commands
// =======================================================================
int roclist[16] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


CMD_PROC(select)
{
	int rocmin, rocmax, i;
	PAR_RANGE(rocmin,rocmax, 0,15)

	for (i=0; i<rocmin;  i++) roclist[i] = 0;
	for (;    i<=rocmax; i++) roclist[i] = 1;
	for (;    i<16;      i++) roclist[i] = 0;

	tb.roc_I2cAddr(rocmin);
	return true;
}

CMD_PROC(dac)
{
	int addr, value;
	PAR_INT(addr,1,255)
	PAR_INT(value,0,255)

	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i); tb.roc_SetDAC(addr, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(vana)
{
	int value;
	PAR_INT(value,0,255)
	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i); tb.roc_SetDAC(Vana, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(vtrim)
{
	int value;
	PAR_INT(value,0,255)
	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i); tb.roc_SetDAC(Vtrim, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(vthr)
{
	int value;
	PAR_INT(value,0,255)
	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i); tb.roc_SetDAC(VthrComp, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(vcal)
{
	int value;
	PAR_INT(value,0,255)
	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i); tb.roc_SetDAC(Vcal, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(wbc)
{
	int value;
	PAR_INT(value,0,255)
	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i);  tb.roc_SetDAC(WBC, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(ctl)
{
	int value;
	PAR_INT(value,0,255)
	for (int i=0; i<16; i++) if (roclist[i])
	{ tb.roc_I2cAddr(i); tb.roc_SetDAC(CtrlReg, value); }

	DO_FLUSH
	return true;
}

CMD_PROC(cole)
{
	int col, colmin, colmax;
	PAR_RANGE(colmin,colmax, 0,ROC_NUMCOLS-1)
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		for (col=colmin; col<=colmax; col+=2)
			tb.roc_Col_Enable(col, 1);
	}

	DO_FLUSH
	return true;
}

CMD_PROC(cold)
{
	int col, colmin, colmax;
	PAR_RANGE(colmin,colmax, 0,ROC_NUMCOLS-1)
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		for (col=colmin; col<=colmax; col+=2)
			tb.roc_Col_Enable(col, 0);
	}

	DO_FLUSH
	return true;
}

CMD_PROC(pixe)
{
	int col,colmin,colmax, row,rowmin,rowmax, value;
	PAR_RANGE(colmin,colmax, 0,ROC_NUMCOLS-1)
	PAR_RANGE(rowmin,rowmax, 0,ROC_NUMROWS-1)
	PAR_INT(value,0,15)
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		for(col=colmin; col<=colmax; col++) for (row=rowmin; row<=rowmax; row++)
			tb.roc_Pix_Trim(col,row, 15-value);
	}

	DO_FLUSH
	return true;
}

CMD_PROC(pixd)
{
	int col,colmin,colmax, row,rowmin,rowmax;
	PAR_RANGE(colmin,colmax, 0,ROC_NUMCOLS-1)
	PAR_RANGE(rowmin,rowmax, 0,ROC_NUMROWS-1)
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		for(col=colmin; col<=colmax; col++) for (row=rowmin; row<=rowmax; row++)
			tb.roc_Pix_Mask(col,row);
	}

	DO_FLUSH
	return true;
}

CMD_PROC(cal)
{
	int col,colmin,colmax, row,rowmin,rowmax;
	PAR_RANGE(colmin,colmax, 0,ROC_NUMCOLS-1)
	PAR_RANGE(rowmin,rowmax, 0,ROC_NUMROWS-1)
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		for(col=colmin; col<=colmax; col++) for (row=rowmin; row<=rowmax; row++)
			tb.roc_Pix_Cal(col,row,false);
	}

	DO_FLUSH
	return true;
}

CMD_PROC(cals)
{
	int col,colmin,colmax, row,rowmin,rowmax;
	PAR_RANGE(colmin,colmax, 0,ROC_NUMCOLS-1)
	PAR_RANGE(rowmin,rowmax, 0,ROC_NUMROWS-1)
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		for(col=colmin; col<=colmax; col++) for (row=rowmin; row<=rowmax; row++)
			tb.roc_Pix_Cal(col,row,true);
	}

	DO_FLUSH
	return true;
}

CMD_PROC(cald)
{
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		tb.roc_ClrCal();
	}

	DO_FLUSH
	return true;
}

CMD_PROC(mask)
{
	for (int i=0; i<16; i++) if (roclist[i])
	{
		tb.roc_I2cAddr(i);
		tb.roc_Chip_Mask();
	}

	DO_FLUSH
	return true;
}

