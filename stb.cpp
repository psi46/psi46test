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


void HvTest()
{	PROFILING
	if (tb.stb_IsPresent())
	{
		if (slot.module.size() == 0)
		{
			printf("Run sectortest first!\n");
			return;
		}
		slot.StartAllModules();
		tb.HVon();
		for (int i = 0; i <= 6; i++) tb.stb_HVon(i);
		tb.Flush();
	}
}


} // namespace stb
