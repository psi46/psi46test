// stb_test.h

#pragma once

#include "stb_tools.h"


namespace stb
{


struct SigSdata
{
	Value<double> vp, vn; // 78 mV
	Value<int> idle; // 0 = ok, 1 = no transitions, 2 = transitions but no 2-gap
	Value<double> quality;
	Value<double> phase;
	void Invalidate();
	const char* Report();
};


class CModule
{
public:
	// --- fixed module characteristics
	CModType mod;

	Value<int> hub; // TBM hub address (layer 1: hub, hub+1)

	Value<double> vd_pon, va_pon, id_pon, ia_pon;
	Value<double> vd, va, id, ia;

	SigSdata sdata[4];

	Value<int> roc_present[16];

	// --- individual TBM parameters
	unsigned int tbm_phase0; // 1st TBM
	unsigned int tbm_phase1; // 2nd TBM for Layer 1 modules
	unsigned int tbm_delay[4]; // (TBM n, Reg e7) (TBM n, Reg f7) (TBM n+1, Reg e7) (TBM n+1, Reg f7)

	void SetTBM_Default();
	void SetTBM_Phase160(unsigned int ph160, unsigned int tbm = 0);
	void SetTBM_Phase400(unsigned int ph400, unsigned int tbm = 0);
	// L1: 0=ROC01, 1=ROC23, 2=ROC45, 3=ROC67, 4=ROC89, 5=ROCAB, 6=ROCCD, 7=ROCEF 
	// L234: 0=ROC0123, 1=ROC4567, 2=ROC89AB, 3=ROCCDEF
	void SetTBM_Delay(unsigned int delay, unsigned int group);

	// --- Module test
private:
	void _InitModule();
	void UpdateTBM();
	void InitTBM(unsigned int tbm = 0);
	void InitROC();

	bool Test_PowerOn();
	void Test_Init();
	void Test_Sdata();
	bool Find_HubAddress(bool debug = false);
	void Test_RocProgramming();

public:
	CModule(int selector) : mod(selector) { SetTBM_Default(); }
	void InitModule(bool forceAllHubs = false);
	void Invalidate();
	bool Report();
	void Test();

	void DelayScan(unsigned int group);
};


class CSlot
{
public:
	string name;
	Value<double> cb_voltage;
	Value<double> cb_current;
	Value<double> cb_current_lo;
	Value<double> cb_current_hi;

	list<CModule> module;

	// Slot test
private:
	double GetIC();

	void Test_Init();
	bool Test_ConnectorBoard();
	void _Test();

	void RdaSingleTest(int sel, int hub, CModType m);
public:
	void Invalidate();
	void Report();
	void Test(const string &slotName);

	void StartAllModules();
	void StopAllModules();
	void RdaTest(int sel = -1, int hub = -1);
	void AssignHub(unsigned int sel, int hub = -1);

	CModule* FindModuleBySelector(unsigned int sel);
	void DelayScan(unsigned int sel, unsigned int group);
};


} // namespace stb
