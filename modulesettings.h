// modulesettings.h

#pragma once

#include "stb_tools.h"

class CModuleSettings
{
	static const unsigned int DELAYA = 0xed; // 1_1_101_101
	static const unsigned int DELAYB = 0xed; // 1_1_101_101
	static const unsigned int PHASE  = 0x24; // 001_001_--  160 MHz, 400 MHz
/*
	static const unsigned int DELAYA = 0x6d; // 0_1_101_101
	static const unsigned int DELAYB = 0x6d; // 0_1_101_101
	static const unsigned int PHASE  = 0x2d; // 001_011_01  160 MHz, 400 MHz
*/
public:
	unsigned int delayA; // 1_1_100_100
	unsigned int delayB;
	unsigned int phase;  // 001_001_--

	CModuleSettings() : delayA(DELAYA), delayB(DELAYB), phase(PHASE) {}
	
	void SetDefault() { delayA = DELAYA; delayB = DELAYB; phase = PHASE; }
	void SetDelay(unsigned int br, unsigned int value); // 0..7
	void SeTokenDelayA(unsigned int value);   // 0, 1
	void SeTokenDelayB(unsigned int value);   // 0, 1
	void SetHdrTrlDelayA(unsigned int value); // 0, 1
	void SetHdrTrlDelayB(unsigned int value); // 0, 1
	void SetPhase160(unsigned int value);     // 0..7
	void SetPhase400(unsigned int value);     // 0..7

	void InitTBM(stb::CModType mod);
	void InitROC(stb::CModType mod);
	void InitModule(stb::CModType mod);
	void InitAllModules(stb::CModType mod);
};

extern CModuleSettings modSettings;
