// stb_tools.cpp

#include "psi46test.h"
#include "datastream.h"
#include "stb_tools.h"


namespace stb {


// === CModType =============================================================

const MC CModType::MODCONF[42] =
{ // adapter      layer sdata      nSdata  powerGrp hvGrp  tbm        roc     moduleConnector
	{ MC::ADP_L12, 1, MC::SDATA_4321, 4,     0, 1,    0,  MC::TBM10, MC::PROC600,  112}, //  0
	{ MC::ADP_L12, 1, MC::SDATA_4321, 4,     1, 1,    1,  MC::TBM10, MC::PROC600,  113}, //  1
	{ MC::ADP_L12, 1, MC::SDATA_4321, 4,     2, 1,    2,  MC::TBM10, MC::PROC600,  114}, //  2

	{ MC::ADP_L12, 2, MC::SDATA_43,   2,     3, 3,    3,  MC::TBM9,  MC::PSI46DIG, 211}, //  3
	{ MC::ADP_L12, 2, MC::SDATA_21,   2,     3, 3,    4,  MC::TBM9,  MC::PSI46DIG, 212}, //  4
	{ MC::ADP_L12, 2, MC::SDATA_43,   2,     3, 3,    5,  MC::TBM9,  MC::PSI46DIG, 213}, //  5
	{ MC::ADP_L12, 2, MC::SDATA_21,   2,     4, 2,    6,  MC::TBM9,  MC::PSI46DIG, 214}, //  6
	{ MC::ADP_L12, 2, MC::SDATA_43,   2,     4, 2,    3,  MC::TBM9,  MC::PSI46DIG, 221}, //  7
	{ MC::ADP_L12, 2, MC::SDATA_21,   2,     5, 3,    4,  MC::TBM9,  MC::PSI46DIG, 222}, //  8
	{ MC::ADP_L12, 2, MC::SDATA_43,   2,     5, 3,    5,  MC::TBM9,  MC::PSI46DIG, 223}, //  9
	{ MC::ADP_L12, 2, MC::SDATA_21,   2,     5, 3,    6,  MC::TBM9,  MC::PSI46DIG, 224}, // 10

	{ MC::ADP_L3,  3, MC::SDATA_4,    1,     0, 4,    0,  MC::TBM8,  MC::PSI46DIG, 311}, // 11
	{ MC::ADP_L3,  3, MC::SDATA_3,    1,     0, 4,    1,  MC::TBM8,  MC::PSI46DIG, 312}, // 12
	{ MC::ADP_L3,  3, MC::SDATA_2,    1,     0, 4,    2,  MC::TBM8,  MC::PSI46DIG, 313}, // 13
	{ MC::ADP_L3,  3, MC::SDATA_1,    1,     0, 4,    3,  MC::TBM8,  MC::PSI46DIG, 314}, // 14
	{ MC::ADP_L3,  3, MC::SDATA_4,    1,     1, 4,    0,  MC::TBM8,  MC::PSI46DIG, 321}, // 15
	{ MC::ADP_L3,  3, MC::SDATA_3,    1,     1, 4,    1,  MC::TBM8,  MC::PSI46DIG, 322}, // 16
	{ MC::ADP_L3,  3, MC::SDATA_2,    1,     1, 4,    2,  MC::TBM8,  MC::PSI46DIG, 323}, // 17
	{ MC::ADP_L3,  3, MC::SDATA_1,    1,     1, 4,    3,  MC::TBM8,  MC::PSI46DIG, 324}, // 18
	{ MC::ADP_L3,  3, MC::SDATA_4,    1,     2, 4,    0,  MC::TBM8,  MC::PSI46DIG, 331}, // 19
	{ MC::ADP_L3,  3, MC::SDATA_3,    1,     2, 4,    1,  MC::TBM8,  MC::PSI46DIG, 332}, // 20
	{ MC::ADP_L3,  3, MC::SDATA_2,    1,     2, 4,    2,  MC::TBM8,  MC::PSI46DIG, 333}, // 21
	{ MC::ADP_L3,  3, MC::SDATA_1,    1,     2, 4,    3,  MC::TBM8,  MC::PSI46DIG, 334}, // 22

	{ MC::ADP_L4,  4, MC::SDATA_4,    1,     0, 4,    0,  MC::TBM8,  MC::PSI46DIG, 411}, // 23
	{ MC::ADP_L4,  4, MC::SDATA_3,    1,     0, 4,    1,  MC::TBM8,  MC::PSI46DIG, 412}, // 24
	{ MC::ADP_L4,  4, MC::SDATA_2,    1,     0, 4,    2,  MC::TBM8,  MC::PSI46DIG, 413}, // 25
	{ MC::ADP_L4,  4, MC::SDATA_1,    1,     0, 4,    3,  MC::TBM8,  MC::PSI46DIG, 414}, // 26
	{ MC::ADP_L4,  4, MC::SDATA_4,    1,     1, 4,    0,  MC::TBM8,  MC::PSI46DIG, 421}, // 27
	{ MC::ADP_L4,  4, MC::SDATA_3,    1,     1, 4,    1,  MC::TBM8,  MC::PSI46DIG, 422}, // 28
	{ MC::ADP_L4,  4, MC::SDATA_2,    1,     1, 4,    2,  MC::TBM8,  MC::PSI46DIG, 423}, // 29
	{ MC::ADP_L4,  4, MC::SDATA_1,    1,     1, 4,    3,  MC::TBM8,  MC::PSI46DIG, 424}, // 30
	{ MC::ADP_L4,  4, MC::SDATA_4,    1,     2, 4,    0,  MC::TBM8,  MC::PSI46DIG, 431}, // 31
	{ MC::ADP_L4,  4, MC::SDATA_3,    1,     2, 4,    1,  MC::TBM8,  MC::PSI46DIG, 432}, // 32
	{ MC::ADP_L4,  4, MC::SDATA_2,    1,     2, 4,    2,  MC::TBM8,  MC::PSI46DIG, 433}, // 33
	{ MC::ADP_L4,  4, MC::SDATA_1,    1,     2, 4,    3,  MC::TBM8,  MC::PSI46DIG, 434}, // 34
	{ MC::ADP_L4,  4, MC::SDATA_4,    1,     3, 4,    0,  MC::TBM8,  MC::PSI46DIG, 441}, // 35
	{ MC::ADP_L4,  4, MC::SDATA_3,    1,     3, 4,    1,  MC::TBM8,  MC::PSI46DIG, 442}, // 36
	{ MC::ADP_L4,  4, MC::SDATA_2,    1,     3, 4,    2,  MC::TBM8,  MC::PSI46DIG, 443}, // 37
	{ MC::ADP_L4,  4, MC::SDATA_1,    1,     3, 4,    3,  MC::TBM8,  MC::PSI46DIG, 444}, // 38

	{ MC::ADP_NO,  0, MC::SDATA_L1,   4,    -1, 1,    0,  MC::TBM10, MC::PROC600,  0},   // MODTYPE_L1
	{ MC::ADP_NO,  0, MC::SDATA_L2,   2,    -1, 1,    0,  MC::TBM9,  MC::PSI46DIG, 0},   // MODTYPE_L2
	{ MC::ADP_NO,  0, MC::SDATA_L34,  1,    -1, 1,    0,  MC::TBM8,  MC::PSI46DIG, 0}    // MODTYPE_L34

};


void CModType::SetHubAddr(int hub)
{
	if (Get().tbm == MC::TBM10) tb.mod_Addr(hub+1, hub);
	else tb.mod_Addr(hub);
}


void CModType::SetRocAddr(int roc)
{
	tb.roc_I2cAddr(roc);
}



// === CPower power control =================================================

/*
	bool SingleMode;
	bool PGOn[6];
*/

const double CPower::VD_MODULE = 2.7;  // V
const double CPower::VD_SLOPE  = 0.07; // V/module
const double CPower::ID_0      = 0.2;  // A
const double CPower::ID_MODULE = 0.6;  // A/module

const double CPower::VA_MODULE = 1.7;  // V
const double CPower::VA_SLOPE  = 0.05; // V/module
const double CPower::IA_0      = 0.15; // A
const double CPower::IA_MODULE = 0.5;  // A/module

const double CPower::VD_CB     = 2.6;  // V
const double CPower::ID_CB     = 0.4;  // A


CPower::CPower()
{
	singleMode = true;
	for (int i=0; i<6; i++) PGOn[i] = false;
}



void CPower::ModPon(CModType mod, int msDelay)
{
	int grp = mod.Get().powerGrp;

	if (grp >= 0)
	{ // multi module test on STB
		if (singleMode)
		{
			// do nothing if group is already on
			if (PGOn[grp]) return;

			// switch off all active power groups
			for (int gi=0; gi<6; gi++) if (PGOn[gi])
			{
				tb.stb_Poff(gi);
				PGOn[gi] = false;
			}
			
			// switch on
			int grpsize = mod.Get().powerGrpSize;
			tb.stb_SetVD(grp, VD_MODULE + VD_SLOPE*grpsize);
			tb.stb_SetID(grp, ID_0      + ID_MODULE*grpsize);
			tb.stb_SetVA(grp, VA_MODULE + VA_SLOPE*grpsize);
			tb.stb_SetIA(grp, IA_0      + IA_MODULE*grpsize);
			tb.stb_Pon(grp);
			PGOn[grp] = true;
		}
		else
		{
			// switch on all power groups
			int adpt = mod.Get().adapter;
			for (int i=0; i<=41; i++)
			{
				CModType m = i;
				if (m.Get().adapter == adpt)
				{
					int pgrp = m.Get().powerGrp;
					if (!PGOn[pgrp])
					{
						int grpsize = m.Get().powerGrpSize;
						tb.stb_SetVD(pgrp, VD_MODULE + VD_SLOPE*grpsize);
						tb.stb_SetID(pgrp, ID_0      + ID_MODULE*grpsize);
						tb.stb_SetVA(pgrp, VA_MODULE + VA_SLOPE*grpsize);
						tb.stb_SetIA(pgrp, IA_0      + IA_MODULE*grpsize);
						tb.stb_Pon(pgrp);
						PGOn[pgrp] = true;
						tb.mDelay(50);
					}
				}
			}
		}
	}
	else
	{ // single module test on DTB
		tb.SetVD(VD_MODULE + VD_SLOPE);
		tb.SetID(ID_0      + ID_MODULE);
		tb.SetVA(VA_MODULE + VA_SLOPE);
		tb.SetIA(IA_0      + IA_MODULE);
		tb.Pon();
	}
	tb.mDelay(msDelay);
}


void CPower::ModPoff(CModType mod)
{
	if (mod.Get().NeedsSTB())
	{
		for (int i=0; i<6; i++)
		{
			tb.stb_Poff(i);
			PGOn[i] = false;
		}
	}
	else tb.Poff();
}


void CPower::ModPoffAll()
{
	if (tb.stb_IsPresent())
	{
		for (int i = 0; i<6; i++)
		{
			tb.stb_Poff(i);
			PGOn[i] = false;
		}
	}
	tb.Poff();
}


void CPower::CbPon()
{
	if (tb.stb_IsPresent())
	{
		tb.SetVD(VD_CB);
		tb.SetID(ID_CB);
		tb.Pon();
		tb.mDelay(500);
	}
}


void CPower::CbPoff()
{
	if (tb.stb_IsPresent())
	{
		tb.Poff();
	}
}


double CPower::GetVD(CModType mod)
{
	if (mod.Get().NeedsSTB())
		return tb.stb_GetVD(mod.Get().powerGrp);

	return tb.GetVD();
}


double CPower::GetID(CModType mod)
{
	int grp = mod.Get().powerGrp;
	if (grp >= 0)
		return tb.stb_GetID(grp);

	return tb.GetID();
}


double CPower::GetVA(CModType mod)
{
	int grp = mod.Get().powerGrp;
	if (grp >= 0)
		return tb.stb_GetVA(grp);

	return tb.GetVA();
}


double CPower::GetIA(CModType mod)
{
	int grp = mod.Get().powerGrp;
	if (grp >= 0)
		return tb.stb_GetIA(grp);

	return tb.GetIA();
}


double CPower::GetVC()
{
	return tb.GetVD();
}


double CPower::GetIC()
{
	return tb.GetID();
}



// === CDataChannel =========================================================

void CDataChannel::Open(CModType roc, int ch)
{
	channel = ch;
	if (roc.Get().roc == MC::PROC600) dec = &decLin; else dec = &decDig;
	src >> raw >> *dec >> dummy >> pump;
	a_first = &dummy;
	rawPipeInserted = false;
	isOpen = src.OpenModDig(tb, channel, false, 100000);
}


void CDataChannel::AddPipe(CAnalyzer &a)
{
	if (isOpen)
	{
		if (a_first == &dummy) *dec >> a >> pump;
		else (*a_first) >> a >> pump;
		a_first = &a;
	}
}


void CDataChannel::AddPipe(CDataPipe<CDataRecord*> &a)
{
	if (isOpen && !rawPipeInserted)
	{
		raw >> a >> *dec;
		rawPipeInserted = true;
	}
}


void CDataChannel::RemoveAllPipes()
{
	if (isOpen)
	{
		src >> raw >> *dec >> dummy >> pump;
		rawPipeInserted = false;
		a_first = &dummy;
	}
}


void CDataChannel::GetAll()
{
	try	{ pump.GetAll(); }
	catch (DataPipeException) {}
}



// === CDataProcessing ======================================================

CDataProcessing::CDataProcessing() : nChannels(0)
{
	for (unsigned int i=0; i<8; i++) debug_event[i] = 0;
}


CDataProcessing::CDataProcessing(CModType mod) : nChannels(0)
{
	for (unsigned int i=0; i<8; i++) debug_event[i] = 0;
	Open(mod);
}


void CDataProcessing::Open(CModType mod)
{
	m = mod;
	if (m.Get().NeedsSTB()) tb.stb_SetSdata(m.GetSel());

	switch (m.Get().sdata)
	{
	case MC::SDATA_4321:
		c[0].Open(mod, 0); // ROC 0, 1
		c[1].Open(mod, 1); // ROC 2, 3
		c[2].Open(mod, 6); // ROC 4, 5
		c[3].Open(mod, 7); // ROC 6, 7
		c[4].Open(mod, 4); // ROC 8, 9
		c[5].Open(mod, 5); // ROC 10, 11
		c[6].Open(mod, 2); // ROC 12, 13
		c[7].Open(mod, 3); // ROC 14, 15
		nChannels = 8;
		break;
	case MC::SDATA_43:
		c[0].Open(mod, 6); // ROC 0, 1, 2, 3
		c[1].Open(mod, 7); // ROC 4, 5, 6, 7
		c[2].Open(mod, 4); // ROC 8, 9, 10, 11
		c[3].Open(mod, 5); // ROC 12, 13, 14, 15
		nChannels = 4;
		break;
	case MC::SDATA_21:
		c[0].Open(mod, 2); // ROC 0, 1, 2, 3
		c[1].Open(mod, 3); // ROC 4, 5, 6, 7
		c[2].Open(mod, 0); // ROC 8, 9, 10, 11
		c[3].Open(mod, 1); // ROC 12, 13, 14, 15
		nChannels = 4;
		break;
	case MC::SDATA_4:
		c[0].Open(mod, 6); // ROC 0..7
		c[1].Open(mod, 7); // ROC 8..15
		nChannels = 2;
		break;
	case MC::SDATA_3:
		c[0].Open(mod, 4); // ROC 0..7
		c[1].Open(mod, 5); // ROC 8..15
		nChannels = 2;
		break;
	case MC::SDATA_2:
		c[0].Open(mod, 2); // ROC 0..7
		c[1].Open(mod, 3); // ROC 8..15
		nChannels = 2;
		break;
	case MC::SDATA_1:
		c[0].Open(mod, 0); // ROC 0..7
		c[1].Open(mod, 1); // ROC 8..15
		nChannels = 2;
		break;
	case MC::SDATA_L1:
		c[0].Open(mod, 6); // ROC 0, 1
		c[1].Open(mod, 7); // ROC 2, 3
		c[2].Open(mod, 0); // ROC 4, 5
		c[3].Open(mod, 1); // ROC 6, 7
		c[4].Open(mod, 2); // ROC 8, 9
		c[5].Open(mod, 3); // ROC 10, 11
		c[6].Open(mod, 4); // ROC 12, 13
		c[7].Open(mod, 5); // ROC 14, 15
		nChannels = 8;
		break;
	case MC::SDATA_L2:
		c[0].Open(mod, 0); // ROC 0, 1, 2, 3
		c[1].Open(mod, 1); // ROC 4, 5, 6, 7
		c[2].Open(mod, 2); // ROC 8, 9, 10, 11
		c[3].Open(mod, 3); // ROC 12, 13, 14, 15
		nChannels = 4;
		break;
	case MC::SDATA_L34:
		c[0].Open(mod, 0); // ROC 0..7
		c[1].Open(mod, 1); // ROC 8..15
		nChannels = 2;
		break;
	}
}


void CDataProcessing::AddPipe(unsigned int channel, CAnalyzer &a)
{
	if (channel < nChannels) c[channel].AddPipe(a);
}


void CDataProcessing::AddPipe(unsigned int channel, CDataPipe<CDataRecord*> &a)
{
	if (channel < nChannels) c[channel].AddPipe(a);
}


void CDataProcessing::RemoveAllPipes()
{
	for (unsigned int i=0; i<nChannels; i++)
		c[i].RemoveAllPipes();
}


void CDataProcessing::Close()
{
	for (unsigned int i=0; i<nChannels; i++)
	{
		c[i].Close();
		if (debug_event[i])
		{
			delete debug_event[i];
			debug_event[i] = 0;
		}
	}
}


void CDataProcessing::EnableLogging(const char *prefix)
{
	for (unsigned int i=0; i<nChannels; i++)
	{
		char filename[128];
		sprintf(filename, "%s%u.txt", prefix, i);
		debug_event[i] = new CEventPrinter(filename);
		c[i].AddPipe(*(debug_event[i]));
	}
}


bool CDataProcessing::GetVsdata(unsigned int sdata, double &vp, double &vn)
{
	static const int T4321[4] = {3, 2, 1, 0};
	static const int T43[4]   = {3, 2,-1,-1};
	static const int T21[4]   = {1, 0,-1,-1};
	static const int T4[4]    = {3,-1,-1,-1};
	static const int T3[4]    = {2,-1,-1,-1};
	static const int T2[4]    = {1,-1,-1,-1};
	static const int T1[4]    = {0,-1,-1,-1};

	if (sdata > 3) return false;

	int i;

	switch(m.Get().sdata)
	{
	case MC::SDATA_4321: i = T4321[sdata]; break;
	case MC::SDATA_43:   i = T43[sdata]; break;
	case MC::SDATA_21:   i = T21[sdata]; break;
	case MC::SDATA_4:    i = T4[sdata]; break;
	case MC::SDATA_3:    i = T3[sdata]; break;
	case MC::SDATA_2:    i = T2[sdata]; break;
	case MC::SDATA_1:    i = T1[sdata]; break;
	default: i = -1;
	}

	if (i < 0) return false;

	vp = tb.stb_GetVSdata(i, true);
	vn = tb.stb_GetVSdata(i, false);
	return true;
}


void CDataProcessing::SendReset()
{
	tb.Pg_SetCmd(0, PG_REST);
	tb.Pg_Single();
	tb.uDelay(10);
}


void CDataProcessing::SendTrigger(unsigned int count)
{
	tb.Pg_SetCmd(0, PG_TRG);
	for (unsigned int i=0; i<count; i++)
	{
		tb.Pg_Single();
		tb.uDelay(10);
	}
}


void CDataProcessing::TakeData(unsigned nTrigger)
{
	Enable();
	SendTrigger(nTrigger);
	Disable();
	GetAll();
}



// === CSignalQuality =======================================================

CEvent* CSignalQuality::Read()
{
	x = Get();

	if (x->roc.size() != 0)	Add(x->roc[0].header >> 4);
		
	return x;
};


void CSignalQuality::Add(unsigned int xor)
{
	unsigned int i, mask;
	for (i=0, mask = 0x01; i<8; i++, mask <<= 1)
	{
		if (xor & mask)	{ n++; histo[i]++; }
	}
}


void CSignalQuality::Clear()
{
	n = 0;
	for (int i=0; i<8; i++) histo[i] = 0;
}


void CSignalQuality::CalculateQP()
{
	static const double sp4 = 1.0/std::sqrt(2.0);
	static const double sc[10] = { 0.0, sp4, 1.0, sp4, 0.0, -sp4, -1.0, -sp4, 0.0, sp4 };

	if (n)
	{
		// calculate mean and var (cyclic)
		double re = 0.0, im = 0.0;
		for (int i=0; i<8; i++)
		{
			re  += histo[i]*sc[i+2];
			im  += histo[i]*sc[i];
		}
		re /= n; im /= n;
		phase   = fmod(std::atan2(im,re)*4/M_PI+8.0, 8.0);
		quality = sqrt(re*re + im*im);
	} else phase = quality = 0.0;
}


void CSignalQuality::Print()
{
	if (n)
	{
		Log.printf("Q = %4.2f; Ph = %3.1f; <", quality, phase);
		for (int i=0; i<8; i++) Log.printf(" %3u", histo[i]);
		Log.printf(">\n");
	}
	else Log.printf("no data\n");
}



// === CReadback ============================================================

void CRdbValue::Add(unsigned int v)
{
	shift <<= 1;
	if (v&1) shift++; // data bit set
	if (v&2) // start bit set
	{
		value = shift;
		shift = 0;
	}
}


CEvent* CReadback::Read()
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
			} else rdb[r].Clear();
		}
		if (f)
		{

		}
	}
	else { for (r = 0; r < 8; r++) rdb[r].value = -1; }
	return x;
}


void CReadback::Print()
{
	for (int r=0; r<8; r++)
	{
		int v = rdb[r].value;
		if (v >= 0) Log.printf(" %2i(%2i:%3i)", v>>12, (v>>8) & 0xf, v & 0xff);
	}
	Log.printf("\n");
}


} // namespace stb

