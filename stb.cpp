// stb.cpp
//
// Sector Test Board

#include "psi46test.h"
#include "datastream.h"
// #include "stb_tools.h"
#include "stb.h"
#include "modulesettings.h"
#include "profiler.h"


namespace stb {


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
		if      (vp < VMIN) { s[2] = 'P'; s[3] = 'L'; lvlErr = true; }
		else if (vp > VMAX) { s[2] = 'P'; s[3] = 'H'; lvlErr = true; }
		if      (vn < VMIN) { s[5] = 'N'; s[6] = 'L'; lvlErr = true; }
		else if (vn > VMAX) { s[5] = 'N'; s[6] = 'H'; lvlErr = true; }
		if     (fabs(vp-vn) > VDIFFMAX) { s[4] = '*'; lvlErr = true; }
		if (lvlErr) return s;
	}

	if (idle.IsValid() && idle) return " { LOS }";

	if (quality.IsValid()) { sprintf(s, " {%3.0f%% }", quality*100.0); return s; }

	return "  ----- ";
}


void ModuleData::Invalidate()
{
	mod.Invalidate();
	hub.Invalidate();
	int i;
	for (i=0; i<4; i++) sdata[i].Invalidate();
	for (i=0; i<16; i++) roc_present[i].Invalidate();

}


bool ModuleData::Report()
{
	int i;
	char s[128];
	bool error = false;

	if (mod.IsValid())
	{
		sprintf(s, " %2i %3i", mod.Get().GetSel(), mod.Get().Get().moduleConnector);
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
	}
	else { Log.puts("  ?"); printf("  ?"); }

	if (error) { Log.puts(" ERROR\n"); printf(" ERROR\n"); }
	else       { Log.puts("\n"); printf("\n"); }

	return !error;
}


void SlotData::Report()
{
	bool error = false;

	Log.section("REPORT", name.c_str());

	if (cb_current.IsValid())
	{
		char s[128];
		sprintf(s, "connector_board  %0.0f mA (lo:%0.0fmA  hi:%0.0fmA)\n",
			1000.0*cb_current, 1000.0*cb_current_lo, 1000.0*cb_current_hi);
		Log.puts(s); printf(s);
	}

	static const char title[] = "    Con Hub   SDATA1  SDATA2  SDATA3  SDATA4  ------ROC-------\n";
	Log.puts(title); printf(title);

	list<ModuleData>::iterator i;
	for (i=module.begin(); i != module.end(); i++)
	{
		if (!i->Report()) error = true;
	}

	// check for multiple hub ids
	int hubCount[32] = { 0 };
	for (i=module.begin(); i != module.end(); i++)
	{
		int h = i->hub;
		if (h < 0 || h > 31) continue;
		hubCount[h]++;
	}

	for (int h=0; h<32; h++)
	{
		if (hubCount[h] > 1)
		{
			printf("Multiple hub id: %i", h);
			error = true;
		}
	}

	if (error) printf("Error exists!\n");

	Log.section("REPORT_END");
}


void SlotData::Invalidate()
{
	cb_current.Invalidate();
	module.clear();
}


SlotData   result_slot;
ModuleData result_module;


CPower power;


// === Test functions =======================================================

void Test_Init()
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


double GetIC()
{ PROFILING
	tb.mDelay(100);
	static const int N = 10;
	double ic = 0.0;
	for (int k=0; k<N; k++) ic += power.GetIC();
	ic /= N;
	return ic;
}


bool Test_ConnectorBoard()
{ PROFILING
	double vc, ic, ic_lo, ic_hi;

	Log.section("CB_PON");
	power.CbPon();
	tb.mDelay(500);
	vc = power.GetVC();
	ic = GetIC();
	result_slot.cb_current = ic;
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

	result_slot.cb_current_lo = ic_lo;
	result_slot.cb_current_hi = ic_hi;
	Log.printf(" IC_LO: %0.0fmA   IC_HI: %0.0fmA\n", ic_lo*1000.0, ic_hi*1000.0);

	return true;
}



bool ModuleOn(CModType mod)
{ PROFILING
	
	if (mod.Get().powerGrp >= 0)
	{
		Log.section("PON", false);
		Log.printf(" power group %i\n", mod.Get().powerGrp);
	} else Log.section("PON");

	power.ModPon(mod);

	tb.mDelay(400);
	tb.ResetOff();
	tb.mDelay(100);

	double vd = power.GetVD(mod);
	double id = power.GetID(mod);
	double va = power.GetVA(mod);
	double ia = power.GetIA(mod);

	Log.printf(" VD(%0.3fV, %0.3fA)\n VA(%0.3fV, %0.3fA)\n", vd, id, va, ia);
	if (id > 2.4) { power.ModPoff(mod); return false; }
	return true;
}


void ModuleOff(CModType mod)
{ PROFILING
	power.ModPoff(mod);
}



void Module_Init(CModType mod)
{ PROFILING
	Log.section("INIT");
	modSettings.InitAllModules(mod);
	tb.mDelay(500);
	double vd = power.GetVD(mod);
	double id = power.GetID(mod);
	double va = power.GetVA(mod);
	double ia = power.GetIA(mod);
	Log.printf(" VD(%0.3fV, %0.3fA)\n VA(%0.3fV, %0.3fA)\n", vd, id, va, ia);
}


void Check_Sdata(CModType mod)
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
				result_module.sdata[sd].vp = vp;
				result_module.sdata[sd].vn = vn;
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
			result_module.sdata[sd].idle = 3; // assume async
			for (int i=0; i<8; i++)
			{
				if ((patternOr[sd] & 3) == 0) // gap found XXXXXX..
				{
					result_module.sdata[sd].idle = 0; // ok
					break;
				}
				patternOr[sd] <<= 1; if (patternOr[sd] & 0x100) patternOr[sd]++; // rol
			}
		}
		else
		{
			result_module.sdata[sd].idle = 1; // missing transitions
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


	data.Enable();
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
			result_module.sdata[sd].phase   = ph/n;
			result_module.sdata[sd].quality = q/n;
		}
	}
}


int Find_HubAddress(CModType mod)
{ PROFILING
	int ch, nCh, i, roc, nRoc, hub, hub_incr;
	// set up data pipe
	CDataProcessing data(mod);
	nCh = mod.Get().nSdata*2;
	CReadback rdb[8];
	for (ch=0; ch<nCh; ch++) data.AddPipe(ch, rdb[ch]);

	hub_incr = (mod.Get().tbm == MC::TBM10)? 2 : 1;
	for (hub = 0; hub < 32; hub += hub_incr)
	{
		mod.SetHubAddr(hub);
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

	if (hub >= 32) hub = -1;
	Log.section("HUB", false);
	Log.printf("%i\n", hub);
	return hub; // -1 = hub address not found
}


void Check_RocProgramming(CModType mod)
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

	for (roc=0; roc<16; roc++) result_module.roc_present[roc] = roc_res[roc] == 2;
}


void Test_Module(CModType mod)
{ PROFILING
	result_module.Invalidate();
	result_module.mod = mod;

	ModuleOn(mod);
	Module_Init(mod);
	Check_Sdata(mod);
	result_module.hub = Find_HubAddress(mod);
	if (result_module.hub >= 0)
	{
		mod.SetHubAddr(result_module.hub);
		Check_RocProgramming(mod);
	}
	ModuleOff(mod);
	tb.Flush();

	result_slot.module.push_back(result_module);
}



// ==========================================================================


void Test_Slot(int layer)
{ PROFILING
	if (layer == 2) layer = 1;
	if (Test_ConnectorBoard())
	{
		for (int i=0; i<42; i++)
		{
			CModType mod = i;
			int modLayer = mod.Get().layer;
			if (modLayer == 2) modLayer = 1;
			if (modLayer != layer) continue;

			Log.section("MODULE", false);
			Log.printf("%2i Connector:%3i\n", mod.GetSel(), mod.Get().moduleConnector);
			Test_Module(mod);
			Log.section("MODULE_END");
		}
	}
	power.CbPoff();
	tb.Flush();
}



void Sectortest(const char *name)
{ PROFILING
	int adapter = 3;
	int layer   = 1;
	CModType mod;
	bool stbPresent = tb.stb_IsPresent();

	// --- check slot/module name
	unsigned int lenName = strlen(name);
	if (strlen(name) < 2) {	printf("Missing name\n"); return; }

	if (stbPresent)
	{ // if STB present test via connector board
		if (lenName == 0) { printf("Slot name expected!\n"); return; }
		adapter = tb.stb_GetAdapterId();
		switch (adapter)
		{
			case ADP_L12: layer = 1; break;
			case ADP_L3:  layer = 3; break;
			case ADP_L4:  layer = 4; break;
			default:
				printf("No STB adapter connected!\n");
				return;
		}
	}
	else
	{ // if STB not present test via module adapter
		if ((lenName < 2) || ((name[0] != 'L') && (name[0] != 'l')))
		{
			printf("Illegal module name. Name must start with L1, L2, L3 or L4\n");
			return;
		}
		switch (name[1])
		{
			case '1': mod = MODTYPE_L1; break;
			case '2': mod = MODTYPE_L2; break;
			case '3':
			case '4': mod = MODTYPE_L34; break;
			default:
				printf("Wrong module type!\n");
				return;
		}
	}

	// --- test
	Log.timestamp("TEST_BEGIN");
	result_slot.Invalidate();
	result_slot.name = name;
	Test_Init();

	if (stbPresent)
	{ // --- if STB present test via connector board
		Log.section("SLOT", name);
		Log.section("LAYER", false); Log.printf("%i\n", layer);
		Test_Slot(layer);
		Log.section("SLOT_END");
	}
	else
	{ // --- if STB not present test via module adapter
			Log.section("MODULE", name);
			Test_Module(mod);
			Log.section("MODULE_END");
	}

	result_slot.Report();
	Log.timestamp("TEST_END");
	Log.puts("\n");
	Log.flush();
}


} // namespace stb
