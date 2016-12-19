// stb.cpp
//
// Sector Test Board

#include "psi46test.h"
#include "stb_test.h"
#include "profiler.h"


namespace stb {


CSlot  slot;


void Sectortest(const char *name)
{ PROFILING
	slot.Test(name);
}


bool StartModules()
{
	if (!tb.stb_IsPresent())
	{
		printf("STB not present!\n");
		return false;
	}

	if (slot.module.size() == 0)
	{
		printf("Run sectortest first!\n");
		return false;
	}

	slot.StartAllModules();
	tb.Flush();

	return true;
}


void StopModules()
{
	if (!tb.stb_IsPresent())
	{
		printf("STB not present!\n");
		return;
	}
	slot.StopAllModules();
	tb.Flush();
}


void HvTest()
{	PROFILING
	if (StartModules())
	{
		tb.HVon();
		for (int i = 0; i <= 6; i++) tb.stb_HVon(i);
		tb.Flush();
	}
}


void RdaTest(int sel, int hub)
{
	if (!tb.stb_IsPresent())
	{
		printf("STB not present!\n");
		return;
	}

	if (sel >= 0 && slot.module.size() == 0)
	{
		printf("Run sectortest first!\n");
		return;
	}

	slot.RdaTest(sel, hub);
}


void AssignHub(unsigned int sel, int hub)
{
	slot.AssignHub(sel, hub);
}


void DelayScan(unsigned int sel, unsigned int group)
{
	slot.DelayScan(sel, group);
}


void PhaseScan(unsigned int sel, int hub)
{
	slot.PhaseScan(sel, hub);
}


} // namespace stb


