// modulesettings.cpp

#include "psi46test.h"
#include "modulesettings.h"


CModuleSettings modSettings;


/*
DELAYA = 0xed; // 1_1_101_101
DELAYB = 0xed; // 1_1_101_101
PHASE  = 0x24; // 001_001_--  160 MHz, 400 MHz

DELAYA = 0x6d; // 0_1_101_101
DELAYB = 0x6d; // 0_1_101_101
PHASE  = 0x2d; // 001_011_01  160 MHz, 400 MHz
*/

void CTbmSettings::SetDefault(stb::CModType mod)
{
	phase0 = phase1 = 0x24; // 001_001_00  160 MHz, 400 MHz
}


void CModuleSettings::SetDefault()
{
	delayA = delayB = 0x00; // 0x6d:0_1_101_101; 0xed:1_1_101_101
	for (int i=0; i<42; i++)
	{
		stb::CModType mod = i;
		module[i].SetDefault(mod);
	}
}


void CTbmSettings::SetPhase160(unsigned int ph160, unsigned int tbm)
{
	if (ph160 > 7) ph160 = 7;

	if (tbm == 0) phase0 = (phase0 & 0x1f) + (ph160 << 5);
	else          phase1 = (phase1 & 0x1f) + (ph160 << 5);
}


void CTbmSettings::SetPhase400(unsigned int ph400, unsigned int tbm)
{
	if (ph400 > 7) ph400 = 7;

	if (tbm == 0) phase0 = (phase0 & 0xe3) + (ph400 << 2);
	else          phase1 = (phase1 & 0xe3) + (ph400 << 2);
}


void CModuleSettings::SetPhase160(stb::CModType mod, unsigned int ph160, unsigned int tbm)
{
	int id = mod.GetSel();
	if (0 <= id && id < 42)	module[id].SetPhase160(ph160, tbm);
}


void CModuleSettings::SetPhase400(stb::CModType mod, unsigned int ph400, unsigned int tbm)
{
	int id = mod.GetSel();
	if (0 <= id && id < 42)	module[id].SetPhase400(ph400, tbm);
}


void CModuleSettings::UpdateTBM(stb::CModType mod)
{ PROFILING
	int id = mod.GetSel();
	if (!(0 <= id && id < 42)) return;

	if (mod.Get().tbm == stb::MC::TBM10)
	{
		mod.SetRocAddr(0); // switch to TBM n+1
		tb.tbm_Set( 0xea, delayA);
		tb.tbm_Set( 0xfa, delayB);
		tb.tbm_Set( 0xee, module[id].phase1);

		mod.SetRocAddr(4); // switch to TBM n
		tb.tbm_Set( 0xea, delayA);
		tb.tbm_Set( 0xfa, delayB);
		tb.tbm_Set( 0xee, module[id].phase0);
	}
	else
	{
		tb.tbm_Set( 0xea, delayA);
		tb.tbm_Set( 0xfa, delayB);
		tb.tbm_Set( 0xee, module[id].phase0);
	}
}



void CModuleSettings::InitROC(stb::CModType mod)
{ PROFILING
	if (mod.Get().roc == stb::MC::PROC600)
	{ // PROC600
		tb.roc_SetDAC(  1,  10); // Vdig
		tb.roc_SetDAC(  2,  50);
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
		tb.roc_SetDAC(  2,  70);
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


void CModuleSettings::InitTBM(stb::CModType mod, unsigned int tbm)
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
	tb.tbm_Set( 0xea, delayA);
	tb.tbm_Set( 0xfa, delayB);

	// Auto reset rate (x+1)*256
	tb.tbm_Set( 0xec, 128);
	tb.tbm_Set( 0xfc, 128);

	// 160/400 MHz phase adjust
	int id = mod.GetSel();
	tb.tbm_Set( 0xee, (tbm==0)? module[id].phase0 : module[id].phase1);

	// Temp measurement control
	tb.tbm_Set( 0xfe, 0x00);
}


void CModuleSettings::InitModule(stb::CModType mod)
{ PROFILING
	if (mod.Get().tbm == stb::MC::TBM10)
	{
		mod.SetRocAddr(0); // switch to TBM n+1
		InitTBM(mod, 1);
		mod.SetRocAddr(4); // switch to TBM n
		InitTBM(mod, 0);
	}
	else
	{
		InitTBM(mod);
	}
	
	for (int roc=0; roc<16; roc++)
	{
		mod.SetRocAddr(roc);
		InitROC(mod); 
	}
}


void CModuleSettings::InitAllModules(stb::CModType mod)
{ PROFILING
	int hub_incr = (mod.Get().tbm == stb::MC::TBM10)? 2 : 1;
	for (int hub = 0; hub < 32; hub += hub_incr)
	{
		mod.SetHubAddr(hub);
		modSettings.InitModule(mod);
	}
}
