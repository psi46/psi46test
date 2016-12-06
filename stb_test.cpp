// stb_test.cpp

#include "conio.h"
#include "psi46test.h"
#include "datastream.h"
// #include "stb_tools.h"
#include "stb_test.h"
#include "profiler.h"


namespace stb
{

CPower power;


// === SigData ==============================================================
//
// ==========================================================================

void SigSdata::Invalidate()
{
	vp.Invalidate();
	vn.Invalidate();
	idle.Invalidate();
	quality.Invalidate();
	phase.Invalidate();
}


const char* SigSdata::Report()
{
	static const double VMIN     =  60.0; // mV
	static const double VMAX     = 120.0; // mV
	static const double VDIFFMAX =  30.0; // mV

	static char s[16];

	if (vp.IsValid() && vn.IsValid())
	{
		bool lvlErr = false;
		strcpy(s, " {     }");
		if      (vp < VMIN) { s[2] = '+'; s[3] = 'L'; lvlErr = true; }
		else if (vp > VMAX) { s[2] = '+'; s[3] = 'H'; lvlErr = true; }
		if      (vn < VMIN) { s[5] = '-'; s[6] = 'L'; lvlErr = true; }
		else if (vn > VMAX) { s[5] = '-'; s[6] = 'H'; lvlErr = true; }
		if     (fabs(vp-vn) > VDIFFMAX) { s[4] = '*'; lvlErr = true; }
		if (lvlErr) return s;
	}

	if (idle.IsValid())
	{
		if (idle.Get() == 1) return " { LOS }";
		if (idle.Get() == 2) return " {ASYNC}";
	}

	if (quality.IsValid()) { sprintf(s, " {%3.0f%% }", quality*100.0); return s; }

	return "  ----- ";
}



// === CModule ==============================================================
//
// ==========================================================================

void CModule::Invalidate()
{ PROFILING
	hub.Invalidate();

	vd_pon.Invalidate();
	va_pon.Invalidate();
	id_pon.Invalidate();
	ia_pon.Invalidate();

	vd.Invalidate();
	va.Invalidate();
	id.Invalidate();
	ia.Invalidate();

	int i;
	for (i=0; i<4; i++) sdata[i].Invalidate();
	for (i=0; i<16; i++) roc_present[i].Invalidate();

	SetTBM_Default();
}


bool CModule::Report()
{ PROFILING
	int i;
	char s[128];
	bool error = false;

	sprintf(s, " %2i %3i %2i %2i", mod.GetSel(), mod.Get().moduleConnector,
		mod.Get().powerGrp, mod.Get().hvGrp);
	Log.puts(s); printf(s);

	if (hub.IsValid() && hub >= 0)
	{
		sprintf(s, "  %2i ", hub.Get());
		Log.puts(s); printf(s);
	}
	else
	{
		Log.puts("   . ");
		printf  ("   . ");
		error = true;
	}

	for (i=0; i<4; i++)
	{
		const char *s =	sdata[i].Report();
		Log.puts(s); fputs(s, stdout);
	}

	Log.puts("  "); printf("  ");

	for (i=0; i<16; i++)
	{
		if (roc_present[i].IsValid())
		{
			if (roc_present[i].Get())
			{
				sprintf(s, "%X", i);
				Log.puts(s); printf(s);
			}
			else
			{
				Log.puts(".");
				printf(".");
				error = true;
			}
		}
		else
		{
			Log.puts(".");
			printf(".");
			error = true;
		}
	}

	if (error) { Log.puts(" ERROR\n"); printf(" ERROR\n"); }
	else       { Log.puts("\n"); printf("\n"); }

	return !error;
}


void CModule::SetTBM_Default()
{
	tbm_phase0 = tbm_phase1 = 0x24; // 001_001_00  160 MHz, 400 MHz

	if (mod.Get().tbm == MC::TBM10)
	{
		for (int i=0; i<4; i++)
			tbm_delay[i] = 0x00; // 0_0_000_000
	}
	else
	{
		for (int i=0; i<4; i++)
			tbm_delay[i] = 0xe4; // 1_1_100_100
	}
}


void CModule::SetTBM_Phase160(unsigned int ph160, unsigned int tbm)
{
	if (ph160 > 7) ph160 = 7;

	if (tbm == 0) tbm_phase0 = (tbm_phase0 & 0x1f) + (ph160 << 5);
	else          tbm_phase1 = (tbm_phase1 & 0x1f) + (ph160 << 5);
}


void CModule::SetTBM_Phase400(unsigned int ph400, unsigned int tbm)
{
	if (ph400 > 7) ph400 = 7;

	if (tbm == 0) tbm_phase0 = (tbm_phase0 & 0xe3) + (ph400 << 2);
	else          tbm_phase1 = (tbm_phase1 & 0xe3) + (ph400 << 2);
}


void CModule::SetTBM_Delay(unsigned int delay, unsigned int group)
{ // (TBM n, Reg e7) (TBM n, Reg f7) (TBM n+1, Reg e7) (TBM n+1, Reg f7)
  // 1_1_100_100
	if (delay > 7) delay = 7;

	if (mod.Get().tbm == MC::TBM10)
	{ // Layer 1 module
		switch (group)
		{
		case 0: tbm_delay[3] = (tbm_delay[3] & 0xf8) + delay;      break;
		case 1: tbm_delay[3] = (tbm_delay[3] & 0xc7) + (delay<<3); break;
		case 2: tbm_delay[0] = (tbm_delay[0] & 0xf8) + delay;      break;
		case 3: tbm_delay[0] = (tbm_delay[0] & 0xc7) + (delay<<3); break;
		case 4: tbm_delay[1] = (tbm_delay[1] & 0xf8) + delay;      break;
		case 5: tbm_delay[1] = (tbm_delay[1] & 0xc7) + (delay<<3); break;
		case 6: tbm_delay[2] = (tbm_delay[2] & 0xf8) + delay;      break;
		case 7: tbm_delay[2] = (tbm_delay[2] & 0xc7) + (delay<<3); break;
		}
	}
	else
	{ // Layer 2, 3, 4 module
		switch (group)
		{
		case 0: tbm_delay[0] = (tbm_delay[0] & 0xf8) + delay;      break;
		case 1: tbm_delay[0] = (tbm_delay[0] & 0xc7) + (delay<<3); break;
		case 2: tbm_delay[1] = (tbm_delay[1] & 0xf8) + delay;      break;
		case 3: tbm_delay[1] = (tbm_delay[1] & 0xc7) + (delay<<3); break;
		}
	}
}


void CModule::UpdateTBM()
{ PROFILING
	int id = mod.GetSel();

	if (mod.Get().tbm == stb::MC::TBM10)
	{
		mod.SetRocAddr(0); // switch to TBM n+1
		tb.tbm_Set( 0xea, tbm_delay[2]);
		tb.tbm_Set( 0xfa, tbm_delay[3]);
		tb.tbm_Set( 0xee, tbm_phase1);

		mod.SetRocAddr(4); // switch to TBM n
		tb.tbm_Set( 0xea, tbm_delay[0]);
		tb.tbm_Set( 0xfa, tbm_delay[1]);
		tb.tbm_Set( 0xee, tbm_phase0);
	}
	else
	{
		tb.tbm_Set( 0xea, tbm_delay[0]);
		tb.tbm_Set( 0xfa, tbm_delay[1]);
		tb.tbm_Set( 0xee, tbm_phase0);
	}
}


void CModule::InitTBM(unsigned int tbm)
{ PROFILING
	// Init TBM, Reset ROC
	tb.tbm_Set( 0xe4, 0xf0); // 11110000
	tb.tbm_Set( 0xf4, 0xf0); // Clear Trigger Counter, Clear Token Out, Clear Stack, Inject TBM Reset

	// Disable Auto Reset, Disable PKAM Counter
	tb.tbm_Set( 0xe0, 0x81); // 10000001
	tb.tbm_Set( 0xf0, 0x81); // Disable Auto Reset, Disable PKAM Counter

	// Mode: Calibration
	tb.tbm_Set( 0xe2, 0xc0); // 11000000
	tb.tbm_Set( 0xf2, 0xc0);

	// Set PKAM Counter (x+1)*6.4us
	tb.tbm_Set( 0xe8, 128);
	tb.tbm_Set( 0xf8, 128);

	// Delays: Tok _ Hdr/Trl _ Port 1 _ Port 0
	tb.tbm_Set( 0xea, (tbm==0)? tbm_delay[0] : tbm_delay[2]);
	tb.tbm_Set( 0xfa, (tbm==0)? tbm_delay[1] : tbm_delay[3]);

	// Auto reset rate (x+1)*256
	tb.tbm_Set( 0xec, 128);
	tb.tbm_Set( 0xfc, 128);

	// 160/400 MHz phase adjust
	tb.tbm_Set( 0xee, (tbm==0)? tbm_phase0 : tbm_phase1);

	// Temp measurement control
	tb.tbm_Set( 0xfe, 0x00);
}


void CModule::InitROC()
{ PROFILING
	if (mod.Get().roc == stb::MC::PROC600)
	{ // PROC600
		tb.roc_SetDAC(  1,  10);    // Vdig
		tb.roc_SetDAC(  2,  35);    // Vana
		tb.roc_SetDAC(  3,   8);    // *new* Iph (new 4 bit, previously Vsf)
		tb.roc_SetDAC(  4,  12);    // Vcomp

		tb.roc_SetDAC(  7, 150);    // VwllPr
		tb.roc_SetDAC(  9, 150);    // VwllSh
		tb.roc_SetDAC( 11,  40);    // Vtrim
		tb.roc_SetDAC( 12,  40);    // VthrComp

		tb.roc_SetDAC( 13, 100);    // *new* VColor (previously VIBias_Bus)

		tb.roc_SetDAC( 17, 125);    // *new* VoffsetRO (previously default: 170)

		tb.roc_SetDAC( 19,  50);    // Vcomp_ADC
		tb.roc_SetDAC( 20,  90);    // VIref_ADC

		tb.roc_SetDAC( 25,   2);    // Vcal
		tb.roc_SetDAC( 26,  68);    // CalDel

		tb.roc_SetDAC( 0xfe, 14);   // WBC
		tb.roc_SetDAC( 0xfd, 0x0c); // *new* CtrlReg (trigger output control)
		tb.roc_AllCol_Enable(false);
	}
	else
	{ // PSI46dig
		tb.roc_SetDAC(  1,  10);    // Vdig
		tb.roc_SetDAC(  2,  30);    // Vana
		tb.roc_SetDAC(  3,  30);    // Vsf
		tb.roc_SetDAC(  4,  12);    // Vcomp

		tb.roc_SetDAC(  7, 150);    // VwllPr
		tb.roc_SetDAC(  9, 150);    // VwllSh
		tb.roc_SetDAC( 10, 117);    // VhldDel
		tb.roc_SetDAC( 11,  40);    // Vtrim
		tb.roc_SetDAC( 12,  40);    // VthrComp

		tb.roc_SetDAC( 13,  30);    // VIBias_Bus
		tb.roc_SetDAC( 22,  99);    // VIColOr

		tb.roc_SetDAC( 17, 170);    // VoffsetRO

		tb.roc_SetDAC( 19,  50);    // Vcomp_ADC
		tb.roc_SetDAC( 20,  90);    // VIref_ADC

		tb.roc_SetDAC( 25,   2);    // Vcal
		tb.roc_SetDAC( 26,  68);    // CalDel

		tb.roc_SetDAC( 0xfe, 14);   // WBC
		tb.roc_SetDAC( 0xfd,  4);   // CtrlReg
		tb.roc_AllCol_Enable(false);
	}
}


void CModule::_InitModule()
{ PROFILING
	if (mod.Get().tbm == stb::MC::TBM10)
	{
		mod.SetRocAddr(4); // switch to TBM n
		InitTBM(0);
		mod.SetRocAddr(0); // switch to TBM n+1
		InitTBM(1);
	}
	else
	{
		InitTBM();
	}

	for (int roc=0; roc<16; roc++)
	{
		mod.SetRocAddr(roc);
		InitROC();
	}
}


void CModule::InitModule(bool forceAllHubs)
{ PROFILING
	if (hub.IsValid())
	{
		mod.SetHubAddr(hub.Get());
		_InitModule();
	}
	else if (forceAllHubs)
	{
		int hub_incr = (mod.Get().tbm == stb::MC::TBM10)? 2 : 1;
		for (int hub = 0; hub < 32; hub += hub_incr)
		{
			mod.SetHubAddr(hub);
			_InitModule();
		}
	}
}


bool CModule::Test_PowerOn()
{ PROFILING

	if (mod.Get().powerGrp >= 0)
	{
		Log.section("PON", false);
		Log.printf(" power group %i\n", mod.Get().powerGrp);
	} else Log.section("PON");

	power.ModPon(mod);
	tb.ResetOff();
	tb.uDelay(100);

	vd_pon = power.GetVD(mod);
	id_pon = power.GetID(mod);
	va_pon = power.GetVA(mod);
	ia_pon = power.GetIA(mod);

	Log.printf(" VD(%0.3fV, %0.3fA)\n VA(%0.3fV, %0.3fA)\n",
		vd_pon.Get(), id_pon.Get(), va_pon.Get(), ia_pon.Get());
	if (id_pon.Get() > 2.4 || ia_pon.Get() > 2.2) { power.ModPoff(mod); return false; }
	return true;
}


void CModule::Test_Init()
{ PROFILING
	Log.section("INIT");
	InitModule(true);
	tb.mDelay(300);
	vd = power.GetVD(mod);
	id = power.GetID(mod);
	va = power.GetVA(mod);
	ia = power.GetIA(mod);
	Log.printf(" VD(%0.3fV, %0.3fA)\n VA(%0.3fV, %0.3fA)\n",
		vd.Get(), id.Get(), va.Get(), ia.Get());
}


void CModule::Test_Sdata()
{ PROFILING
	CDataProcessing data(mod);
	data.Enable();
	int sd, nSdata = mod.Get().nSdata;

	// --- SDATA signal voltage level
	if (mod.Get().NeedsSTB())
	{
		Log.section("SDATA_LEVEL");

		for (sd=0; sd<nSdata; sd++)
		{
			double vp, vn;
			if (data.GetVsdata(sd, vp, vn))
			{
				vp *= 1000.0; vn *= 1000.0; // V -> mV
				sdata[sd].vp = vp;
				sdata[sd].vn = vn;
				Log.printf(" V+ = %4.0fmV  V- = %4.0fmV", vp, vn);
			} else Log.puts("   ---- ----  ");
		}
		Log.puts("\n");
	}
	// --- SDATA idle pattern transitions (DESER400)
	Log.section("SDATA_IDLE", "for each channel <xor phase>");

	char s[9];

	int transCount[4] = {0, 0, 0, 0};
	int patternOr[4] = {0, 0, 0, 0};
	for (int i=0; i<10; i++) // repeat 10 times
	{
		for (sd=0; sd<nSdata; sd++) // for all sdata
		{
			int deser = data.GetDeser(sd*2);
			int phase = tb.Deser400_GetPhase(deser);
			int xor = tb.Deser400_GetXor(deser);
			if (xor) transCount[sd]++;
			patternOr[sd] |= xor;
			for (int k=0, mask=1; k<8; k++, mask<<=1) s[k] = (xor & mask) ? 'X' : '.';
			s[8] = 0;
			Log.printf(" <%s  %i>", s, phase);
		}
		Log.puts("\n");
	}

	// analyze xor pattern
	for (sd=0; sd<nSdata; sd++) // for all sdata
	{
		if (transCount[sd] == 10) // transitions detected ?
		{
			sdata[sd].idle = 2; // assume async
			for (int i=0; i<8; i++)
			{
				if ((patternOr[sd] & 3) == 0) // gap found XXXXXX..
				{
					sdata[sd].idle = 0; // ok
					break;
				}
				patternOr[sd] <<= 1; if (patternOr[sd] & 0x100) patternOr[sd]++; // rol
			}
		}
		else
		{
			sdata[sd].idle = 1; // missing transitions
		}

	}

	// --- SDATA quality measurement during data transmission
	Log.section("SDATA_QUALITY");

	int ch, nCh = nSdata*2;
	CSignalQuality sig[8];

	for (ch=0; ch<nCh; ch++) data.AddPipe(ch, sig[ch]);

	// --- DEBUG ---------------------------------------
	//CRocRawDataPrinter dbg_raw[8];
	//CEventPrinter dbg_event[8];
	//for (ch=0; ch<nCh; ch++)
	//{
	//	char s[32];
	//	sprintf(s, "DEBUG_raw_%i.txt", ch);
	//	dbg_raw[ch].Open(s);
	//	data.AddPipe(ch, dbg_raw[ch]);

	//	sprintf(s, "DEBUG_event_%i.txt", ch);
	//	dbg_event[ch].Open(s);
	//	data.AddPipe(ch, dbg_event[ch]);
	//}
	// --- DEBUG_END -----------------------------------


	data.SendReset();
	data.TakeData(900);

	for (ch=0; ch<nCh; ch++)
	{
		sig[ch].CalculateQP();
		Log.printf("%i: ", ch);
		sig[ch].Print();
	}

	for (sd=0; sd<nSdata; sd++)
	{
		int n = 0;
		double ph = 0.0, q = 0.0;
		if (sig[sd*2].n)
		{
			n++;
			ph += sig[sd*2].phase;
			q  += sig[sd*2].quality;
		}
		if (sig[sd*2+1].n)
		{
			n++;
			ph += sig[sd*2+1].phase;
			q  += sig[sd*2+1].quality;
		}
		if (n)
		{
			sdata[sd].phase   = ph/n;
			sdata[sd].quality = q/n;
		}
	}
}


bool CModule::Find_HubAddress(bool debug)
{ PROFILING
	int ch, nCh, i, roc, nRoc, hub_id, hub_id_incr;
	// set up data pipe
	CDataProcessing data(mod);
	nCh = mod.Get().nSdata*2;
	CReadback rdb[8];
	for (ch=0; ch<nCh; ch++)
	{
//		rdb[ch].Logging();
		data.AddPipe(ch, rdb[ch]);
	}
	if (debug) data.EnableLogging("debug_event_");

	hub_id_incr = (mod.Get().tbm == MC::TBM10)? 2 : 1;
	for (hub_id = 0; hub_id < 32; hub_id += hub_id_incr)
	{
		mod.SetHubAddr(hub_id);
		for (i=0; i<8; i++) rdb[i].Clear();

		// readback select "last dac" for all rocs
		for (roc=0; roc<16; roc++) { mod.SetRocAddr(roc); tb.roc_SetDAC(255, 0); }

		// write (10+rocAddr) to register 30
		for (roc=0; roc<16; roc++)
		{
			mod.SetRocAddr(roc);
			tb.roc_SetDAC(30, 10+roc);
		}

		data.TakeData(32);

		nRoc = 8/mod.Get().nSdata;
		roc = 0;
		int nGood = 0;
		for (ch = 0; ch < nCh; ch++) for (i=0; i<nRoc; i++)
		{
			int value = rdb[ch][i];
			if (value >= 0)
			{
				int vRoc = value >> 12;
				value &= 0xff;
				if (vRoc == roc && value == (10+roc)) nGood++;
			}
			roc++;
		}
		if (nGood >=4) break;
	}

	if (hub_id >= 32) hub_id = -1;
	Log.section("HUB", false);
	Log.printf("%i\n", hub_id);

	if (hub_id >= 0) hub = hub_id;
	return hub_id >= 0;
}


void CModule::Test_RocProgramming()
{ PROFILING
	Log.section("ROCPROG");

	// set up data pipe
	CDataProcessing data(mod);

	int i;
	int ch,  nCh  = mod.Get().nSdata*2;
	int roc, nRoc = 8/mod.Get().nSdata;

	CReadback rdb[8];
	for (ch=0; ch<nCh; ch++) data.AddPipe(ch, rdb[ch]);

	// take data
	data.SendReset();

	unsigned int roc_res[16];
	for (roc=0; roc<16; roc++)
	{
		mod.SetRocAddr(roc);
		tb.roc_SetDAC(255, 0);
		roc_res[roc] = 0;
	}

	Log.printf("DAC = 20 ...\n");
	for (roc=0; roc<16; roc++) { mod.SetRocAddr(roc); tb.roc_SetDAC(30, 10+roc); }
	data.TakeData(32);
	for (ch=0; ch<nCh; ch++) { Log.printf("RDB%i:", ch); rdb[ch].Print(); }

	roc = 0;
	for (ch = 0; ch < nCh; ch++) for (i=0; i<nRoc; i++)
	{
		int value = rdb[ch][i];
		if (value >= 0)
		{
			int vRoc = value >> 12;
			value &= 0xff;
			if (vRoc == roc && value == (10+roc)) roc_res[roc]++;
		}
		roc++;
	}

	Log.printf("DAC = 100 ...\n");
	for (roc=0; roc<16; roc++) { mod.SetRocAddr(roc); tb.roc_SetDAC(30, 100+roc); }
	data.TakeData(32);
	for (ch=0; ch<nCh; ch++) { Log.printf("RDB%i:", ch); rdb[ch].Print(); }

	roc = 0;
	for (ch = 0; ch < nCh; ch++) for (i=0; i<nRoc; i++)
	{
		int value = rdb[ch][i];
		if (value >= 0)
		{
			int vRoc = value >> 12;
			value &= 0xff;
			if (vRoc == roc && value == (100+roc)) roc_res[roc]++;
		}
		roc++;
	}

	for (roc=0; roc<16; roc++) roc_present[roc] = roc_res[roc] == 2;
}


void CModule::DelayScan(unsigned int group)
{
	if (!hub.IsValid())
	{
		printf("No hub address assigned!\n");
		return;
	}

	Log.SummaryMode(true);
	Log.section("DELAY", false);
	Log.printf(" sel:%i; hub:%i; grp:%i\n", mod.GetSel(), hub.Get(), group);
	Log.SummaryMode(false);
	// --- module power on
	power.PowerSave(true);
	power.ModPon(mod);
	
	// --- init module
	mod.SetHubAddr(hub);
	_InitModule();
	
	// --- setup data processing
	int ch, nCh = mod.Get().nSdata*2;
	int i, roc, nRoc = 8/mod.Get().nSdata;
	CDataProcessing data(mod);
//	data.EnableLogging("delayscan");
	CReadback rdb[8];
	for (ch=0; ch<nCh; ch++) data.AddPipe(ch, rdb[ch]);

	// --- scan
	std::string map[8];
	for (int i=0; i<8; i++) map[i] = "........";

	for (int del0 = 0; del0 < 8; del0++) for (int del1 = 0; del1 < 8; del1++)
	{
		SetTBM_Delay(del0, group);
		SetTBM_Delay(del1, group+1);
		UpdateTBM();
		data.TakeData(32);
		roc = 0;
		std::string s = "................";
		for (ch = 0; ch < nCh; ch++) for (i=0; i<nRoc; i++)
		{
			int value = rdb[ch][i];
			if (value >= 0)
			{
				int vRoc = value >> 12;
				s[roc] = (vRoc > 9) ? 'A' + vRoc - 10 : '0' + vRoc;
			} else s[roc] = '.';
			roc++;
		}
		bool isOk = s == "0123456789ABCDEF";
		if (isOk) map[del0][del1] = 'X';

		Log.printf("Delay %i %i: %s", del0, del1, s.c_str());
		if (!isOk) Log.puts(" ERROR\n"); else Log.puts("\n");
	}
	Log.SummaryMode(true);
	printf("   01234567\n");
	Log.puts("   01234567\n");
	for (int i=0; i<8; i++)
	{
		printf(" %i %s\n", i, map[i].c_str());
		Log.printf(" %i %s\n", i, map[i].c_str());
	}
	Log.flush();
	Log.SummaryMode(false);

	// module power off
	power.ModPoff(mod);
	Log.flush();
}


void CModule::Test()
{ PROFILING
	Invalidate();

	if (Test_PowerOn())
	{
		Test_Init();
		Test_Sdata();
		Find_HubAddress(false);
		if (hub.IsValid())
		{
			mod.SetHubAddr(hub);
			Test_RocProgramming();
		}
	}
	tb.Flush();

}



// === CSlot ================================================================
//
// ==========================================================================


void CSlot::Invalidate()
{ PROFILING
	cb_voltage.Invalidate();
	cb_current.Invalidate();
	cb_current_lo.Invalidate();
	cb_current_hi.Invalidate();
	module.clear();
}


void CSlot::Report()
{ PROFILING
	bool error = false;
	Log.puts("\n");
	Log.section("REPORT", name.c_str());

	list<CModule>::iterator i;

	// --- connector board power -----------------------------------------------
	if (cb_voltage.IsValid())
	{
		char s[128];
		sprintf(s, "cb  %4.2f V  %0.0f mA (lo:%0.0f mA  hi:%0.0f mA)\n",
			cb_voltage.Get(),  1000.0*cb_current, 1000.0*cb_current_lo, 1000.0*cb_current_hi);
		Log.puts(s); printf(s);
	}

	// --- power groups -----------------------------------------------------
	int k;
	unsigned int n[6], nexist[6];
	char ct[6];
	double vd[6], id[6], va[6], ia[6];
	for (k=0; k<6; k++) { vd[k] = id[k] = va[k] = ia[k] = 0.0; n[k] = nexist[k] = 0; ct[k] = '?'; }

	int pgrp;
	for (i=module.begin(); i != module.end(); i++)
	{
		pgrp = i->mod.Get().powerGrp;
		if (0 <= pgrp && pgrp < 6)
		{
			if (i->vd.IsValid())
			{
				ct[pgrp] = ' ';
				n[pgrp]++;
				if (i->hub.IsValid()) nexist[pgrp]++;
				vd[pgrp] += i->vd;
				id[pgrp] += i->id;
				va[pgrp] += i->va;
				ia[pgrp] += i->ia;
			}
			else if (i->vd_pon.IsValid())
			{
				ct[pgrp] = '*';
				n[pgrp]++;
				if (i->hub.IsValid()) nexist[pgrp]++;
				vd[pgrp] += i->vd_pon;
				id[pgrp] += i->id_pon;
				va[pgrp] += i->va_pon;
				ia[pgrp] += i->ia_pon;
			}
		}
	}

	for (pgrp=0; pgrp<6; pgrp++)
	{
		if (n[pgrp])
		{
			vd[pgrp] /= n[pgrp];
			id[pgrp] /= n[pgrp];
			va[pgrp] /= n[pgrp];
			ia[pgrp] /= n[pgrp];
			char s[256];
			sprintf(s, "pg%i%c(%i/%u): VD(%4.2f V, %4.2f A) VA(%4.2f V, %4.2f A)\n",
				pgrp, ct[pgrp], nexist[pgrp], n[pgrp], vd[pgrp], id[pgrp], va[pgrp], ia[pgrp]);
			Log.puts(s);
			fputs(s, stdout);
		}
	}

	// --- data channels ----------------------------------------------------
	static const char title[] = "sdata\n    Con  P HV Hub   SDATA1  SDATA2  SDATA3  SDATA4  ------ROC-------\n";
	Log.puts(title); printf(title);

	for (i=module.begin(); i != module.end(); i++)
	{
		if (!i->Report()) error = true;
	}

	// --- error report -----------------------------------------------------
	// check for multiple hub ids and missing hubs
	int missingHubId = 0;
	int activeSdata  = 0;
	int hubCount[32] = { 0 };
	for (i=module.begin(); i != module.end(); i++)
	{
		// check sdata 0 levels
		int chCnt = i->mod.Get().nSdata;
		int chActive = 0;
		for (int ch = 0; ch < chCnt; ch++)
		{
			if (i->sdata[ch].vp.IsValid() && (i->sdata[ch].vp.Get() > 20.0)) chActive++;
			if (i->sdata[ch].vn.IsValid() && (i->sdata[ch].vn.Get() > 20.0)) chActive++;
		}
		if (chActive > 0) activeSdata++;

		// check hub id
		if (i->hub.IsValid())
		{
			int h = i->hub;
			if (0 <= h && h <= 31)
			{
				hubCount[h]++;
				if (hubCount[h] > 1) error = true;
			}
		} else missingHubId++;
	}

	int missingModule = module.size() - activeSdata;
	if (missingModule || missingHubId) error = true;

	if (error)
	{
		printf("=======ERROR=======\n");
		Log.section("ERROR");

		if (missingModule)
		{
			printf(    "missing modules: %2i\n", missingModule);
			Log.printf("missing modules: %2i\n", missingModule);
		}

		if (missingHubId)
		{
			printf(    "missing hub ids: %2i\n", missingHubId);
			Log.printf("missing hub ids: %2i\n", missingHubId);
		}
		for (int h=0; h<32; h++)
		{
			if (hubCount[h] > 1)
			{
				printf(    "multiple hub id: %2i\n", h);
				Log.printf("multiple hub id: %2i\n", h);
			}
		}
	} else printf("ok\n");
	Log.section("REPORT_END");
}


void CSlot::Test_Init()
{ PROFILING
	tb.Sig_SetDelay(SIG_CLK,  4);
	tb.Sig_SetDelay(SIG_SDA, 19);
	tb.Sig_SetDelay(SIG_CTR,  4);
	tb.Sig_SetDelay(SIG_TIN,  9);

	tb.Sig_SetLevel(SIG_CLK, 10);
	tb.Sig_SetLevel(SIG_SDA, 10);
	tb.Sig_SetLevel(SIG_CTR, 10);
	tb.Sig_SetLevel(SIG_TIN, 10);

	power.PowerSave(true);
	tb.tbm_Enable(true);
	tb.Flush();
}


double CSlot::GetIC()
{ PROFILING
	tb.mDelay(100);
	static const int N = 10;
	double ic = 0.0;
	for (int k=0; k<N; k++) ic += power.GetIC();
	ic /= N;
	return ic;
}


bool CSlot::Test_ConnectorBoard()
{ PROFILING
	double vc, ic, ic_lo, ic_hi;

	Log.section("CB_PON");
	power.CbPon();
	vc = power.GetVC();
	cb_voltage = vc;
	ic = GetIC();
	cb_current = ic;
	Log.printf(" VC(%0.3fV, %0.0fmA)\n", vc, ic*1000.0);
	if (ic > 0.3)
	{
		printf("ic = %0.0fmA (> 300mA)\n", ic*1000.0);
		return false;
	}

	tb.Sig_SetMode(SIG_CLK, SIG_MODE_LO);
	tb.Sig_SetMode(SIG_CTR, SIG_MODE_LO);
	tb.Sig_SetMode(SIG_SDA, SIG_MODE_LO);
	tb.Sig_SetMode(SIG_CTR, SIG_MODE_LO);
	ic_lo = GetIC();

	tb.Sig_SetMode(SIG_CLK, SIG_MODE_HI);
	tb.Sig_SetMode(SIG_CTR, SIG_MODE_HI);
	tb.Sig_SetMode(SIG_SDA, SIG_MODE_HI);
	tb.Sig_SetMode(SIG_CTR, SIG_MODE_HI);
	ic_hi = GetIC();

	tb.Sig_SetMode(SIG_CLK, SIG_MODE_NORMAL);
	tb.Sig_SetMode(SIG_CTR, SIG_MODE_NORMAL);
	tb.Sig_SetMode(SIG_SDA, SIG_MODE_NORMAL);
	tb.Sig_SetMode(SIG_CTR, SIG_MODE_NORMAL);

	cb_current_lo = ic_lo;
	cb_current_hi = ic_hi;
	Log.printf(" IC_LO: %0.0fmA   IC_HI: %0.0fmA\n", ic_lo*1000.0, ic_hi*1000.0);

	return true;
}


void CSlot::_Test()
{	PROFILING
	Test_Init();
	if (Test_ConnectorBoard())
	{
		printf("HUB:");
		
		list<CModule>::iterator i;
		for (i = module.begin(); i != module.end(); i++)
		{
			Log.section("MODULE", false);
			Log.printf("%2i Connector:%3i\n", i->mod.GetSel(), i->mod.Get().moduleConnector);

			i->Test();

			if (i->hub.IsValid() && i->hub.Get() >= 0)
				printf(" %i(%i)", i->mod.Get().moduleConnector, i->hub.Get());
			else printf(" %i(?)", i->mod.Get().moduleConnector);

			Log.section("MODULE_END");
		}
		power.ModPoffAll();
		printf("\n");
	}
	power.CbPoff();
	tb.Flush();
}


void CSlot::Test(const string &slotName)
{ PROFILING
	Invalidate();

	bool stbPresent = false;
	int adapter = MC::ADP_NO;

	stbPresent = tb.stb_IsPresent();
	if (stbPresent) adapter = tb.stb_GetAdapterId();

	// --- check slot/module name  and create modules -----------------------
	if (stbPresent)
	{ // if STB present test via connector board
		if (slotName.length() == 0) { printf("Slot name expected!\n"); return; }

		if (adapter == MC::ADP_NO)
		{
			printf("No STB adapter connected!\n");
			return;
		}
		name = slotName;
		for (int sel = 0; sel <= 38; sel++)
		{
			CModType mod = sel;
			if (mod.Get().adapter == adapter) module.push_back(CModule(sel));
		}
	}
	else
	{ // if STB not present test via module adapter
		if (( slotName.length() < 2) || ((name[0] != 'L') && (name[0] != 'l')))
		{
			printf("Illegal module name. Name must start with L1, L2, L3 or L4\n");
			return;
		}
		name = slotName;
		switch (name[1])
		{
			case '1':
				module.push_back(CModule(MODTYPE_L1));
				break;
			case '2':
				module.push_back(CModule(MODTYPE_L2));
				break;
			case '3':
			case '4':
				module.push_back(CModule(MODTYPE_L34));
				break;
			default:
				printf("Wrong module type!\n");
				return;
		}
	}

	if (module.size() == 0) { printf("No modules to test!\n"); return; }

	// --- test
	Log.timestamp("TEST_BEGIN");

	if (stbPresent)
	{ // --- if STB present test via connector board
		Log.section("SLOT", name.c_str());

		int layer = module.begin()->mod.Get().layer;
		Log.section("LAYER", false); Log.printf("%i\n", layer);
		_Test();
		Log.section("SLOT_END");
	}
	else
	{ // --- if STB not present test via module adapter
		Log.section("MODULE", name.c_str());
		_Test();
		Log.section("MODULE_END");
	}

	Log.SummaryMode(true);
	Report();
	Log.puts("\n");
	Log.flush();
	Log.SummaryMode(false);
	Log.timestamp("TEST_END");
	Log.flush();
}


void CSlot::StartAllModules()
{
	power.PowerSave(false);
	power.CbPon();
	tb.mDelay(500);
	printf("Module init:");
	list<CModule>::iterator k;
	for (k = module.begin(); k != module.end(); k++)
	{
		power.ModPon(k->mod, 50);
		if (k->hub.IsValid())
		{
			printf(" %i(%i)", k->mod.Get().moduleConnector, k->hub.Get());
			tb.mDelay(300);
			k->InitModule();
		}
	}
	printf("\n");
}


void CSlot::StopAllModules()
{
	power.ModPoffAll();
	power.PowerSave(false);
}


void CSlot::RdaSingleTest(int sel, int hub, CModType m)
{
	for (int r=0; r<16; r++)
	{
		m.SetRocAddr(r);
		tb.roc_SetDAC(2, 0); // vana
		tb.uDelay(100);
	}
	tb.mDelay(500);
	printf("Sel:%2i   Hub:%2i   IA_lo: %3.0f mA", sel, hub, power.GetIA(m)*1000.0);

	for (int r=0; r<16; r++)
	{
		m.SetRocAddr(r);
		tb.roc_SetDAC(2, 30); // vana
		tb.uDelay(100);
	}
	tb.mDelay(500);
	printf("  IA_hi: %3.0f mA\n", power.GetIA(m)*1000.0);
}


void CSlot::RdaTest(int sel, int hub)
{
	if (sel >= 0)
	{
		CModType m = sel;

		if (hub < 0)
		{
			list<CModule>::iterator k;
			for (k = module.begin(); k != module.end(); k++)
			{
				if (k->mod.GetSel() == sel && k->hub.IsValid())
				{
					hub = k->hub;
					break;
				}
			}
			if (hub < 0) return;
		}

		char ch = 0;
		m.SetHubAddr(hub);
		do
		{
			RdaSingleTest(sel, hub, m);
			ch = getch();
		} while (ch != 27);
	}
	else
	{
		char ch = 0;
		list<CModule>::iterator k;
		for (k = module.begin(); k != module.end(); k++)
		{
			if (k->hub.IsValid())
			{
				k->mod.SetHubAddr(k->hub);
				do
				{
					RdaSingleTest(k->mod.GetSel(), k->hub, k->mod);
					ch = getch();
				} while(ch == 'r');
			}
			if (ch == 27) break;
		}
	}
}


CModule* CSlot::FindModuleBySelector(unsigned int sel)
{
	list<CModule>::iterator k;
	for (k = module.begin(); k != module.end(); k++)
		if (k->mod.GetSel() == sel) return &(*k);
	return 0;
}


void CSlot::AssignHub(unsigned int sel, int hub)
{
	CModule *k = FindModuleBySelector(sel);
	if (k)
	{
		if (hub < 0) k->hub.Invalidate();
		else k->hub = hub;
	}
}


void CSlot::DelayScan(unsigned int sel, unsigned int group)
{
	CModule *k = FindModuleBySelector(sel);
	if (k)
	{
		power.CbPon();
		k->DelayScan(group);
		k->SetTBM_Default();
		power.CbPoff();
	}
	tb.Flush();
}


} // namespace stb
