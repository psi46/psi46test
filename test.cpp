/* -------------------------------------------------------------
 *
 *  file:        test.cpp
 *
 *  description: test procedures for PSI46V2
 *
 *  author:      Beat Meier
 *  modified:    31.1.2007
 *
 *  rev:         20.4.2006 bump bonder test
 *
 * -------------------------------------------------------------
 */

#include <math.h>
#include "psi46test.h"
#include "analog.h"


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




// =======================================================================
//  initialization
// =======================================================================

int tct_wbc = 0;

int frequencyDivider = 0;

void SetFreqDivider(int MHz)
{
	if (MHz == 0) MHz = settings.clock;

	switch (MHz)
	{
		case 40: frequencyDivider = 0; break;
		case 20: frequencyDivider = 1; break;
		case 10: frequencyDivider = 2; break;
		case  5: frequencyDivider = 3; break;
		default: frequencyDivider = 0;
	}
}



enum
{ 
	INIT_CLK, INIT_SDA, INIT_CTR, INIT_TIN, INIT_TOUT,
	INIT_TRGDELAY,
	INIT_VCOMP,	INIT_LEAK, INIT_RGPR, INIT_WLLPR, INIT_RGSH, INIT_WLLSH,
	INIT_HLDDEL, INIT_TRIM, INIT_THR, INIT_BUS, INIT_SF,
	INIT_OFFSOP, INIT_BIASOP, INIT_RO, INIT_ON,
	INIT_PH, INIT_DAC, INIT_ROC,
	INIT_COLOR, INIT_NPIX, INIT_SUMCOL, INIT_CAL, INIT_CALDEL,
	INIT_RTEMP, INIT_WBC
 };


const int StdSet[31][4] =
{   // 40   20   10    5  MHz 
	{   0,   0,   0,   0},  // CLK delay
	{  21,  40,  75, 150},  // SDA delay
	{  13,   0,   0,   0},  // CTR delay
	{  13,  25,  40,  80},  // TIN delay
	{   0,   0,   0,   0},  // TOUT delay

	{   3,   1,  -1,  -1},  // TCT-WBC offset

	{  12,  10,  10,  10},  // Vcomp
	{   0,   0,   0,   0},  // Vleak_comp
	{   0,   0,   0,   0},  // VrgPr
	{  60,  35,  35,  35},  // VwllPr
	{   0,   0,   0,   0},  // VrgSh
	{  60,  35,  35,  35},  // VwllSh
	{ 117, 160, 160, 160},  // VhldDel
	{  29,   7,   7,   7},  // Vtrim
	{  60,  70,  70,  70},  // VthrComp
	{  30,  30,  30,  30},  // VIBias_Bus
	{   6,  10,  10,  10},  // Vbias_sf
	{  40,  50,  50,  50},  // VoffsetOp
	{ 115, 115, 115, 115},  // VIbiasOp
	{ 140, 120, 120, 120},  // VoffsetRO
	{ 115, 115, 115, 115},  // VIon
	{ 100, 100, 100, 100},  // Vcomp_ADC 100
	{ 160, 160, 160, 160},  // VIref_ADC 160
	{ 220, 220, 220, 220},  // VIbias_roc
	{  99,  99,  99,  99},  // VIColOr
	{   0,   0,   0,   0},  // Vnpix
	{   0,   0,   0,   0},  // VsumCol
	{   2,   2,   2,   2},  // Vcal
	{  30,  90,  40,  10},  // CalDel  
	{   0,   0,   0,   0},  // RangeTemp
	{  20,  20,  20,  20}   // WBC
};


void WriteSettings()
{
	int f=frequencyDivider, i;
	Log.section("SETTINGS");
	Log.printf("f=%i\n", f);
	for (i=0; i<31; i++) Log.printf(" %4i", StdSet[i][f]);
}


void InitDAC(bool reset)
{
	int f = frequencyDivider;

	if (reset) g_chipdata.InitVana = VANA0;
	                            //         20  MHz
	tb.roc_SetDAC( Vdig,        VDIG0);  //   8
	tb.roc_SetDAC( Vana, g_chipdata.InitVana);
	tb.roc_SetDAC( Vsh,         VSH0);   // 255

	tb.roc_SetDAC( Vcomp,        StdSet[INIT_VCOMP ][f]);
	tb.roc_SetDAC( Vleak_comp,   StdSet[INIT_LEAK  ][f]);
	tb.roc_SetDAC( VrgPr,        StdSet[INIT_RGPR  ][f]);
	tb.roc_SetDAC( VwllPr,       StdSet[INIT_WLLPR ][f]);
	tb.roc_SetDAC( VrgSh,        StdSet[INIT_RGSH  ][f]);
	tb.roc_SetDAC( VwllSh,       StdSet[INIT_WLLSH ][f]);
	tb.roc_SetDAC( VhldDel,      StdSet[INIT_HLDDEL][f]);
	tb.roc_SetDAC( Vtrim,        StdSet[INIT_TRIM  ][f]);
	tb.roc_SetDAC( VthrComp,     StdSet[INIT_THR   ][f]);
	tb.roc_SetDAC( VIBias_Bus,   StdSet[INIT_BUS   ][f]);
	tb.roc_SetDAC( Vbias_sf,     StdSet[INIT_SF    ][f]);
	tb.roc_SetDAC( VoffsetOp,    StdSet[INIT_OFFSOP][f]);
	tb.roc_SetDAC( VIbiasOp,     StdSet[INIT_BIASOP][f]);
	tb.roc_SetDAC( VoffsetRO,    StdSet[INIT_RO    ][f]);
	tb.roc_SetDAC( VIon,         StdSet[INIT_ON    ][f]);
	tb.roc_SetDAC( VIbias_PH,    StdSet[INIT_PH    ][f]);
	tb.roc_SetDAC( Ibias_DAC,    StdSet[INIT_DAC   ][f]);
	tb.roc_SetDAC( VIbias_roc,   StdSet[INIT_ROC   ][f]);
	tb.roc_SetDAC( VIColOr,      StdSet[INIT_COLOR ][f]);
	tb.roc_SetDAC( Vnpix,        StdSet[INIT_NPIX  ][f]);
	tb.roc_SetDAC( VsumCol,      StdSet[INIT_SUMCOL][f]);
	tb.roc_SetDAC( Vcal,         StdSet[INIT_CAL   ][f]);
	tb.roc_SetDAC( CalDel,       StdSet[INIT_CALDEL][f]);
	tb.roc_SetDAC( RangeTemp,    StdSet[INIT_RTEMP ][f]);
	tb.roc_SetDAC( WBC,          StdSet[INIT_WBC   ][f]);
	tb.roc_SetDAC( CtrlReg,    0x00);  // x00
	tb.Flush();
}


void InitChip()
{
	InitDAC(true);
	tb.roc_ClrCal();
	tb.roc_Chip_Mask();
	tb.Flush();
}


void SetMHz(int MHz = 0)
{
	SetFreqDivider(MHz);
	int f = frequencyDivider;

	const int MHZ[] = {MHZ_40, MHZ_20, MHZ_10, MHZ_5};
	tb.SetClock(MHZ[f]);
	tb.SetDelay(SIGNAL_CLK,  StdSet[INIT_CLK ][f]);
	tb.SetDelay(SIGNAL_SDA,  StdSet[INIT_SDA ][f]);
	tb.SetDelay(SIGNAL_CTR,  StdSet[INIT_CTR ][f]);
	tb.SetDelay(SIGNAL_TIN,  StdSet[INIT_TIN ][f]);
	tb.SetDelay(SIGNAL_TOUT, StdSet[INIT_TOUT][f]);
	tb.Flush();

	tct_wbc = StdSet[INIT_TRGDELAY][f];
}




// =======================================================================
//  chip startup
// =======================================================================

#define ERROR_IMAX  1
#define ERROR_IMIN  2

int test_startup(bool probecard)
{
	double Idig = 0.0, Iana = 0.0;

	// power on, supply current limits
	Log.section("PON", false);
	Log.printf(" %i\n", nEntry);
	g_chipdata.nEntry = nEntry;
	tb.Pon();
	tb.mDelay(400);

	g_chipdata.IdigOn = Idig = tb.GetID()*1000.0;
	Log.printf("Idig=%6.2lf mA\n", Idig);

	g_chipdata.IanaOn = Iana = tb.GetIA()*1000;
	Log.printf("Iana=%6.2lf mA\n", Iana);

	if (Idig >120.0 || Iana >120.0) return ERROR_IMAX;

	// initialize, supply currents
	Log.section("INIT");
	InitChip();
	tb.mDelay(300);

	g_chipdata.IdigInit = Idig = tb.GetID()*1000.0;
	Log.printf("Idig=%6.2lf mA\n", Idig);

	g_chipdata.IanaInit = Iana = tb.GetIA()*1000.0;
	Log.printf("Iana=%6.2lf mA\n", Iana);

	if (Idig >100.0 || Iana >100.0) return ERROR_IMAX;
	if (Idig < 10.0 || Iana < 1.0) return ERROR_IMIN;

	g_chipdata.probecard.isValid = probecard;
	if (probecard)
	{
		// check return voltages
		g_chipdata.probecard.vd_cap = tb.GetVD_CAP();
		Log.section("VDCAP", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.vd_cap);

		g_chipdata.probecard.vd_reg = tb.GetVD_Reg();
		Log.section("VDREG", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.vd_reg);

		g_chipdata.probecard.v_dac = tb.GetVDAC_CAP();
		Log.section("VDAC", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.v_dac);

		g_chipdata.probecard.v_tout = tb.GetTOUT_COM();
		Log.section("VTOUT", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.v_tout);

		g_chipdata.probecard.v_aout = tb.GetAOUT_COM();
		Log.section("VAOUT", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.v_aout);
	}

	return 0;
}



// =======================================================================
//  token out check
// =======================================================================

#define ERROR_TOKEN_MISSING  3
#define ERROR_TOKEN_TIME     4

int test_tout()
{
	int cnt;

	Log.section("TOKEN", false);
	tb.roc_SetDAC( CtrlReg, 0x04);
	tb.Set(T_ResCal, 20);
	tb.Single(RES|TOK);
	tb.cDelay(100);
	tb.SendRoCnt();
	tb.Flush();
	g_chipdata.token = cnt = tb.RecvRoCnt();
	Log.printf(" %i\n", cnt);

	if (cnt == 255) return ERROR_TOKEN_MISSING;
	if (cnt != 0) return ERROR_TOKEN_TIME;  // no empty readout
	return 0;
}



// =======================================================================
// I2C address scan
// =======================================================================

#define ERROR_I2C  5
#define ERROR_I2C0 6 // address 0 works


int test_i2c()
{
	// --- init
	tb.Clear();
	Log.section("I2C not implemented");
	g_chipdata.i2c = -1;
	return 0;
}



// =======================================================================
//  read temperature sensor
// =======================================================================




int test_TempSensor()
{
	Log.section("TEMP not implemented");
	return 0;
}



// =======================================================================
//  Va supply current characteristic
// =======================================================================

#define VANASTEPS  18

double getIana(int dac, bool prot = false)
{
	double Iana;
	tb.Single(RES);
	tb.Flush();
	tb.uDelay(100);
	tb.roc_SetDAC(Vana, dac);
//	tb.Flush();
	tb.mDelay(200);
	Iana = tb.GetIA()*1000.0;
	if (prot)
	{
		Log.section("IANA",false);
		Log.printf("%3i %6.2lf mA\n", dac, Iana);
	}
	return Iana;
}


void test_current()
{
	int xmin = 30;
	tb.roc_SetDAC(VwllPr,  0);
	tb.roc_SetDAC(VwllSh,  0);
	tb.Flush();

	// Iana @ Vana
	double ia = 0.0;
	const int dac[VANASTEPS] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170 };

	Log.section("VANA");
	for (int i=0; i<VANASTEPS; i++)
	{
		ia = getIana(dac[i]);
		g_chipdata.Iana[i] = ia;
		Log.printf("%3i %6.2lf mA\n", dac[i], ia);
		if (ia<24.0) xmin = dac[i];
	}

	// set Iana to 24+/-2 mA
	if (xmin==0) return;
	int xmax = xmin+30, x=0;
	for (int n=0; n<6; n++)
	{
		x = (xmin+xmax)/2;
		ia = getIana(x);
		if (ia < 0.0) break;
		if (ia > 24.0) xmax = x; else xmin = x;
	}
	g_chipdata.InitVana = (ia>=0.0)? x : -1;
	g_chipdata.InitIana = ia;
	Log.section("ITRIM", false);
	Log.printf("%i %1.2lf mA\n", g_chipdata.InitVana, g_chipdata.InitIana);
	InitDAC();
}



// =======================================================================
//  dac scans
// =======================================================================

bool test_single_dac(unsigned char dac, unsigned char initValue, bool bit8)
{
	int max = bit8 ? 255 : 15;
	short y[256];
	if (tb.ScanDac(dac, 10, 0, max, y))
	{
		Log.printf("%3u  ", (unsigned int)dac);
		for (int i=0; i<=max; i++) Log.printf(" %5i", (int)y[i]);
		Log.puts("\n");
		tb.roc_SetDAC(dac, initValue);
		return true;
	}
	tb.roc_SetDAC(dac, initValue);
	return false;
}

void test_dac()
{
	int f = frequencyDivider;
	Log.section("DAC not implemented");
	return;

	test_single_dac( Vdig,        VDIG0,                  false );
	test_single_dac( Vsh,         VSH0,                   true  );
	test_single_dac( Vcomp,       StdSet[INIT_VCOMP ][f], false );
	test_single_dac( Vleak_comp,  StdSet[INIT_LEAK  ][f], true  );
	test_single_dac( VrgPr,       StdSet[INIT_RGPR  ][f], false );
	test_single_dac( VwllPr,      StdSet[INIT_WLLPR ][f], true  );
	test_single_dac( VrgSh,       StdSet[INIT_RGSH  ][f], false );
	test_single_dac( VwllSh,      StdSet[INIT_WLLSH ][f], true  );
	test_single_dac( VhldDel,     StdSet[INIT_HLDDEL][f], true  );
	test_single_dac( Vtrim,       StdSet[INIT_TRIM  ][f], true  );
	test_single_dac( VthrComp,    StdSet[INIT_THR   ][f], true  );
	test_single_dac( VIBias_Bus,  StdSet[INIT_BUS   ][f], true  );
	test_single_dac( Vbias_sf,    StdSet[INIT_SF    ][f], false );
	test_single_dac( VoffsetOp,   StdSet[INIT_OFFSOP][f], true  );
	test_single_dac( VIbiasOp,    StdSet[INIT_BIASOP][f], true  );
	test_single_dac( VoffsetRO,   StdSet[INIT_RO    ][f], true  );
	test_single_dac( VIon,        StdSet[INIT_ON    ][f], true  );
//	test_single_dac( VIbias_PH,   StdSet[INIT_PH    ][f], true  );
//	test_single_dac( Ibias_DAC,   StdSet[INIT_DAC   ][f], true  );
//	test_single_dac( VIbias_roc,  StdSet[INIT_ROC   ][f], true  );
	test_single_dac( VIColOr,     StdSet[INIT_COLOR ][f], true  );
	test_single_dac( Vnpix,       StdSet[INIT_NPIX  ][f], true  );
	test_single_dac( VsumCol,     StdSet[INIT_SUMCOL][f], true  );
	test_single_dac( Vcal,        StdSet[INIT_CAL   ][f], true  );
	test_single_dac( CalDel,      StdSet[INIT_CALDEL][f], true  );
}



// =======================================================================
//  pixel alive test
// =======================================================================


void test_pixel()
{
/*
	//	int col, row;
	CAnalog aout(&tb, settings.l1_bl_shift);

	// tb.Clear();

	// load settings
	tb.roc_SetDAC(Vcal, VCAL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04);

	tb.Set(T_ResCal, 15);
	tb.Set(T_CalTrg, 15 + tct_wbc);
	tb.roc_SetDAC(WBC,  15);
	tb.Set(T_TrgTok, 10);
	tb.Set(CALREP,    1);

	// send test sequence
	aout.ScanROC();

	tb.roc_SetDAC(CtrlReg,0);

	// get results
	aout.ReadRaw();
//	aout.SaveRaw("run.bin");

	aout.DetectLevel(Log);
	aout.LevelHisto(Log);
	aout.LevelCheck( g_chipdata.pixmap, Log);
	aout.LevelCheck2(g_chipdata.pixmap, Log);
	tb.mDelay(1000);
*/
}


// =======================================================================
//  pulse height
// =======================================================================

#define PULSE_VCAL1 150 // Low Range
#define PULSE_VCAL2  60 // High Range
#define PULSE_VTHR  100
#define PULSE_REP     5

void test_pulse_height()
{
	// tb.Clear();

	// load settings
	tb.roc_SetDAC(CtrlReg,0);
	tb.roc_SetDAC(Vcal, PULSE_VCAL1);
	tb.roc_SetDAC(CtrlReg,0x00);
	tb.roc_SetDAC(VthrComp, PULSE_VTHR);

	tb.Set(T_ResCal, 15);
	tb.Set(T_CalTrg, 15 + tct_wbc);
	tb.roc_SetDAC(WBC,  15);
	tb.Set(T_TrgTok, 10);
	tb.Set(CALREP,    1);

	int col, row;
	short data[ROC_NUMROWS];

	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.GetColPulseHeight(col,PULSE_REP, data);
		for (row=0; row<ROC_NUMROWS; row++)
			g_chipdata.pixmap.SetPulseHeight1(col,row, data[row]);
	}
	g_chipdata.pixmap.pulseHeight1Exist = true;


	tb.roc_SetDAC(CtrlReg,0x04);
	tb.roc_SetDAC(Vcal, PULSE_VCAL2);
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.GetColPulseHeight(col,PULSE_REP, data);
		for (row=0; row<ROC_NUMROWS; row++)
			g_chipdata.pixmap.SetPulseHeight2(col,row, data[row]);
	}
	g_chipdata.pixmap.pulseHeight2Exist = true;
}



// =======================================================================
//  cal delay scan
// =======================================================================

struct CPixelAddr
{
	int x, y;
};


int FindPixel(int dcol, int count, CPixelAddr pix[])
{
	if (dcol<0 || dcol>=ROC_NUMDCOLS) return 0;
	int x, y;
	int x2 = 2*dcol+1;
	int pos = 0;

	for (x=2*dcol; x<x2; x++) for (y=0; y<ROC_NUMROWS; y++)
	{
		if (!(pos < count)) return pos;
		if (!g_chipdata.pixmap.IsDefect(x,y)) { pix[pos].x = x; pix[pos].y = y; pos++; }
	}
	return pos;
}



int delay_scan()
{
	const int dcol = 13;
	CPixelAddr pix;
	Log.section("DELSCAN");
	if (FindPixel(dcol,1,&pix) != 1) return 0;
	Log.printf("pixel: %2i %2i\n     ", pix.x, pix.y);
	tb.roc_ClrCal();
	tb.roc_SetDAC(CtrlReg,0x00);
	tb.roc_SetDAC(Vcal, VCAL_LEVEL);
	tb.roc_Pix_Trim(pix.x, pix.y, 15);
	tb.roc_Pix_Cal (pix.x, pix.y, 0);
	tb.roc_Col_Enable(2*dcol, 1);
	tb.Set(T_ResCal, 15);
	tb.roc_SetDAC(WBC,  15);
	tb.Set(T_TrgTok, 10);
	tb.Set(CALREP,    1);

	int delta_wbc, cdel;
	for (cdel=0; cdel<=200; cdel+=5) Log.printf(" %3i", cdel);
	Log.puts("\n");

	for (delta_wbc=-1; delta_wbc<=1; delta_wbc++)
	{
		tb.Set(T_CalTrg, 15 + delta_wbc + tct_wbc);
		Log.printf("%3i  ", delta_wbc);
		for (cdel=0; cdel<=200; cdel+=5)
		{
			tb.roc_SetDAC(CalDel, cdel);
			tb.uDelay(50);
			unsigned int level = tb.test_PUC(pix.x,pix.y,15);
			Log.printf(" %3u", level);
		}
		Log.puts("\n");
	}
	tb.roc_ClrCal();
	tb.roc_Pix_Mask(pix.x, pix.y);
	tb.roc_Col_Enable(2*dcol, 0);
	tb.roc_SetDAC(CtrlReg,0x00);
	tb.roc_SetDAC(CalDel, StdSet[INIT_CALDEL][frequencyDivider]);
	tb.Flush();

	return 0;
}



// =======================================================================
// DCOL test
// =======================================================================

#define ERROR_WBC  8


int g_dcol[ROC_NUMDCOLS];


int test_WBC_SBC(int dcol)
{
	// find pixel for test
	CPixelAddr pix;
	if (FindPixel(dcol, 1, &pix) != 1)
	{	// error: no pixel found
		Log.printf("wbc(%2i)= no_pixel\n", dcol);
		g_dcol[dcol] |= (DCOL_NOPIX | DCOL_WBCERR);
		return ERROR_WBC;
	}

	// init chip
	tb.roc_SetDAC(Vcal, VCAL_DCOL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04);
	tb.roc_ClrCal();
	tb.roc_Pix_Trim(pix.x, pix.y, 15);
	tb.roc_Pix_Cal (pix.x, pix.y, 0);
	tb.roc_Col_Enable(2*dcol, 1);
	tb.Set(T_ResCal, 15);
	tb.Set(T_CalTrg, 15 + tct_wbc);
	tb.roc_SetDAC(WBC,  15);
	tb.Set(T_TrgTok, 10);
	tb.Set(CALREP,    1);

	// send test sequence
#define WBCSTEPS  8
	int wbc[WBCSTEPS] = { 0x08, 0x09, 0x0A, 0x0C, 0x10, 0x20, 0x40, 0x80 };

	int n;
	int td;
	for (n=0; n<WBCSTEPS; n++)
	{
		tb.roc_SetDAC(WBC, wbc[n]);
		tb.cDelay(10);
		for (td=6; td<=255; td++)
		{
			tb.Set(T_CalTrg, td + tct_wbc);
			tb.Single(RES|CAL|TRG|TOK);
			tb.cDelay(260);
			tb.SendRoCnt();
		}
	}
	tb.roc_ClrCal();
	tb.roc_Pix_Mask(pix.x, pix.y);
	tb.roc_Col_Enable(2*dcol, 0);
	tb.roc_SetDAC(CtrlReg,0x00);
	tb.Flush();

	// get results
	bool err = false;
	bool res[WBCSTEPS];
	for (n=0; n<WBCSTEPS; n++)
	{
		res[n] = false;
		for (td=6; td<=255; td++)
		{
			int cnt = tb.RecvRoCnt();
			if (wbc[n]==td && cnt==0 || wbc[n]!=td && cnt!=0)
			{
				err = true;
				res[n] = true;
			}
		}
	}

	if (err)
	{
		Log.printf("wbc(%2i)=", dcol);
		for (n=0; n<WBCSTEPS; n++)
			Log.printf("%i", res[n] ? 1 : 0);
		Log.puts("\n");
		g_dcol[dcol] |= DCOL_WBCERR;
		return ERROR_WBC;
	}

	return 0;
}


// -----------------------------------------------------------------------
// DCOL: Timstamp Buffer Test

#define ERROR_TS 9


int test_TB(int dcol)
{
	// find pixel
	CPixelAddr pix;
	if (FindPixel(dcol, 1, &pix) != 1)
	{	// error: no pixel found
		Log.printf("tb (%2i)= no_pixel\n", dcol);
		g_dcol[dcol] |= (DCOL_NOPIX | DCOL_TSERR);
		return ERROR_TS;
	}

	// init chip
	tb.roc_SetDAC(Vcal, VCAL_DCOL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04);
	tb.roc_ClrCal();
	tb.roc_Pix_Trim(pix.x, pix.y, 15);
	tb.roc_Pix_Cal (pix.x, pix.y, 0);
	tb.roc_Col_Enable(2*dcol, 1);
	tb.Set(T_ResCal, 15);
	tb.Set(T_CalCal,  6);
	tb.Set(T_CalTrg, 120 + tct_wbc);
	tb.roc_SetDAC(WBC,  120);
	tb.Set(T_TrgTok, 15);

	// send test sequence
#define TBSTEPS 15

	int n;
	for (n=1; n<TBSTEPS; n++)
	{
		tb.Set(CALREP, n);
		tb.Single(RES|CAL|TRG|TOK);
		tb.cDelay(300);
		tb.SendRoCnt();
	}
	tb.roc_ClrCal();
	tb.roc_Pix_Mask(pix.x, pix.y);
	tb.roc_Col_Enable(2*dcol, 0);
	tb.roc_SetDAC(CtrlReg,0x00);
	tb.Flush();

	// get results
	int res[TBSTEPS];

	bool err = false;
	for (n=1; n<TBSTEPS; n++)
	{
		res[n] = tb.RecvRoCnt();
		if (n<=12 && res[n]!= 1 || n>12 && res[n]!= 0) err = true;
	}

	if (err)
	{
		Log.printf("tb (%2i)=", dcol);
		for (n=1; n<TBSTEPS; n++) Log.printf("%4i", res[n]);
		Log.puts("\n");
		g_dcol[dcol] |= DCOL_TSERR;
		return ERROR_TS;
	}

	return 0;
}


// -----------------------------------------------------------------------
// DCOL: Data Buffer Test

#define ERROR_DB  10

#define NUM_DB_TEST_PIXEL 32


int test_DB(int dcol)
{
	// find pixel
	CPixelAddr pix[NUM_DB_TEST_PIXEL];
	if (FindPixel(dcol, NUM_DB_TEST_PIXEL, pix) != NUM_DB_TEST_PIXEL)
	{	// error: no pixel found
		Log.printf("db (%2i)= no_pixel\n", dcol);
		g_dcol[dcol] |= (DCOL_NOPIX | DCOL_DBERR);
		return ERROR_DB;
	}

	// init chip
	tb.roc_SetDAC(Vcal, VCAL_DCOL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04);
	tb.roc_ClrCal();
	tb.roc_Col_Enable(2*dcol, 1);
	tb.Set(T_ResCal, 15);
	tb.Set(T_CalCal,  6);
	tb.Set(T_CalTrg, 80 + tct_wbc);
	tb.roc_SetDAC(WBC,  80);
	tb.Set(T_TrgTok, 20);
	tb.Set(CALREP,    1);

	// send test sequence
	int n;
	for (n=0; n<NUM_DB_TEST_PIXEL; n++)
	{
		tb.roc_Pix_Trim(pix[n].x, pix[n].y, 15);
		tb.cDelay(50);
		tb.roc_Pix_Cal (pix[n].x, pix[n].y, 0);
		tb.cDelay(50);
		tb.Single(RES|CAL|TRG|TOK);
		tb.cDelay(200+6*n);
		tb.SendRoCnt();
	}
	tb.roc_ClrCal();
	for (n=0; n<NUM_DB_TEST_PIXEL; n++) tb.roc_Pix_Mask(pix[n].x, pix[n].y);
	tb.roc_Col_Enable(2*dcol, 0);
	tb.roc_SetDAC(CtrlReg,0x00);
	tb.Flush();

	// get results
	int res[NUM_DB_TEST_PIXEL];
	bool err = false;
	for (n=0; n<NUM_DB_TEST_PIXEL; n++)
	{
		res[n] = tb.RecvRoCnt();
		if (n<31 && res[n]!= n+1 || n>=31 && res[n]!= 0) err = true;
	}

	if (err)
	{
		Log.printf("db (%2i)=", dcol);
		for (n=0; n<NUM_DB_TEST_PIXEL; n++) Log.printf("%4i", res[n]);
		Log.puts("\n");
		g_dcol[dcol] |= DCOL_DBERR;
		return ERROR_DB;
	}

	return 0;
}


// -----------------------------------------------------------------------
// DCOLs

int test_DCOLs()
{
	int dcol;
	Log.section("DCOL");
	tb.Clear();
	if (settings.clock > 20) { SetMHz(20); InitDAC(); }
	for (dcol=0; dcol<ROC_NUMDCOLS; dcol++)
	{
		g_dcol[dcol] = 0;
		test_WBC_SBC(dcol);
		test_TB(dcol);
		test_DB(dcol);
		g_chipdata.dcol.set(dcol,g_dcol[dcol]);
	}
	if (settings.clock > 20) { SetMHz(); InitDAC(); }
	return 0;
}



// =======================================================================
//  test pixel level
// =======================================================================


void testAllPixel(int vtrim, unsigned int trimbit=100 /* reference */ )
{
	unsigned char res[ROC_NUMROWS];
	tb.roc_SetDAC(Vtrim, vtrim);
	tb.uDelay(100);

	unsigned int trimvalue = (trimbit<4) ? (~(0x01<<trimbit)&15) : 15;

	int col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.testColPixel(col,trimvalue,res);

		for(row=0; row<ROC_NUMROWS; row++)
		{
			if (trimbit>3) g_chipdata.pixmap.SetRefLevel(col,row,res[row]);
			else g_chipdata.pixmap.SetLevel(col,row,trimbit,res[row]);
		}
	}
}


int test_PUCs(bool forceDefTest = false)
{
	InitDAC();
	tb.Set(T_ResCal, 20);
	tb.Set(T_CalTrg, 20 + tct_wbc);
	tb.roc_SetDAC(WBC,  20);
	tb.Set(T_TrgTok, 20);
	tb.Set(CALREP,    1);
	tb.roc_SetDAC(Vcal, settings.sensor ? VCAL_LEVEL_SENSOR : VCAL_LEVEL);

	testAllPixel( 80);
	testAllPixel( 80, 3);
	testAllPixel(110, 2);
	testAllPixel(150, 1);
	testAllPixel(200, 0);
	g_chipdata.pixmap.UpdateTrimDefects();

	InitDAC();
	return 0;
}



// =======================================================================
//
//    shmoo plot
//
// =======================================================================

void Shmoo(int col, int row,
		int vx, int xmin, int xmax,
		int vy, int ymin, int ymax,
		bool scal)
{
	int count = xmax-xmin;
	if (count < 1 || count > 256) return;

	Log.section("SHMOO", false);
	Log.printf("pix=(%i/%i); regX(%i)=%i:%i; regY(%i)=%i:%i\n",
		col, row, vx, xmin, xmax, vy, ymin, ymax);
	tb.roc_Col_Enable(col, 1);
	tb.roc_Pix_Trim(col, row, 15);
	tb.roc_Pix_Cal(col, row, scal);

	for (int y=ymin; y<=ymax; y++)
	{
		tb.roc_SetDAC(vy,y);
		tb.uDelay(100);
		unsigned char s[260];
		tb.Scan1D(vx, xmin, xmax, 1, 5, 50, s);
		for (int i=0; i<count; i++) 
		{ if (s[i]) s[i] += '0'; else s[i] = '.'; }
		s[count] = 0;
		Log.printf("%3i%s\n", y, s);
		Log.flush();
	}

	tb.roc_ClrCal();
	tb.roc_Pix_Mask(col,row);
	tb.roc_Col_Enable(col, 0);
}




// =======================================================================
//
//    ROC test
//
// =======================================================================


void test_cleanup(int bin)
{
	tb.Single(0);
	tb.Poff();
	tb.Flush();
	g_chipdata.bin = bin;
	g_chipdata.Calculate();
	Log.section("CLASS", false);
	Log.printf(" %i\n", g_chipdata.chipClass);
	Log.section("POFF", false);
	Log.printf(" %i\n", bin);
}


void ask(char msg[])
{
	printf(" %s!", msg);
	getchar();
}


int test_roc(bool &repeat)
{
	repeat = false;
	g_chipdata.InitVana = VANA0;

	tb.SetVD(2.5);
	tb.SetID(0.4);
	tb.SetVA(1.5);
	tb.SetIA(0.4);

	tb.SetReg(41,0x20);
//	tb.SetReg(26,0);
	tb.SetReg(43,2);
	tb.SetTriggerMode(TRIGGER_ROC);

	SetMHz();
	Log.section("FREQ", false);
	Log.printf("%i\n", settings.clock);
	g_chipdata.frequency = settings.clock;

	int bin = 1;
	tb.roc_I2cAddr(0);
	tb.I2cAddr(0);

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

	if (test_tout())
	{
		bin = 3; // kein Token Out
		test_cleanup(bin);
		return bin;
	}

/*
	switch (test_i2c())
	{
	case ERROR_I2C:
		bin = 4;
		test_cleanup(bin);
		return bin;
	case ERROR_I2C0: bin = 4;
	}
*/
	test_current();
//	test_Iana();

//	test_TempSensor();

	if (settings.sensor)
	{
		tb.HVon();
		tb.mDelay(200);
	}

	test_dac();

//	g_chipdata.pixmap.Init(); is allready done

	test_pixel();
	unsigned int pixcnt = g_chipdata.pixmap.DefectPixelCount();

	delay_scan();

	if (pixcnt<=400) test_pulse_height();

	test_DCOLs();

	test_PUCs(pixcnt<500);

	if (settings.sensor)
	{
		tb.HVoff();
		tb.mDelay(200);
	}

	// --- Testresultat auswerten ------------------------------

	Log.section("PIXMAP");
	g_chipdata.pixmap.Print(Log);
	Log.section("PULSE");
	g_chipdata.pixmap.PrintPulseHeight(Log);
	g_chipdata.pixmap.PrintPulseHeight1(Log);
	g_chipdata.pixmap.PrintPulseHeight2(Log);
	Log.section("PUC1");
	g_chipdata.pixmap.PrintRefLevel(Log);
	Log.section("PUC2");
	g_chipdata.pixmap.PrintLevel(3, Log);
	Log.section("PUC3");
	g_chipdata.pixmap.PrintLevel(2, Log);
	Log.section("PUC4");
	g_chipdata.pixmap.PrintLevel(1, Log);
	Log.section("PUC5");
	g_chipdata.pixmap.PrintLevel(0, Log);

	if (bin==4)
	{
		test_cleanup(bin);
		return bin;
	}

	int x;
	// count defect dcols
	int dcolcnt = 0;
	for (x=0; x<ROC_NUMDCOLS; x++) if (g_dcol[x]) dcolcnt++;

	if      (dcolcnt>=5) bin = 5; // >= 5 dcols defect
	else if (dcolcnt>=2) bin = 6; // >= 2 dcols defect
	else if (dcolcnt==1) bin = 7; // 1 dcol defect

	if (dcolcnt>0)
	{
		test_cleanup(bin);
		return bin;
	}

	// count defect pixels
	pixcnt = g_chipdata.pixmap.DefectPixelCount();

	if      (pixcnt>=30) bin = 8;  // >= 30 pixel defect
	else if (pixcnt>=10) bin = 9;  // >= 10 pixel defect
	else if (pixcnt>= 3) bin = 10; // >=  3 pixel defect
	else if (pixcnt== 2) bin = 11; // ==  2 pixel defect
	else if (pixcnt== 1) bin = 12; // ==  1 pixel defect

	int addrErrors = 0;
	if (bin > 8)
	{
		int col, row;
		for (col=0; col<52; col++) for (row=0; row<80; row++)
			if (g_chipdata.pixmap.GetDefectAddrCode(col,row)) addrErrors++;
	}

	if (pixcnt>0)
	{
		test_cleanup(bin);
		if (bin > 8 && addrErrors > 0) repeat = true;
		return bin;
	}

	bin = 0; // no error

	test_cleanup(bin);
	return bin;
}



// =======================================================================
//
//    test on bump bonder (DLL)
//
// =======================================================================


void test_cleanup_bonder(int bin, int cClass = 0)
{
	tb.Single(0);
	tb.Poff();
	tb.Flush();
	g_chipdata.bin = bin;

	g_chipdata.Calculate();
	if (cClass > 0) g_chipdata.chipClass = cClass;

	Log.section("CLASS", false);
	Log.printf(" %i\n", g_chipdata.chipClass);

	Log.section("POFF", false);
	Log.printf(" %i\n", bin);
}


int test_roc_bumpbonder()
{
	int chipClass;
	g_chipdata.InitVana = VANA0;
	
	tb.SetVD(2.5);
	tb.SetID(0.4);
	tb.SetVA(1.5);
	tb.SetIA(0.4);

	tb.SetReg(41,0x20);
//	tb.SetReg(26,0);
	tb.SetReg(43,2);
	tb.SetTriggerMode(TRIGGER_ROC);

	SetMHz();
	Log.section("FREQ", false);
	Log.printf("%i\n", settings.clock);
	g_chipdata.frequency = settings.clock;

	int bin = 1;
	tb.roc_I2cAddr(0);
	tb.I2cAddr(0);

	switch (test_startup(false))
	{
		case ERROR_IMAX:
			bin = 1; // Ueberstrom
			test_cleanup_bonder(bin);
			return bin;
		case ERROR_IMIN:
			bin = 2; // kein Strom
			test_cleanup_bonder(bin);
			return bin;
	}

	if (test_tout())
	{
		bin = 3; // kein Token Out
		test_cleanup_bonder(bin);
		return bin;
	}

	switch (test_i2c())
	{
	case ERROR_I2C:
		bin = 4;
		test_cleanup_bonder(bin);
		return bin;
	case ERROR_I2C0: bin = 4;
	}

	test_current();

	test_TempSensor();

//	g_pixelmap.Init();

	test_pixel();

	Log.section("PIXMAP");
	g_chipdata.pixmap.Print(Log);

	if (bin==4)
	{
		test_cleanup_bonder(bin);
		return bin;
	}

	// --- class 1 2 3 chips ---------------------------------------------

	unsigned int nPixDefect     = 0;
	unsigned int nPixNoSignal   = 0;
	unsigned int nPixNoisy      = 0;
	unsigned int nPixUnmaskable = 0;
	unsigned int nPixAddrDefect = 0;
//	unsigned int nPixNoTrim     = 0;

	int col, row;
	for (col=0; col<52; col++) for (row=0; row<80; row++)
	{
		if (g_chipdata.pixmap.GetMaskedCount(col,row) > 0) nPixUnmaskable++;
		if (g_chipdata.pixmap.GetUnmaskedCount(col,row) == 0) nPixNoSignal++;
		else if (g_chipdata.pixmap.GetUnmaskedCount(col,row) > 1) nPixNoisy++;
		if (g_chipdata.pixmap.GetDefectAddrCode(col,row)) nPixAddrDefect++;
//		if (g_pixelmap.GetDefectTrimBit(col,row)) nPixNoTrim++;
		if (g_chipdata.pixmap.IsDefect(col,row)) { nPixDefect++; continue; }
	}

//	nColDefect = 0;
//	for (col=0; col<26; col++) if (chip.dcol.get(col)) nColDefect++;

	// === chip classification ===========================================

	// --- class 4 -------------------------------------------------------
	chipClass = 4;
	// no dcol test
//	if (nColDefect > 0) goto fail;

	if (nPixDefect >40) goto fail;

	if (nPixUnmaskable>0) goto fail;

	if (nPixAddrDefect>0) goto fail;

	if (                              70.0 < g_chipdata.IdigOn)   goto fail;
	if (                              70.0 < g_chipdata.IanaOn)   goto fail;
	if (g_chipdata.IdigInit < 15.0 || 50.0 < g_chipdata.IdigInit) goto fail;
	if (g_chipdata.IanaInit <  8.0 || 60.0 < g_chipdata.IanaInit) goto fail;

	// --- class 3 -------------------------------------------------------
	chipClass = 3;
	if (nPixDefect>4) goto fail; // 0.1% ... 1%

//	if (bl   <  -8.0 ||  8.0 < bl)    goto fail;

	// --- class 2 -------------------------------------------------------
	chipClass = 2;
	if (nPixDefect>0) { chipClass = 2; goto fail; }

	// --- class 1 -------------------------------------------------------
	chipClass = 1;

fail:
	test_cleanup_bonder(bin, chipClass);
	g_chipdata.chipClass = chipClass;

	return -chipClass;
}
