// stb.cpp
//
// Sector Test Board

#include "psi46test.h"
#include "stb_test.h"
#include "profiler.h"


namespace stb {


CSlot  slot;


// ==========================================================================


void Sectortest(const char *name)
{ PROFILING
	slot.Test(name);
}


void StartAllModules(int id_min, int id_max)
{ PROFILING
/*	int adapter = 3;
	int layer   = 1;
	CModType mod;
	bool stbPresent = tb.stb_IsPresent();

	// --- check slot/module name
	if (stbPresent)
	{ // if STB present test via connector board
		adapter = tb.stb_GetAdapterId();
		switch (adapter)
		{
			case ADP_L12:
				layer = 1;
				if (id_min < 0) id_min = 0;	else if (id_max > 10) id_max = 10;
				if (id_max < id_min) { printf("No layer 1 or 2 module selected!\n"); return; }
				break;
			case ADP_L3:  layer = 3;
				if (id_min < 11) id_min = 11; else if (id_max > 22) id_max = 22;
				if (id_max < id_min) { printf("No layer 3 module selected!\n"); return; }
				break;
			case ADP_L4:
				layer = 4;
				if (id_min < 23) id_min = 23; else if (id_max > 38) id_max = 38;
				if (id_max < id_min) { printf("No layer 4 module selected!\n"); return; }
				break;
			default:
				printf("No STB adapter connected!\n");
				return;
		}
	}
	else
	{ // if STB not present test via module adapter
		if (id_min < 39) id_min = 39; else if (id_max > 41) id_max = 41;
		if (id_max < id_min) { printf("No module selected!\n"); return; }
	}

	Test_Init();


	list<ModuleData>::iterator k;
	for (k = result_slot.module.begin(); k != result_slot.module.end(), k++)
	{
		int id = k->mod.Get().GetSel();
		if (id_min <= id && id <= id_max)
		{
			mod = id;
			int hub = k->hub;
			if (hub >= 0)
			{
				mod.SetHubAddr(hub);
				
			}
		}
	}


	for (int i=id_min; i<=id_max; i++)
	{
		mod = i;
		list<ModuleData>::iterator k;
		for (k = result_slot.module.begin(); k != result_slot.module.end(), k++)
		{
			k->mod.Get().GetSel();
		}

		result_slot.module
		int hub = result_module.hub;
		if (result_module.hub >= 0)
		{
			mod.SetHubAddr(result_module.hub);
			modSettings.InitModule(mod);
		}
		printf(" %i", i);
	}

	printf("\n");
	*/
}


} // namespace stb
