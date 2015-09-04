

#include "psi46test.h"
#include "chipdatabase.h"

#include "profiler.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "datastream.h"



#define VCAL_TEST          35  // 35 (high range) pixel alive test
                                //   schwelle @ 40MHz = 17
#define VCAL_DCOL_TEST     65  // 30 ... 120

#define VCAL_LEVEL        140	//  135 vcal for level test without sensor
#define VCAL_LEVEL_SENSOR  45	//  45 vcal for level test with sensor
#define VCAL_LEVEL_EXT    150	// 150 vcal for external calibrate
#define VANA0             145   // default vana value
#define VDIG0               4
#define VSH0              150   // 225



// =======================================================================
//  global variables for test results
// =======================================================================


namespace TestRocAna
{

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
	}

	tb.roc_SetDAC(  1,  VDIG0); // Vdig
	tb.roc_SetDAC(  2,  g_chipdata.InitVana);
	tb.roc_SetDAC(  3,  30);    // Vsf
	tb.roc_SetDAC(  4,  12);    // Vcomp
	tb.roc_SetDAC(  5,   0);    // Vleak_comp
	tb.roc_SetDAC(  6,   0);    // VrgPr
	tb.roc_SetDAC(  7,  35);    // VwllPr
	tb.roc_SetDAC(  8,   0);    // VrgSh
	tb.roc_SetDAC(  9,  35);    // VwllSh
	tb.roc_SetDAC( 10, 160);    // VhldDel
	tb.roc_SetDAC( 11,   7);    // Vtrim
	tb.roc_SetDAC( 12,  70);    // VthrComp

	tb.roc_SetDAC( 13,  30);    // VIBias_Bus
	tb.roc_SetDAC( 14,  10);    // Vbias_sf
	tb.roc_SetDAC( 15,  50);    // VoffsetOp
	tb.roc_SetDAC( 16, 115);    // VIbiasOp
	tb.roc_SetDAC( 17, 120);    // VoffsetRO
	tb.roc_SetDAC( 18, 115);    // VIon
	tb.roc_SetDAC( 19, 220);    // VIbias_PH
	tb.roc_SetDAC( 20, 153);    // VIbias_DAC
	tb.roc_SetDAC( 21, 220);    // VIbias_roc
	tb.roc_SetDAC( 22,  99);    // VIColOr
	tb.roc_SetDAC( 23,   0);    // Vnpix
	tb.roc_SetDAC( 24,   0);    // VsumCol

	tb.roc_SetDAC( 25,   2);    // Vcal
	tb.roc_SetDAC( 26,  40);    // 30 CalDel
	tb.roc_SetDAC( 27,   0);    // RangeTemp

	tb.roc_SetDAC( 0xfe, 20);   // WBC
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
{ PROFILING
	tb.Sig_SetDelay(SIG_CLK,  settings.adc_clkDelay );
	tb.Sig_SetDelay(SIG_SDA,  settings.adc_clkDelay+15);
	tb.Sig_SetDelay(SIG_CTR,  settings.adc_clkDelay);
	tb.Sig_SetDelay(SIG_TIN,  settings.adc_clkDelay+5);
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
        tb.GetVD_Cap(); tb.GetVD_Cap(); tb.GetVD_Cap(); tb.GetVD_Cap();
        g_chipdata.probecard.vd_cap =tb.GetVD_Cap();
        Log.section("VDCAP", false);
        Log.printf("%5.3f\n", g_chipdata.probecard.vd_cap);

		tb.GetVD_Reg(); tb.GetVD_Reg(); tb.GetVD_Reg(); tb.GetVD_Reg();
        g_chipdata.probecard.vd_reg = tb.GetVD_Reg();
        Log.section("VDREG", false);
        Log.printf("%5.3f\n", g_chipdata.probecard.vd_reg);

		tb.GetVDAC_Reg(); tb.GetVDAC_Reg(); tb.GetVDAC_Reg(); tb.GetVDAC_Reg();
        g_chipdata.probecard.v_dac = tb.GetVDAC_Reg();
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

	unsigned int cnt;

	CDtbSource src;
	CDataRecordScannerROC raw;
	CSink<CDataRecord*> data;
	src >> raw >> data;
	src.OpenRocAna(tb, 14, 10, 300, false, 5000);

	try
	{
		src.Enable();
		tb.Pg_Single();
		tb.uDelay(4000);
		src.Disable();
		cnt = data.Get()->GetSize();
		cnt = cnt >= 3 ? (cnt-3)/6 : 0;
		if (cnt > 255) cnt = 255;
	} catch (DataPipeException e) { cnt = 255; }

	src.Close();
	tb.Flush();

	g_chipdata.token = cnt;
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


void check_20_40(int addr_i2c)
{
	tb.roc_I2cAddr(addr_i2c);
	tb.roc_SetDAC(CtrlReg, 0x05);
	tb.cDelay(10);
	tb.Pg_Single();
	tb.cDelay(50);
	tb.roc_SetDAC(CtrlReg, 0x04);
	tb.cDelay(5);
}


int test_i2c()
{ PROFILING
	// --- init
	Log.section("I2C");
	tb.Pg_SetCmd(0, PG_RESR+20);
	tb.Pg_SetCmd(1, PG_TOK);

	int i, k;
	unsigned int mask;

	CDtbSource src;
	CDataRecordScannerROC raw;
	CSink<CDataRecord*> data;
	src >> raw >> data;
	src.OpenRocAna(tb, 14, 10, 300, false, 1000);
	src.Enable();

	// scan adresses
	for (i=0; i<16; i++)
	{
		tb.SetRocAddress(i);
		tb.cDelay(16000);
		for (k=0; k<16; k++) check_20_40(k);
	}
	tb.SetRocAddress(0);
	tb.roc_I2cAddr(0);
	tb.cDelay(10000);
	src.Disable();
	tb.Flush();

	unsigned int res[16];
	for (i=0; i<16; i++) res[i] = 0;

	std::stringstream sslog;
	sslog << "   ";
	for (i=0; i<16; i++) sslog << std::hex << std::setw(2) << i;
	sslog << std::endl;

	for (i=0; i<16; i++)
	{
		sslog << std::setw(2) << std::hex << i << ": ";
		try
		{
			for (k=0, mask=1; k<16; k++, mask<<=1)
			{
				CDataRecord *px = data.Get();
				if (px->GetSize() > 3)
				{
					res[i] |= mask;
					sslog << "1 ";
				}
				else sslog << ". ";
			}
		} catch (DataPipeException e) {}
		sslog << std::endl;
	}

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
//  Va supply current characteristic
// =======================================================================

#define VANASTEPS  5

double getIana(int dac, bool prot = false)
{ PROFILING
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
	const int dac[VANASTEPS] = { 64, 96, 128, 160, 192 };

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
// calibrate analog decoding
// =======================================================================

int ub_level;
int b_level;

void test_calibrate_decoding()
{ PROFILING
	// load settings
	Log.section("CALREADOUT", false);

	InitDAC();
	tb.roc_Chip_Mask();

	tb.Pg_SetCmd(0, PG_RESR + 25);
	tb.Pg_SetCmd(1, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	CDtbSource src;
	CDataRecordScannerROC raw;
//	CRocRawDataPrinter debug("debug.txt", true);
	CSink<CDataRecord*> data;
//	src >> raw >> debug >> data;
	src >> raw >> data;

	src.OpenRocAna(tb, 14, 10, 100, false, 1000);
	src.Enable();

	// --- scan all pixel ------------------------------------------------------
	int i;
	for (i=0; i<30; i++)
	{
		tb.Pg_Single();
		tb.uDelay(10);
	}
	src.Disable();

	int ub = 0, b = 0, n = 0;
	try
	{
		for (i=0; i<30; i++)
		{
			CDataRecord *ev = data.Get();
			if (ev->GetSize() >= 2)
			{
				ub += CAnalogLevelDecoder::ExpandSign((*ev)[0]);
				b  += CAnalogLevelDecoder::ExpandSign((*ev)[1]);
				n++;
			}
		}
	}
	catch (DataPipeException e)
	{
		printf("\nERROR test_calibrate_decoding: %s\n", e.what());
	}

	if (n>0)
	{
		ub_level = ub/n;
		b_level  = b/n;
	}
	else
	{
		ub_level = -350;
		b_level  = 0;
	}

	Log.printf(" %i %i\n", ub_level, b_level);
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
	tb.Pg_SetCmd(1, PG_CAL  + 20 + tct_wbc);
	tb.Pg_SetCmd(2, PG_TRG  + 16);
	tb.Pg_SetCmd(3, PG_TOK);
	tb.uDelay(100);
	tb.Flush();

	CDtbSource src;
	CDataRecordScannerROC raw;
//	CLevelHistogram hist;
	CRocAnaDecoder dec;
	dec.Calibrate(ub_level, b_level);

	CSink<CEvent*> data;
//	src >> raw >> hist >> dec >> data;
	src >> raw >> dec >> data;

	src.OpenRocAna(tb, 14, 10, 100, false, 100000);
	src.Enable();

	// --- scan all pixel ------------------------------------------------------
	unsigned char col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		tb.roc_Col_Enable(col, true);
		tb.uDelay(10);
		for (row=0; row<ROC_NUMROWS; row++)
		{
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
	src.Disable();

	// --- analyze data --------------------------------------------------------
	// for each col, for each row, (masked pixel, unmasked pixel)
	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)
		{
			for (row=0; row<ROC_NUMROWS; row++)
			{
				// must be empty readout

				CEvent *ev = data.Get();
				g_chipdata.pixmap.SetMaskedCount(col, row, ev->roc[0].pixel.size());

				// must be single pixel hit
				ev = data.Get();
				g_chipdata.pixmap.SetUnmaskedCount(col, row, ev->roc[0].pixel.size());
				if (ev->roc[0].pixel.size() > 0)
				{
					g_chipdata.pixmap.SetDefectColCode(col, row, ev->roc[0].pixel[0].x != col);
					g_chipdata.pixmap.SetDefectRowCode(col, row, ev->roc[0].pixel[0].y != row);
					g_chipdata.pixmap.SetPulseHeight(col, row, ev->roc[0].pixel[0].ph);
				}
			}
		}
	} catch (DataPipeException e) { printf("\nERROR TestPixel: %s\n", e.what()); }

//	hist.Report(Log);
	src.Close();
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

	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 100000);
	src.Enable();

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

	src.Disable();

	// analyze data
	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)	for (row=0; row<ROC_NUMROWS; row++)
		{
			CEvent *ev = data.Get();
			if (ev->roc[0].pixel.size() != 0)	g_chipdata.pixmap.SetPulseHeight1(col,row, ev->roc[0].pixel[0].ph);
		}
	} catch (DataPipeException e) { printf("\nERROR Test Pulse Height 1: %s\n", e.what()); return; }

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

	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 100000);
	src.Enable();

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

	src.Disable();

	// analyze data
	try
	{
		for (col=0; col<ROC_NUMCOLS; col++)	for (row=0; row<ROC_NUMROWS; row++)
		{
			CEvent *ev = data.Get();
			if (ev->roc[0].pixel.size() != 0)	g_chipdata.pixmap.SetPulseHeight2(col,row, ev->roc[0].pixel[0].ph);
		}
	} catch (DataPipeException e) { printf("\nERROR Test Pulse Height 2: %s\n", e.what()); return; }

	g_chipdata.pixmap.pulseHeight2Exist = true;
}



// =======================================================================
//  test pixel threshold
// =======================================================================

bool GetPixel(CDtbSource &src, CSink<CEvent*> &data, unsigned int x)
{ PROFILING
	const unsigned int count = 9;
	unsigned int i;
	unsigned int n = 0;
	tb.roc_SetDAC(VthrComp, x);
	tb.uDelay(30);

	src.Enable();
	for (i=0; i<count; i++)
	{
		tb.Pg_Single();
		tb.uDelay(5);
	}
	src.Disable();

	for (i=0; i<count; i++)
	{
		CEvent *ev = data.Get();
		if (ev->roc[0].pixel.size() > 0) n++;
	}

	return n > count/2;
}


unsigned char FindLevel(CDtbSource &src, CSink<CEvent*> &data)
{ PROFILING
	static unsigned char x = 20;  // first estimation
	if (x>80) x = 80; else if (x<1) x=1;

	try
	{
		if (GetPixel(src, data, x))
		{
			do x--; while (GetPixel(src, data, x) && x>0);
			x++;
		}
		else
		{
			do x++; while (!GetPixel(src, data, x) && x<100);
			if (x>100) x=100;
		}
	} catch (DataPipeException e) { x = 20; throw; }

	return x;
}


unsigned char test_PUC(CDtbSource &src, CSink<CEvent*> &data, unsigned char col, unsigned char row, unsigned char trim)
{ PROFILING
	tb.roc_Pix_Trim(col, row, trim);
	tb.roc_Pix_Cal (col, row, 0);
	unsigned char res = FindLevel(src, data);
	tb.roc_ClrCal();
	tb.roc_Pix_Mask(col,row);
	return res;
}


void testColPixel(CDtbSource &src, CSink<CEvent*> &data, unsigned char col, unsigned char trimbit, unsigned char res[])
{ PROFILING
	unsigned char row;
	tb.roc_Col_Enable(col, 1);
	for(row=0; row<ROC_NUMROWS; row++)
	{
		res[row] = test_PUC(src, data, col,row,trimbit);
	}
	tb.roc_Col_Enable(col, 0);
}


void testAllPixel(CDtbSource &src, CSink<CEvent*> &data, int vtrim, unsigned int trimbit=4 /* reference */ )
{ PROFILING
	unsigned char res[ROC_NUMROWS];
	tb.roc_SetDAC(Vtrim, vtrim);
	tb.uDelay(100);

	unsigned int trimvalue = (trimbit<4) ? (~(0x01<<trimbit)&15) : 15;

	int col, row;
	for (col=0; col<ROC_NUMCOLS; col++)
	{
		testColPixel(src, data, col,trimvalue,res);

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

	CDtbSource src;
	CDataRecordScannerROC raw;
	CRocDigDecoder dec;
	CSink<CEvent*> data;
	src >> raw >> dec >> data;
	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 10000);

	InitDAC();
	tb.roc_SetDAC(Vcal, settings.sensor ? VCAL_LEVEL_SENSOR : VCAL_LEVEL);

	try
	{
		testAllPixel(src, data,  80);
		testAllPixel(src, data,  80, 3);
		testAllPixel(src, data, 110, 2);
		testAllPixel(src, data, 150, 1);
		testAllPixel(src, data, 255, 0); // 200
		g_chipdata.pixmap.UpdateTrimDefects();
	} catch (DataPipeException e) { printf("\nERROR test_PUCs: %s\n", e.what()); }

	InitDAC();

	return 0;
}



// =======================================================================
//  test pixel threshold
// =======================================================================

int PixelFired(vector<uint16_t> x, int &pos)
{ PROFILING
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
{ PROFILING
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
		if (!tb.TestColPixel(col,trimvalue,false,res)) return false;

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

	CDtbSource src;

	src.OpenRocDig(tb, settings.deser160_tinDelay, false, 100000);
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

	return 0;
}


// =======================================================================
//
//    digital ROC test
//
// =======================================================================

void test_cleanup(int bin)
{ PROFILING
	tb.Init();
	tb.Flush();
	g_chipdata.bin = bin;
	g_chipdata.Calculate();
	Log.section("CLASS", false);
	Log.printf(" %i\n", g_chipdata.chipClass);
	Log.section("POFF", false);
	Log.printf(" %i\n", bin);
}


int test_roc(bool &repeat)
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

	test_current();

	test_calibrate_decoding();
	test_pixel();
	unsigned int pixcnt = g_chipdata.pixmap.DefectPixelCount();

/*	if (pixcnt<=400)
	{
		test_pulse_height1();
		test_pulse_height2();
	}
*/
//	test_DCOLs();

//	test_PUCsC(pixcnt<500);

	// --- Testresultat auswerten ------------------------------

	Log.section("PIXMAP");
	g_chipdata.pixmap.Print(Log);
/*
	Log.section("PULSE");
	g_chipdata.pixmap.PrintPulseHeight(Log);
	g_chipdata.pixmap.PrintPulseHeight1(Log);
	g_chipdata.pixmap.PrintPulseHeight2(Log);
	Log.section("PUC1");
	g_chipdata.pixmap.PrintRefLevel(Log);
	Log.section("PUC2");
	g_chipdata.pixmap.PrintLevel(3, Log);
	Log.section("PUC3");
	g_chipdata.pixmap.PrintLevel(2, Log); */
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
{ PROFILING
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

	test_current();

	test_calibrate_decoding();
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



} // namespace TestRocAna
