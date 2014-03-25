

#include "psi46test.h"
#include "chipdatabase.h"
#include "analyzer.h"

#include "profiler.h"
#include <iostream>
#include <iomanip>
#include <sstream>



#define VCAL_TEST          20  // 20 50 60 (high range) pixel alive test
                                //   schwelle @ 40MHz = 17
#define VCAL_DCOL_TEST     65  // 30 ... 120

#define VCAL_LEVEL         40	//  135 vcal for level test without sensor
#define VCAL_LEVEL_SENSOR  45	//  45 vcal for level test with sensor
#define VCAL_LEVEL_EXT    150	// 150 vcal for external calibrate
#define VANA0             100   // default vana value
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

void WriteSettings()
{
	Log.section("SETTINGS");
	Log.printf("f=40\n");
}


void InitDAC(bool reset)
{ PROFILING
	if (reset)
	{
		g_chipdata.InitVana = VANA0;
		g_chipdata.InitCalDel = 68;
	}

	tb.roc_SetDAC(  1,  VDIG0); // Vdig
	tb.roc_SetDAC(  2,  g_chipdata.InitVana);
	tb.roc_SetDAC(  3,  30);    // Vsf
	tb.roc_SetDAC(  4,  12);    // Vcomp

	tb.roc_SetDAC(  7, 150);    // VwllPr
	tb.roc_SetDAC(  9, 150);    // VwllSh
	tb.roc_SetDAC( 10, 117);    // VhldDel
	tb.roc_SetDAC( 11,  40);    // Vtrim
	tb.roc_SetDAC( 12,  80);    // VthrComp

	tb.roc_SetDAC( 13,  30);    // VIBias_Bus
//	tb.roc_SetDAC( 14,   6);    // Vbias_sf
	tb.roc_SetDAC( 22,  99);    // VIColOr

//	tb.roc_SetDAC( 15,  40);    // VoffsetOp
	tb.roc_SetDAC( 17, 170);    // VoffsetRO
//	tb.roc_SetDAC( 18, 115);    // VIon

	tb.roc_SetDAC( 19,  50);    // Vcomp_ADC
	tb.roc_SetDAC( 20,  90);    // VIref_ADC

	tb.roc_SetDAC( 25,   2);    // Vcal
	tb.roc_SetDAC( 26,  g_chipdata.InitCalDel);  // CalDel

	tb.roc_SetDAC( 0xfe, 14);   // WBC
	tb.roc_SetDAC( 0xfd,  4);   // CtrlReg

	tb.Flush();
}


void InitChip()
{ PROFILING
	InitDAC(true);
	tb.roc_ClrCal();
	tb.roc_Chip_Mask();
	tb.Flush();
}


void SetMHz(int MHz = 0)
{
	tb.Sig_SetDelay(SIG_CLK,  delayAdjust);
	tb.Sig_SetDelay(SIG_SDA,  delayAdjust+15);
	tb.Sig_SetDelay(SIG_CTR,  delayAdjust);
	tb.Sig_SetDelay(SIG_TIN,  delayAdjust+5);
	tb.Flush();

	tct_wbc = 5;
}



// =======================================================================
//  chip startup
// =======================================================================

#define ERROR_IMAX  1
#define ERROR_IMIN  2

// ROCDIGTEST: GetVD_CAP GetVDAC_CAP GetTOUT_COM GetAOUT_COM not implemented

int test_startup(bool probecard)
{ PROFILING
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

	if (Idig >120.0 || Iana > 120.0 ) return ERROR_IMAX;

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
		g_chipdata.probecard.vd_cap = 0; // tb.GetVD_CAP();
		Log.section("VDCAP", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.vd_cap);

		g_chipdata.probecard.vd_reg = 0; // tb.GetVD_CAP();
		Log.section("VDREG", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.vd_reg);

		g_chipdata.probecard.v_dac = 0; // tb.GetVDAC_CAP();
		Log.section("VDAC", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.v_dac);

		g_chipdata.probecard.v_tout = 0; // tb.GetTOUT_COM();
		Log.section("VTOUT", false);
		Log.printf("%5.3f\n", g_chipdata.probecard.v_tout);

		g_chipdata.probecard.v_aout = 0; // tb.GetAOUT_COM();
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
{ PROFILING

	Log.section("TOKEN", false);
	InitDAC();
	tb.roc_SetDAC( CtrlReg, 0x04);
	tb.Pg_SetCmd(0, PG_RESR + 20);
	tb.Pg_SetCmd(1, PG_TOK);
	
	tb.Daq_Open(1000);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.Daq_Start();
	tb.Pg_Single();
	tb.uDelay(4000);
	tb.Pg_Stop();
	unsigned int cnt = tb.Daq_GetSize();
	if (cnt > 255) cnt = 255;
	tb.Daq_Close();
	tb.Flush();

	g_chipdata.token = cnt;
	Log.printf(" %i\n", cnt);


	if (cnt == 255) return ERROR_TOKEN_MISSING;
	if (cnt != 1) return ERROR_TOKEN_TIME;  // no empty readout
	return 0;
}


// =======================================================================
// caldel scan
// =======================================================================

bool CalDelScan(int col, int row)
{ PROFILING
	const int max_caldel = 200;
	int x, k;

	vector<uint16_t> data;

	InitDAC(false);
	tb.roc_SetDAC(Vcal, VCAL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04); // 0x04

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);

	for (int i=0; i<ROC_NUMCOLS; i++) tb.roc_Col_Enable(i, true);
	tb.roc_Pix_Trim(col, row, 15);
	tb.roc_Pix_Cal(col, row);

	// --- take data
	tb.Daq_Open(50000);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.Daq_Start();
	for (x = 0; x<=max_caldel; x++)
	{
		tb.roc_SetDAC(CalDel, x);
		tb.uDelay(100);
		for (k=0; k<10; k++)
		{
			tb.Pg_Single();
			tb.uDelay(5);
		}
	}
	tb.Daq_Stop();
	tb.Daq_Read(data, 10000, 0);
	tb.Daq_Close();

	tb.roc_Pix_Mask(col, row);
	tb.roc_ClrCal();
	
	// --- analyze data
	int pos = 0, count;
	string s;
	s.reserve(max_caldel+1);
	PixelReadoutData pix;

	try
	{
		for (x = 0; x<=max_caldel; x++)
		{
			count = 0;
			for (k=0; k<10; k++)
			{
				DecodePixel(data, pos, pix);
				if (pix.n > 0) count++;
			}
			if (count == 0) s.push_back('.');
			else if (count >= 10) s.push_back('*');
			else s.push_back('0' + count);
		}
	} catch (int e) { printf("\nERROR %i @ %i\n", e, int(s.size())); return false; }

	unsigned int x1, x2, xdiff, xmean;
	x1 = s.find("********");
	if (x1 == string::npos) return false;
	x2 = x1+1;
	while (x2 < (s.size()-1) && s[x2] == '*') x2++;
	xdiff = x2 - x1;
	xmean = (x1 + x2)/2;
	
	Log.section("CALDELSCAN", false);
	Log.printf("%i %i %u\n%s\n", col, row, xmean, s.c_str());
	g_chipdata.InitCalDel = xmean;
	return true;
}


void CalDelScan()
{ PROFILING
	int i = 0;
	while (!CalDelScan(24+2*i,40+i) && i < 4) i++;
}


// =======================================================================
// I2C address scan
// =======================================================================

#define ERROR_I2C  5
#define ERROR_I2C0 6 // address 0 works

/*
0000 I2C data
0001 I2C address
0010 I2C pixel column
0011 I2C pixel row

1000 VD unreg
1001 VA unreg
1010 VA reg
1011 V bandgap
1100 IA

{ rocaddr[3:0], sana, s[2:0], data[7:0] }
*/

int GetReadback()
{ PROFILING
	int i;

	// --- take data
	tb.Daq_Start();
	for (i=0; i<32; i++)
	{
		tb.Pg_Single();
		tb.uDelay(10);
	}
	tb.Daq_Stop();

	// read out data
	vector<uint16_t> data;
	tb.Daq_Read(data, 1000);
//	DumpData(data, 40);
	//analyze data
	int pos = 0;
	PixelReadoutData pix;

	unsigned int value = 0;
	try
	{
		// find start bit
		do
		{
			DecodePixel(data, pos, pix);
		} while ((pix.hdr & 2) == 0);

		// read data
		for (i=0; i<16; i++)
		{
			DecodePixel(data, pos, pix);
			value = (value << 1) + (pix.hdr & 1);
		}

	} catch (int) { value = 0; }

	return value;
}


bool Check_Prog(int addr_i2c)
{ PROFILING
	tb.roc_I2cAddr(addr_i2c);
	tb.roc_SetDAC(Vcal, 0x0);
	if ((GetReadback() & 0xff) != 0x00) return false;
	tb.roc_SetDAC(Vcal, 0x5);
	if ((GetReadback() & 0xff) != 0x05) return false;
	return true;
}


int test_i2c()
{ PROFILING
	// --- init
	Log.section("I2C");
	tb.Pg_SetCmd(0, PG_TOK);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.roc_SetDAC(255, 0);

	int i, k;
	unsigned int mask;
	unsigned int res[16];
	int error = 0;
	for (i=0; i<16; i++) res[i] = 0;

	std::stringstream sslog;
	sslog << "   ";
	for (i=0; i<16; i++) sslog << std::hex << std::setw(2) << i;
	sslog << std::endl;
	tb.Daq_Open(1000);
	try
	{
		for (i=0; i<16; i++)
		{
			sslog << std::setw(2) << std::hex << i << ": ";
			tb.SetRocAddress(i);
			tb.uDelay(400);
			for (k=0, mask=1; k<16; k++, mask<<=1)
				if (Check_Prog(k)) { res[i] |= mask; sslog << "1 "; } else { sslog << ". "; }
			sslog << std::endl;
		}
	} catch (int e) { error = e; }
	tb.Daq_Close();

	tb.SetRocAddress(0);
	tb.roc_I2cAddr(0);
	tb.cDelay(10000);
	tb.Flush();
	Log.puts(sslog.str());

	// check results
	for (i=0, mask=1; i<16; i++, mask<<=1)
	{
		if (mask != res[i])
		{	// i2c error
			for (k=0; k<16; k++) Log.printf("%04X\n", res[k]);
			return (res[0]&0x0001) ? ERROR_I2C : ERROR_I2C0;
		}
	}
	g_chipdata.i2c = 1;
	return 0;
}


// =======================================================================
//  readback test
// =======================================================================


void test_readback()
{ PROFILING
	if ((settings.port_prober >= 0) && (g_chipdata.mapPos != 3)) return;

	tb.Pg_SetCmd(0, PG_TOK);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.Daq_Open(1000);

	tb.roc_SetDAC(0xff, 8);
	int vdig_u = GetReadback() & 0xff;
	vdig_u = GetReadback() & 0xff;

	tb.roc_SetDAC(0xff, 9);
	int vana_u = GetReadback() & 0xff;

	tb.roc_SetDAC(0xff, 10);
	int vana_r = GetReadback() & 0xff;

	tb.roc_SetDAC(0xff, 11);
	int vbg = GetReadback() & 0xff;

	tb.roc_SetDAC(0xff, 12);
	int iana = GetReadback() & 0xff;

	tb.Daq_Close();

	Log.section("READBACK");
	
	double vd = tb.GetVD();
	double va = tb.GetVA();
	double ia = tb.GetIA()*1000.0;
		
	if(vdig_u == 0) return;
	double cal = vd/vdig_u;

	double vdig_u_V = cal*vdig_u;
	Log.printf("Vdig_u  %3i  %5.2lf  %5.2lf \n", vdig_u,  vdig_u_V, vd);

    double vana_u_V = cal*vana_u;
	Log.printf("Vana_u  %3i  %5.2lf  %5.2lf\n",  vana_u,  vana_u_V, va);

	double vana_r_V = cal/2*vana_r;
	Log.printf("Vana_r  %3i  %5.2lf \n",         vana_r,  vana_r_V);

	double vbg_V = cal/2*vbg;
	Log.printf("Vbg     %3i  %5.2lf \n",         vbg,     vbg_V);

	double iana_mA = cal*15.0*iana;
	Log.printf("Iana    %3i  %5.1lf  %5.1lf\n",   iana,    iana_mA,  ia);

}



// =======================================================================
//  Va supply current characteristic
// =======================================================================

#define VANASTEPS  5

double getIana(int dac, bool prot = false)
{
	double Iana;
	tb.Pg_SetCmd(0, PG_RESR);
	tb.Pg_Single();
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
{ PROFILING
	int xmin = 30;
	tb.roc_SetDAC(VwllPr,  0);
	tb.roc_SetDAC(VwllSh,  0);
	tb.Flush();

	// Iana @ Vana
	double ia = 0.0;
	const int dac[VANASTEPS] = { 20, 60, 100, 140, 180 };

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
//  pixel alive test
// =======================================================================


void test_pixel()
{ PROFILING
	// load settings
	InitDAC();
	tb.roc_Chip_Mask();
	tb.roc_SetDAC(Vcal, VCAL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04); // 0x04

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(50000);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.Daq_Start();

	// --- scan all pixel ------------------------------------------------------
	unsigned char col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.roc_Col_Enable(col, true);
		tb.uDelay(10);
		for (row=0; row<ROC_NUMROWS; row++)
		{
//			tb.roc_Pix_Mask(xcol, row);
//			tb.uDelay(50);  // 10
			tb.roc_Pix_Cal (col, row, false);
			tb.uDelay(20);
			tb.Pg_Single();
			tb.uDelay(10);
			tb.roc_Pix_Trim(col, row, 15);
			tb.uDelay(5);
			tb.Pg_Single();
			tb.uDelay(10);

			tb.roc_Pix_Mask(col, row);
			tb.roc_ClrCal();
		}
		tb.roc_Col_Enable(col, false);
		tb.uDelay(10);
	}
	tb.Daq_Stop();

	vector<uint16_t> data;
	tb.Daq_Read(data, 50000);
	tb.Daq_Close();

	// --- analyze data --------------------------------------------------------
	// for each col, for each row, (masked pixel, unmasked pixel)
	PixelReadoutData pix;
	int pos = 0;
	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)
		{
			for (row=0; row<ROC_NUMROWS; row++)
			{
				// must be empty readout

				DecodePixel(data, pos, pix);
				g_chipdata.pixmap.SetMaskedCount(col, row, pix.n);

				// must be single pixel hit
				DecodePixel(data, pos, pix);
				g_chipdata.pixmap.SetUnmaskedCount(col, row, pix.n);
				if (pix.n > 0)
				{
					g_chipdata.pixmap.SetDefectColCode(col, row, pix.x != col);
					g_chipdata.pixmap.SetDefectRowCode(col, row, pix.y != row);
					g_chipdata.pixmap.SetPulseHeight(col, row, pix.p);
				}
			}
		}
	} catch (int) {}

	tb.roc_SetDAC(CtrlReg,0);
}


// =======================================================================
//  pulse height
// =======================================================================

#define PULSE_VCAL1  40 // High Range
#define PULSE_VCAL2  60 // High Range

void test_pulse_height1()
{ PROFILING
	InitDAC();

	// load individual settings
	tb.roc_SetDAC(CtrlReg, 0);
	tb.roc_SetDAC(Vcal, PULSE_VCAL1);
	tb.roc_SetDAC(CtrlReg, 0x04);

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);

	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(50000);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.Daq_Start();

	int col, row;

	// scan data
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.roc_Col_Enable(col, true);
		tb.uDelay(10);
		for (row=0; row<ROC_NUMROWS; row++)
		{
			tb.roc_Pix_Cal (col, row, false);
			tb.roc_Pix_Trim(col, row, 15);
			tb.uDelay(5);
			tb.Pg_Single();
			tb.uDelay(10);
			tb.roc_Pix_Mask(col, row);
			tb.roc_ClrCal();
		}
		tb.roc_Col_Enable(col, false);
		tb.uDelay(10);
	}

	tb.Daq_Stop();

	// read data
	vector<uint16_t> data;
	tb.Daq_Read(data, 50000);
	tb.Daq_Close();

	// analyze data
	PixelReadoutData pix;
	int pos = 0;
	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)	for (row=0; row<ROC_NUMROWS; row++)
		{
			DecodePixel(data, pos, pix);
			if (pix.n != 0)	g_chipdata.pixmap.SetPulseHeight1(col,row, pix.p);
		}
	} catch (int) { return; }
	g_chipdata.pixmap.pulseHeight1Exist = true;
}


void test_pulse_height2()
{ PROFILING
	InitDAC();

	// load individual settings
	tb.roc_SetDAC(CtrlReg, 0);
	tb.roc_SetDAC(Vcal, PULSE_VCAL2);
	tb.roc_SetDAC(CtrlReg, 0x04);

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);

	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(50000);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.Daq_Start();

	int col, row;

	// scan data
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.roc_Col_Enable(col, true);
		tb.uDelay(10);
		for (row=0; row<ROC_NUMROWS; row++)
		{
			tb.roc_Pix_Cal (col, row, false);
			tb.roc_Pix_Trim(col, row, 15);
			tb.uDelay(5);
			tb.Pg_Single();
			tb.uDelay(10);
			tb.roc_Pix_Mask(col, row);
			tb.roc_ClrCal();
		}
		tb.roc_Col_Enable(col, false);
		tb.uDelay(10);
	}

	tb.Daq_Stop();

	// read data
	vector<uint16_t> data;
	tb.Daq_Read(data, 50000);
	tb.Daq_Close();

	// analyze data
	PixelReadoutData pix;
	int pos = 0;
	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)	for (row=0; row<ROC_NUMROWS; row++)
		{
			DecodePixel(data, pos, pix);
			if (pix.n != 0)	g_chipdata.pixmap.SetPulseHeight2(col,row, pix.p);
		}
	} catch (int) { return; }
	g_chipdata.pixmap.pulseHeight2Exist = true;
}



// =======================================================================
//  pulse height scan
// =======================================================================


void test_pulseheight()
{ PROFILING
	int col = 10, row = 10;
	InitDAC();
	tb.roc_SetDAC(Vcal, VCAL_TEST);
	tb.roc_SetDAC(CtrlReg,0x04); // 0x04

	if (g_chipdata.pixmap.GetUnmaskedCount(col, row) == 0) col += 2;
	if (g_chipdata.pixmap.GetUnmaskedCount(col, row) == 0) col += 2;
	if (g_chipdata.pixmap.GetUnmaskedCount(col, row) == 0) col += 2;

	Log.section("PHSCAN");

	const int vcalmin = 0;
	const int vcalmax = 140;

	tb.Pg_Stop();
	tb.Pg_SetCmd(0, PG_RESR + 15);
	tb.Pg_SetCmd(1, PG_CAL  + 20);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);

	tb.Daq_Open(50000);
	tb.Daq_Select_Deser160(deserAdjust);
	tb.uDelay(100);
	tb.Daq_Start();
	tb.uDelay(100);

	// --- scan vcal
	tb.roc_Col_Enable(col, true);
	tb.roc_Pix_Trim(col, row, 15);
	tb.roc_Pix_Cal (col, row, false);

	vector<uint16_t> data;

	for (int cal = vcalmin; cal < vcalmax; cal++)
	{
		tb.roc_SetDAC(Vcal, cal);
		tb.uDelay(100);
		for (int k=0; k<5; k++)
		{
			tb.Pg_Single();
			tb.uDelay(20);
		}
	}

	tb.roc_Pix_Mask(col, row);
	tb.roc_Col_Enable(col, false);
	tb.roc_ClrCal();

	tb.Daq_Stop();
	tb.Daq_Read(data, 4000);
	tb.Daq_Close();
//	DumpData(data, 200);

	// --- plot data
	PixelReadoutData pix;

	int pos = 0;
	try
	{
		for (int cal = vcalmin; cal < vcalmax; cal++)
		{
			int cnt = 0;
			double yi = 0.0;
			for (int k=0; k<5; k++)
			{
				DecodePixel(data, pos, pix);
				if (pix.n > 0) { yi += pix.p; cnt++; }
			}
			if (cnt > 0)
				Log.printf("%3i %5.1f\n", cal, yi/cnt);
			else
				Log.printf("%3i\n", cal);
		}
	} catch (int) {}

	Log.flush();
}



// =======================================================================
//  test pixel threshold
// =======================================================================

bool GetPixel(unsigned int x)
{ PROFILING
	const unsigned int count = 9;
	unsigned int i;
	unsigned int n = 0;
	tb.roc_SetDAC(VthrComp, x);
	tb.uDelay(30);

	tb.Daq_Start();
	for (i=0; i<count; i++)
	{
		tb.Pg_Single();
		tb.uDelay(5);
	}
	tb.Daq_Stop();
	vector<uint16_t> data;
	tb.Daq_Read(data, 10000);
	int pos = 0;
	PixelReadoutData pix;

	for (i=0; i<count; i++)
	{
		DecodePixel(data, pos, pix);
		if (pix.n > 0) n++;
	}

	return n > count/2;
}


unsigned char FindLevel()
{ PROFILING
	static unsigned char x = 20;  // first estimation
	if (x>80) x = 80; else if (x<1) x=1;

	try
	{
		if (GetPixel(x))
		{
			do x--; while (GetPixel(x) && x>0);
			x++;
		}
		else
		{
			do x++; while (!GetPixel(x) && x<100);
			if (x>100) x=100;
		}
	} catch (int) { x = 20;  return 200; }

	return x;
}


unsigned char test_PUC(unsigned char col, unsigned char row, unsigned char trim)
{
	tb.roc_Pix_Trim(col, row, trim);
	tb.roc_Pix_Cal (col, row, 0);
	unsigned char res = FindLevel();
	tb.roc_ClrCal();
	tb.roc_Pix_Mask(col,row);
	return res;
}


void testColPixel(unsigned char col, unsigned char trimbit, unsigned char res[])
{ PROFILING
	unsigned char row;
	tb.roc_Col_Enable(col, 1);
	for(row=0; row<ROC_NUMROWS; row++)
	{
		res[row] = test_PUC(col,row,trimbit);
	}
	tb.roc_Col_Enable(col, 0);
}


void testAllPixel(int vtrim, unsigned int trimbit=4 /* reference */ )
{ PROFILING
	unsigned char res[ROC_NUMROWS];
	tb.roc_SetDAC(Vtrim, vtrim);
	tb.uDelay(100);

	unsigned int trimvalue = (trimbit<4) ? (~(0x01<<trimbit)&15) : 15;

	int col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		testColPixel(col,trimvalue,res);

		for(row=0; row<ROC_NUMROWS; row++)
		{
			if (trimbit>3) g_chipdata.pixmap.SetRefLevel(col,row,res[row]);
			else g_chipdata.pixmap.SetLevel(col,row,trimbit,res[row]);
		}
	}
}


int test_PUCs(bool forceDefTest = false)
{ PROFILING
	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(10000);
	tb.Daq_Select_Deser160(deserAdjust);

	InitDAC();
	tb.roc_SetDAC(Vcal, settings.sensor ? VCAL_LEVEL_SENSOR : VCAL_LEVEL);

	testAllPixel( 80);
	testAllPixel( 80, 3);
	testAllPixel(110, 2);
	testAllPixel(150, 1);
	testAllPixel(255, 0); // 200
	g_chipdata.pixmap.UpdateTrimDefects();

	InitDAC();

	tb.Daq_Close();

	return 0;
}



// =======================================================================
//  test pixel threshold
// =======================================================================

int PixelFired(vector<uint16_t> x, int &pos)
{
	// check header
	if (pos >= int(x.size())) return -1; // missing data
	if ((x[pos] & 0x8ffc) != 0x87f8) return -2; // wrong header
	pos++;

	if (pos >= int(x.size()) || (x[pos] & 0x8000)) return 0; // empty data readout

	// read additional noisy pixel
	while (!(pos >= int(x.size()) || (x[pos] & 0x8000))) { pos++; }

	return 1;
}


int GetPixelC(unsigned int x)
{
	const unsigned int count = 20;
	unsigned int i;
	unsigned int n = 0;
	tb.roc_SetDAC(Vcal, x);
	tb.uDelay(30);

	tb.Daq_Start();
	for (i=0; i<count; i++)
	{
		tb.Pg_Single();
		tb.uDelay(5);
	}
	tb.Daq_Stop();
	vector<uint16_t> data;
	tb.Daq_Read(data, 10000);
	int pos = 0;

	for (i=0; i<count; i++)
	{
		int res = PixelFired(data, pos);
		if (res > 0) n++;
		else if (res < 0) return res; 
	}

	return (n > count/2) ? 1 : 0;
}


int FindLevelC()
{ PROFILING
	static unsigned char x = 9;  // first estimation
	if (x>253) x = 200; else if (x<1) x=1;

	int res = GetPixelC(x);
	if (res < 0) return res;
	if (res > 0)
	{
		do
		{
			x--;
			res = GetPixelC(x);
			if (res < 0) return res;
		} while ((res > 0) && x>0);
		x++;
	}
	else
	{
		do
		{
			x++;
			res = GetPixelC(x);
			if (res < 0) return res;
		} while (!(res > 0) && x<253);
	}

	return x;
}


int test_PUCC(unsigned char col, unsigned char row, unsigned char trim)
{ PROFILING
	tb.roc_Pix_Trim(col, row, trim);
	tb.roc_Pix_Cal (col, row, 0);
	int res = FindLevelC();
	tb.roc_ClrCal();
	tb.roc_Pix_Mask(col,row);
	return res;
}


bool testColPixelC(uint8_t col, uint8_t trimbit, vectorR<uint8_t> &res)
{ PROFILING
	unsigned char row;
	tb.roc_Col_Enable(col, 1);
	res.clear();
	res.reserve(ROC_NUMROWS);
	for(row=0; row<ROC_NUMROWS; row++)
	{
		int r = test_PUCC(col,row,trimbit);
		if (r < 0) return false;
		res.push_back(r);
	}
	tb.roc_Col_Enable(col, 0);
	return true;
}


bool testAllPixelC(int vtrim, unsigned int trimbit=4 /* reference */ )
{ PROFILING
	vector<uint8_t> res;
	tb.roc_SetDAC(Vtrim, vtrim);
	tb.uDelay(100);

	unsigned int trimvalue = (trimbit<4) ? (~(0x01<<trimbit)&15) : 15;

	int col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		if (!tb.TestColPixel(col,trimvalue,res)) return false;

		for(row=0; row<ROC_NUMROWS; row++)
		{
			if (trimbit>3) g_chipdata.pixmap.SetRefLevel(col,row,res[row]);
			else g_chipdata.pixmap.SetLevel(col,row,trimbit,res[row]);
		}
	}
	return true;
}


int test_PUCsC(bool forceDefTest = false)
{ PROFILING
	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	tb.Daq_Open(10000);
	tb.Daq_Select_Deser160(deserAdjust);

	InitDAC();
	tb.roc_SetDAC(VthrComp, 40); // 20
	tb.roc_SetDAC(CtrlReg,0x00); // 0x04

	testAllPixelC(  0);
	testAllPixelC(100, 3);
	testAllPixelC(255, 2); // 110
//	testAllPixelC(255, 1); // 150
//	testAllPixelC(255, 0); // 200
	g_chipdata.pixmap.UpdateTrimDefects();

	InitDAC();

	tb.Daq_Close();

	return 0;
}


// =======================================================================
//
//    digital ROC test
//
// =======================================================================

void test_cleanup(int bin)
{
	tb.Init();
	tb.Flush();
	g_chipdata.bin = bin;
	g_chipdata.Calculate();
	Log.section("CLASS", false);
	Log.printf(" %i\n", g_chipdata.chipClass);
	Log.section("POFF", false);
	Log.printf(" %i\n", bin);
}


int test_roc_dig(bool &repeat)
{ PROFILING
	repeat = false;
	g_chipdata.InitVana = VANA0;

	tb.SetVD(2.5);
	tb.SetID(0.4);
	tb.SetVA(1.5);
	tb.SetIA(0.4);

	SetMHz();

	int bin = 1;
	tb.roc_I2cAddr(0);
	tb.SetRocAddress(0);

	tb.SignalProbeD1(PROBE_TIN);
	tb.SignalProbeA1(PROBEA_SDATA1);

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

 	switch (test_i2c())
	{
	case ERROR_I2C:
		bin = 4;
		test_cleanup(bin);
		return bin;
	case ERROR_I2C0: bin = 4;
	}
	tb.roc_I2cAddr(0);
	tb.SetRocAddress(0);

	test_readback();

	test_current();

	CalDelScan();

	test_pulseheight();
	test_pulseheight();

	test_pixel();
	unsigned int pixcnt = g_chipdata.pixmap.DefectPixelCount();

	if (pixcnt<=400)
	{
		test_pulse_height1();
		test_pulse_height2();
	}

//	test_DCOLs();

	test_PUCsC(pixcnt<500);

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
//	Log.section("PUC4");
//	g_chipdata.pixmap.PrintLevel(1, Log);
//	Log.section("PUC5");
//	g_chipdata.pixmap.PrintLevel(0, Log);

	if (bin==4)
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
//    digital ROC test on bump bonder (DLL)
//
// =======================================================================

void test_cleanup_bonder(int bin, int cClass = 0)
{
	tb.Init();
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
{ PROFILING

	int chipClass = 0;
	g_chipdata.InitVana = VANA0;

	tb.SetVD(2.5);
	tb.SetID(0.5);
	tb.SetVA(1.5);
	tb.SetIA(0.4);

	SetMHz();

	int bin = 0;
	tb.roc_I2cAddr(0);
	tb.SetRocAddress(0);

	switch (test_startup(true))
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

	tb.roc_I2cAddr(0);
	tb.SetRocAddress(0);

	test_readback();

	test_current();

	CalDelScan();

	test_pixel();
	unsigned int pixcnt = g_chipdata.pixmap.DefectPixelCount();

	Log.section("PIXMAP");
	g_chipdata.pixmap.Print(Log);

	if (bin == 4)
	{
		test_cleanup_bonder(bin);
		return bin;
	}


	// === chip classification ==============================================

	// --- class 4 ----------------------------------------------------------
	chipClass = 4;

	if (pixcnt > 40) goto fail; // > 1%

	if (                              70.0 < g_chipdata.IdigOn)   goto fail;
	if (                              10.0 < g_chipdata.IanaOn)   goto fail;
	if (g_chipdata.IdigInit < 10.0 || 50.0 < g_chipdata.IdigInit) goto fail;
	if (g_chipdata.IanaInit <  8.0 || 60.0 < g_chipdata.IanaInit) goto fail;

	// --- class 3 ----------------------------------------------------------
	chipClass = 3;

	if (pixcnt > 4) goto fail;  // > 0.1%

	// --- class 2 ----------------------------------------------------------
	chipClass = 2;

	if (pixcnt > 0) goto fail;

	// --- class 1 ----------------------------------------------------------
	chipClass = 1;

fail:
	test_cleanup_bonder(bin, chipClass);

	return -chipClass;
}