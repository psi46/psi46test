

#include "psi46test.h"
#include "chipdatabase.h"


#define VCAL_TEST          35  // 20 50 60 (high range) pixel alive test
                                //   schwelle @ 40MHz = 17
#define VCAL_DCOL_TEST     65  // 30 ... 120

#define VCAL_LEVEL        140	//  135 vcal for level test without sensor
#define VCAL_LEVEL_SENSOR  45	//  45 vcal for level test with sensor
#define VCAL_LEVEL_EXT    150	// 150 vcal for external calibrate
#define VANA0              70   // default vana value
#define VDIG0               4
#define VSH0              150   // 225

// =======================================================================
//  global variables for test results
// =======================================================================

CChip g_chipdata;

const int StdSet[31] =
{ // 40 MHz 
	   4,  // CLK delay
	  19,  // SDA delay (CLK + 15)
	   4,  // CTR delay (CLK + 0)
	   9,  // TIN delay (CLK + 5)
	   0,  // TOUT delay

	   3,  // TCT-WBC offset

	  12,  // Vcomp
	   0,  // Vleak_comp
	   0,  // VrgPr
	  60,  // VwllPr
	   0,  // VrgSh
	  60,  // VwllSh
	 117,  // VhldDel
	  29,  // Vtrim
	  60,  // VthrComp
	  30,  // VIBias_Bus
	   6,  // Vbias_sf
	  40,  // VoffsetOp
	 115,  // VIbiasOp
	 140,  // VoffsetRO
	 115,  // VIon
	 100,  // Vcomp_ADC 100
	 160,  // VIref_ADC 160
	 220,  // VIbias_roc
	  99,  // VIColOr
	   0,  // Vnpix
	   0,  // VsumCol
	   2,  // Vcal
	  30,  // CalDel  
	   0,  // RangeTemp
	  20,  // WBC
};
// =======================================================================
//
//    digital ROC test
//
// =======================================================================


int test_roc(bool &repeat)
{
	repeat = false;

	tb.SetVD(2.5);
	tb.SetID(0.4);
	tb.SetVA(1.5);
	tb.SetIA(0.4);

	int bin = 1;
	tb.roc_I2cAddr(0);
	tb.SetRocAddress(0);

	switch (test_startup(true))
	{
		case ERROR_IMAX:
			bin = 1; // Ueberstrom
			test_cleanup(bin);
			return bin;
		case ERROR_IMIN:
			bin = 2; // kein Strom
			test_cleanup(bin);
			return bin;
	}


	return 0;
}



