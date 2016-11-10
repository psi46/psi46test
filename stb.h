// stb.h

#pragma once

#include "stb_tools.h"
#include <list>


namespace stb {


template <class T>
class Value
{
	bool valid;
	T value;
public:
	Value() : valid(false) {}
	bool IsValid() { return valid; };
	void Invalidate() { valid = false; }
	T& Get() { return value; }
	void Put(T& x) { value = x; valid = true; }
	T operator=(T x) { Put(x); return x; }
	operator const T&() { return value; }
};


struct SigSdata
{
	Value<double> vp, vn; // 78 mV
	Value<int> idle; // 0 = ok, 1 = no transitions, 2 = transitions but no 2-gap
	Value<double> quality;
	Value<double> phase;
	void Invalidate();
	const char* Report();
};

struct ModuleData
{
	Value<CModType> mod;
	Value<int> hub;
	Value<double> vd, va, id, ia;
	SigSdata sdata[4];

	Value<int> roc_present[16];

	void Invalidate();
	bool Report();
};


struct SlotData
{
	string name;
	Value<double> cb_voltage;
	Value<double> cb_current;
	Value<double> cb_current_lo;
	Value<double> cb_current_hi;

	list<ModuleData> module;

	SlotData() {}
	void Invalidate();
	void Report();
};


void Roc_Init(int id);

// void Tbm_Init();

void Mod_Init(int hub);

void SetDefaultHub(unsigned int id, int hub);
void Sectortest(const char *name);
void StartAllModules(int id_min, int id_max); // for HV test

} // namespace stb
