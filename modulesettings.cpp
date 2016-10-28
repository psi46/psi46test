// modulesettings.cpp

#include "psi46test.h"
#include "modulesettings.h"


CModuleSettings modSettings;


void CModuleSettings::SetDelay(unsigned int br, unsigned int value)
{
	if (value > 7) value = 7;

	switch (br)
	{
	case 0:
		delayA = (delayA & 0xf8) + value;
		tb.tbm_Set( 0xea, delayA);
		break;
	case 1:
		delayA = (delayA & 0xc7) + (value<<3);
		tb.tbm_Set( 0xea, delayA);
		break;
	case 2:
		delayB = (delayA & 0xf8) + value;
		tb.tbm_Set( 0xfa, delayB);
		break;
	case 3:
		delayB = (delayA & 0xc7) + (value<<3);
		tb.tbm_Set( 0xfa, delayB);
		break;
	}
}


void CModuleSettings::SeTokenDelayA(unsigned int value)
{
	if (value) delayA |=  0x80;
	else       delayA &= ~0x80;
	tb.tbm_Set( 0xea, delayA);
}


void CModuleSettings::SeTokenDelayB(unsigned int value)
{
	if (value) delayB |=  0x80;
	else       delayB &= ~0x80;
	tb.tbm_Set( 0xfa, delayB);
}


void CModuleSettings::SetHdrTrlDelayA(unsigned int value)
{
	if (value) delayA |=  0x40;
	else       delayA &= ~0x40;
	tb.tbm_Set( 0xea, delayA);
}


void CModuleSettings::SetHdrTrlDelayB(unsigned int value)
{
	if (value) delayB |=  0x40;
	else       delayB &= ~0x40;
	tb.tbm_Set( 0xfa, delayB);
}


void CModuleSettings::SetPhase160(unsigned int value)
{
	if (value > 7) value = 7;

	phase = (phase & 0x1f) + (value<<5);
	tb.tbm_Set( 0xea, phase);
}


void CModuleSettings::SetPhase400(unsigned int value)
{
	if (value > 7) value = 7;

	phase = (phase & 0xe3) + (value<<2);
	tb.tbm_Set( 0xea, phase);
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


void CModuleSettings::InitTBM(stb::CModType mod)
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
	tb.tbm_Set( 0xee, phase);

	// Temp measurement control
	tb.tbm_Set( 0xfe, 0x00);
}


void CModuleSettings::InitModule(stb::CModType mod)
{ PROFILING
	if (mod.Get().tbm == stb::MC::TBM10)
	{
		mod.SetRocAddr(0); // switch to TBM n+1
		InitTBM(mod);
		mod.SetRocAddr(4); // switch to TBM n
		InitTBM(mod);

	}
	else InitTBM(mod);
	
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
