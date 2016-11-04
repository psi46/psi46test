// modulesettings.h

#pragma once

#include "stb_tools.h"

class CTbmSettings
{
public:
	unsigned int phase0; // 1st TBM
	unsigned int phase1; // 2nd TBM for Layer 1 modules

	void SetDefault(stb::CModType mod);
	void SetPhase160(unsigned int ph160, unsigned int tbm = 0);
	void SetPhase400(unsigned int ph400, unsigned int tbm = 0);
};


class CModuleSettings
{
	CTbmSettings module[42];
public:
	unsigned int delayA; // 1_1_100_100
	unsigned int delayB;
public:
	CModuleSettings() { SetDefault(); }
	void SetDefault();
	void SetPhase160(stb::CModType mod, unsigned int ph160, unsigned int tbm = 0);
	void SetPhase400(stb::CModType mod, unsigned int ph400, unsigned int tbm = 0);

	void InitTBM(stb::CModType mod, unsigned int tbm = 0);
	void UpdateTBM(stb::CModType mod);
	void InitROC(stb::CModType mod);
	void InitModule(stb::CModType mod);
	void InitAllModules(stb::CModType mod);
};

extern CModuleSettings modSettings;
