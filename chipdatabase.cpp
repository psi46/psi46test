// chipdatabase.cpp

#include <math.h>
#include <stdarg.h>
#include <time.h>

#include "ps.h"
#include "color.h"
#include "chipdatabase.h"



void CLogFile::Init()
{
	logTime[0] = 0;
	logVersion[0] = 0;
	productId[0] = 0;
	waferId[0] = 0;
	waferNr[0] = 0;
}


bool CLogFile::open(char logFileName[])
{
	Init();
	if (!Log.open(logFileName)) ERROR_ABORT(ERROR_OPEN)
	if (!readHeader()) return false;
	return true;
}


// logDate "Apr 17 13:53:45 2006"
// xmlDate "04-17-2006 13:53:45"

const char CChip::monthNames[12][4] =
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Okt", "Nov", "Dec" };


bool CChip::ConvertDate(char *xmlDate)
{
	char monstr[4];
	int year, month, day, h, m, s;

	if (sscanf(startTime, "%3s %d %d:%d:%d %d", monstr, &day, &h, &m, &s, &year) != 6)
		return false;

	month = 0;
	while (month<12 && strcmp(monthNames[month],monstr)!=0) month++;
	month++;

	if (month>12) return false;
	if (day<1 || 31<day) return false;
	if (year<1900 || 2500<year) return false;
	if (h<0 || 23<h) return false;
	if (m<0 || 59<m) return false;
	if (s<0 || 59<m) return false;

	sprintf(xmlDate, "%04i-%02i-%02i %02i:%02i:%02i",
		year, month, day, h, m, s);

	return true;
}


bool CLogFile::rewind()
{
	if (!Log.rewind()) ERROR_ABORT(ERROR_OPEN)
	if (!readHeader()) return false;
	return true;
}


bool CLogFile::readHeader()
{
	// read [OPEN] section
	if (!Log.isSection("OPEN")) ERROR_ABORT(ERROR_NO_LOGFILE)
	Log.getNextLine();
	if (strlen(Log.getLine())<12) ERROR_ABORT(ERROR_NO_LOGFILE)
	strncpy(logTime,Log.getNextLine()+5,24);

	// read [VERSION] section
	if (!Log.getNextSection("VERSION")) ERROR_ABORT(ERROR_NO_LOGFILE)
	Log.getNextLine();
	if (strlen(Log.getLine())<3) ERROR_ABORT(ERROR_NO_LOGFILE)
	strncpy(logVersion,Log.getNextLine(),43);
	Log.getNextSection();

	if (Log.isSection("CPU"))
	{
		Log.getNextSection();
	}

	if (Log.isSection("ATB"))
	{
		Log.getNextSection();
	}

	return true;
}


bool CLogFile::readChip(CChip &chip)
{
	// read [WAFER] section if exist
	if (Log.isSection("WAFER"))
	{
		if (sscanf(Log.getNextLine(),"%39s%39s%9s",
			productId,waferId,waferNr) != 3) ERROR_ABORT(ERROR_WAFERID)
		Log.getNextSection();
	}
	chip.Invalidate();
	strcpy(chip.productId, productId);
	strcpy(chip.waferId, waferId);
	strcpy(chip.waferNr, waferNr);
	return chip.Read(Log);
}


bool CLogFile::checkFileEnd()
{
	return Log.isSection("CLOSE");
//	if (!Log.isSection("CLOSE")) ERROR_ABORT(ERROR_CLOSE)
//	return true;
}



bool CAnalogLevel::Read(CScanner &Log)
{
	if (sscanf(Log.getNextLine(), "%i %lf %lf %i %i",
		&n, &mean, &stdev, &min, &max) != 5) return false;
	exist = true;

	return true;
}


void CAnalogLevel::Save(const char name[], FILE *f)
{
	fprintf(f,"%s: exist=%s", name, exist?"true ":"false\n");
	if (exist) fprintf(f,"n=%i mean=%0.1f stdev=%0.1f min=%i max=%i\n",
		n, mean, stdev, min, max);
}


void CChip::Invalidate()
{
	int i;

	multi = 0;
	nEntry = 0;
	productId[0]=waferId[0]=waferNr[0]=chipId[0]=0;
	mapX = mapY = mapPos = 0;
	startTime[0] = endTime[0] = 0;
	frequency = -1;
	IdigOn = IanaOn = IdigInit = IanaInit = -1.0;
	probecard.Init();
	for (i=0; i<5; i++) Iana[i] = -1.0;
	InitVana = -1;  InitIana = -1.0;
	ublack.Init();
	black.Init();
	for (i=0; i<6; i++) l[i].Init();
	token = -1;
	i2c   = -1;
	bin   = -1;
	pixtest2 = 0;
	logChipClass = -1;
	chipClass = 5;
	pickClass = 5;
	pickGroup = 99;
	nPh = 0;
	nPhFail = 0;
	pixmap.Init();
	dcol.Init();
}


void CChip::getCurrent(CScanner &Log, double *Idig, double *Iana)
{
	if (Log.getLine()[0] == 0) return;
	char s[8];
	double i;
	if (sscanf(Log.getLine(),"%5s %lf", s, &i)==2)
	{
		if (strcmp("Iana=",s)==0) *Iana = i;
		else *Idig = i;
	}
	Log.getNextLine();
}


void CChip::Save(FILE *f)
{
	fprintf(f,"nEntry:    %i\n", nEntry);
	fprintf(f,"productId: %s\n", productId);
	fprintf(f,"waferId:   %s\n", waferId);
	fprintf(f,"waferNr:   %s\n", waferNr);
	fprintf(f,"mapX:      %i\n", mapX);
	fprintf(f,"mapY:      %i\n", mapY);
	fprintf(f,"mapPos:    %i\n", mapPos);
	fprintf(f,"chipId:    %s\n", chipId);
	fprintf(f,"startTime: %s",   startTime);
	fprintf(f,"endTime:   %s",   endTime);
	fprintf(f,"frequency: %i\n", frequency);
	fprintf(f,"IdigOn:    %0.1f mA\n", IdigOn);
	fprintf(f,"IanaOn:    %0.1f mA\n", IanaOn);
	fprintf(f,"IdigInit:  %0.1f mA\n", IdigInit);
	fprintf(f,"IanaInit:  %0.1f mA\n", IanaInit);
	fprintf(f,"probecard.isValid: %s\n", probecard.isValid?"true":"false");
	if (probecard.isValid)
	{
		fprintf(f,"probecard.v_aout: %0.3f V\n", probecard.v_aout);
		fprintf(f,"probecard.v_dac:  %0.3f V\n", probecard.v_dac);
		fprintf(f,"probecard.v_tout: %0.3f V\n", probecard.v_tout);
		fprintf(f,"probecard.vd_cap: %0.3f V\n", probecard.vd_cap);
		fprintf(f,"probecard.vd_reg: %0.3f V\n", probecard.vd_reg);
	}
	fprintf(f,"Iana:      %0.1f  %0.1f  %0.1f  %0.1f  %0.1f\n",
		Iana[0],Iana[1],Iana[2],Iana[3],Iana[4]);
	fprintf(f,"InitVana:  %i\n",       InitVana);
	fprintf(f,"InitIana:  %0.1f mA\n", InitIana);

	ublack.Save("ublack", f);
	black.Save("black", f);
	l[0].Save("l[0]", f);
	l[1].Save("l[1]", f);
	l[2].Save("l[2]", f);
	l[3].Save("l[3]", f);
	l[4].Save("l[4]", f);
	l[5].Save("l[5]", f);

	fprintf(f,"token:     %i\n", token);
	fprintf(f,"i2c:       %i\n", i2c);
	fprintf(f,"bin:       %i\n", bin);
//	pixmap.Save(f);
//	dcol.Save(f);
}


bool CChip::Read(CScanner &Log)
{
	// read [CHIP] section
	if (Log.isSection("CHIP")) // chip on wafer
	{
		char ch;
		if (sscanf(Log.getNextLine(),"%i%i %c",&mapX, &mapY, &ch)!=3)
			ERROR_ABORT(ERROR_CHIP);

		if ('A'<=ch && ch<='D')	mapPos = ch - 'A';
		else ERROR_ABORT(ERROR_CHIP);
		sprintf(chipId, "%i%i%c", mapY, mapX, ch);
	}
	else if (Log.isSection("CHIP1")) // single chip
	{
		if (sscanf(Log.getNextLine(),"%40s",chipId) != 1)
			ERROR_ABORT(ERROR_CHIP);
		mapX = mapY = mapPos = 0;
	}
	else ERROR_ABORT(ERROR_OK)
	Log.getNextSection();

	// read [BEGIN] section
	if (!Log.isSection("BEGIN")) ERROR_ABORT(ERROR_BEGIN)
	Log.getNextLine();
	if (strlen(Log.getLine())<12) ERROR_ABORT(ERROR_BEGIN)
	strncpy(startTime,Log.getNextLine()+5,24);
	Log.getNextSection();

	// read FREQ section if exist
	if (Log.isSection("FREQ"))
	{
		if (sscanf(Log.getNextLine(), "%i", &frequency)!=1)
			ERROR_ABORT(ERROR_FREQ)
		Log.getNextSection();
	}

	// read [PON] section
	if (!Log.isSection("PON")) ERROR_ABORT(ERROR_PON)
	if (sscanf(Log.getNextLine(),"%i", &nEntry)!=1)
		ERROR_ABORT(ERROR_PON)
	Log.getNextLine();
	getCurrent(Log,&IdigOn,&IanaOn);
	getCurrent(Log,&IdigOn,&IanaOn);
	Log.getNextSection();

	// read [INIT] section if exist
	if (Log.isSection("INIT"))
	{
		Log.getNextLine();
		Log.getNextLine();
		getCurrent(Log,&IdigInit,&IanaInit);
		getCurrent(Log,&IdigInit,&IanaInit);
		Log.getNextSection();
	}

	// read [VDCAP] if exist
	if (Log.isSection("VDCAP"))
	{
		if (sscanf(Log.getNextLine(), "%lf", &probecard.vd_cap)!=1)
			ERROR_ABORT(ERROR_PROBECARD)
		if (!Log.getNextSection("VDREG","TOKEN"))
			ERROR_ABORT(ERROR_PROBECARD);
		if (sscanf(Log.getNextLine(), "%lf", &probecard.vd_reg)!=1)
			ERROR_ABORT(ERROR_PROBECARD);
		if (!Log.getNextSection("VDAC","TOKEN"))
			ERROR_ABORT(ERROR_PROBECARD);
		if (sscanf(Log.getNextLine(), "%lf", &probecard.v_dac)!=1)
			ERROR_ABORT(ERROR_PROBECARD);
		if (!Log.getNextSection("VTOUT","TOKEN"))
			ERROR_ABORT(ERROR_PROBECARD);
		if (sscanf(Log.getNextLine(), "%lf", &probecard.v_tout)!=1)
			ERROR_ABORT(ERROR_PROBECARD);
		if (!Log.getNextSection("VAOUT","TOKEN"))
			ERROR_ABORT(ERROR_PROBECARD);
		if (sscanf(Log.getNextLine(), "%lf", &probecard.v_aout)!=1)
			ERROR_ABORT(ERROR_PROBECARD);
		probecard.isValid = true;
		Log.getNextSection();
	}

	// read [TOKEN] section if exist
	if (Log.isSection("TOKEN"))
	{
		if (sscanf(Log.getNextLine(),"%i", &token)!=1)
			ERROR_ABORT(ERROR_TOKEN)
		Log.getNextSection();
	}

	// read [I2C] section if exist
	if (Log.isSection("I2C"))
	{
		Log.getNextLine();
		i2c = (strlen(Log.getNextLine()) == 0) ? 1 : 0;
		Log.getNextSection();
	}

	// read [VANA] section if exist
	if (Log.isSection("VANA"))
	{
		int dac;
		Log.getNextLine();
		if (sscanf(Log.getNextLine(),"%i %lf",&dac,&Iana[0])!=2)
			ERROR_ABORT(ERROR_VANA)
		if (dac!=64) ERROR_ABORT(ERROR_VANA)
		if (sscanf(Log.getNextLine(),"%i %lf",&dac,&Iana[1])!=2)
			ERROR_ABORT(ERROR_VANA)
		if (dac!=96) ERROR_ABORT(ERROR_VANA)
		if (sscanf(Log.getNextLine(),"%i %lf",&dac,&Iana[2])!=2)
			ERROR_ABORT(ERROR_VANA)
		if (dac!=128) ERROR_ABORT(ERROR_VANA)
		if (sscanf(Log.getNextLine(),"%i %lf",&dac,&Iana[3])!=2)
			ERROR_ABORT(ERROR_VANA)
		if (dac!=160) ERROR_ABORT(ERROR_VANA)
		if (sscanf(Log.getNextLine(),"%i %lf",&dac,&Iana[4])!=2)
			ERROR_ABORT(ERROR_VANA)
		if (dac!=192) ERROR_ABORT(ERROR_VANA)
		Log.getNextSection();
	}

	// skip [IANA] sections
	while (Log.isSection("IANA")) Log.getNextSection();

	// read [ITRIM] section if exist
	if (Log.isSection("ITRIM"))
	{
		if (sscanf(Log.getNextLine(),"%i %lf", &InitVana, &InitIana)!=2)
			ERROR_ABORT(ERROR_ITRIM)
		Log.getNextSection();
	}

	if (Log.isSection("TEMP")) Log.getNextSection();

	if (Log.isSection("DAC")) Log.getNextSection();

	if (Log.isSection("LVLDET")) Log.getNextSection();

	if (Log.isSection("AOUTHIST")) Log.getNextSection();

	if (Log.isSection("PIXEL")) Log.getNextSection();

	// read [AOUT] section
	if (Log.isSection("AOUT"))
	{
		Log.getNextLine();
		if (!black.Read(Log))  ERROR_ABORT(ERROR_AOUT)
		if (!ublack.Read(Log)) ERROR_ABORT(ERROR_AOUT)
		for (int i=0; i<6; i++)	if (!l[i].Read(Log)) ERROR_ABORT(ERROR_AOUT)
		Log.getNextSection();
	}

	while (Log.isSection("PIXTEST2"))
	{
		char *s = Log.getNextLine();
		char *pos = strchr(s,')');
		if (pos)
		{
			if      (pos[1] == 'd') pixtest2 += 256;
			else if (pos[1] == 'w') pixtest2 += 1;
		}
		Log.getNextSection();
	}

	if (Log.isSection("DELSCAN")) Log.getNextSection();

	// read [DCOL] section if exist
	if (Log.isSection("DCOL"))
	{
		Log.getNextLine();
		if (!dcol.read(Log)) ERROR_ABORT(ERROR_DCOL)
		Log.getNextSection();
	}

	// read [PIXMAP] section if exist
	if (Log.isSection("PIXMAP"))
	{
		Log.getNextLine();
		if (!pixmap.ReadPixMap(Log)) ERROR_ABORT(ERROR_PIXEL)
		Log.getNextSection();
	}

	if (Log.isSection("PULSE"))
	{
		Log.getNextLine();
		if (!pixmap.ReadPulseHeight(Log)) ERROR_ABORT(ERROR_PH)
		Log.getNextSection();
	}

	if (Log.isSection("PH1"))
	{
		Log.getNextLine();
		if (!pixmap.ReadPulseHeight1(Log)) ERROR_ABORT(ERROR_PH1)
		Log.getNextSection();
	}

	if (Log.isSection("PH2"))
	{
		Log.getNextLine();
		if (!pixmap.ReadPulseHeight2(Log)) ERROR_ABORT(ERROR_PH2)
		Log.getNextSection();
	}

	// read [PUCn] sections if exist
	if (Log.isSection("PUC1"))
	{
		// read [PUC1] section
		Log.getNextLine();
		if (!pixmap.ReadRefLevel(Log)) ERROR_ABORT(ERROR_PUC)
		Log.getNextSection();

		// read [PUC2] section
		if (!Log.isSection("PUC2")) ERROR_ABORT(ERROR_PUC)
		Log.getNextLine();
		if (!pixmap.ReadLevel(Log,3)) ERROR_ABORT(ERROR_PUC)
		Log.getNextSection();

		// read [PUC3] section
		if (!Log.isSection("PUC3")) ERROR_ABORT(ERROR_PUC)
		Log.getNextLine();
		if (!pixmap.ReadLevel(Log,2)) ERROR_ABORT(ERROR_PIXEL)
		Log.getNextSection();

		// read [PUC4] section
		if (!Log.isSection("PUC4")) ERROR_ABORT(ERROR_PUC)
		Log.getNextLine();
		if (!pixmap.ReadLevel(Log,1)) ERROR_ABORT(ERROR_PUC)
		Log.getNextSection();

		// read [PUC5] section
		if (!Log.isSection("PUC5")) ERROR_ABORT(ERROR_PUC)
		Log.getNextLine();
		if (!pixmap.ReadLevel(Log,0)) ERROR_ABORT(ERROR_PUC)
		Log.getNextSection();
		pixmap.levelExist = true;
	}

	// read [CLASS] section
	if (Log.isSection("CLASS"))
	{
		if (sscanf(Log.getNextLine(),"%i", &logChipClass)!=1) ERROR_ABORT(ERROR_CLASS)
		Log.getNextSection();
	}

	// read [POFF] section
	if (!Log.isSection("POFF")) ERROR_ABORT(ERROR_POFF)
	if (sscanf(Log.getNextLine(),"%i", &bin)!=1) ERROR_ABORT(ERROR_POFF)
	Log.getNextSection();

	// read [END] section
	if (!Log.isSection("END")) ERROR_ABORT(ERROR_END)
	Log.getNextLine();
	if (strlen(Log.getLine())<12) ERROR_ABORT(ERROR_END)
	strncpy(endTime,Log.getNextLine()+5,24);
	Log.getNextSection();

	return true;
}


bool operator>(CChip &a, CChip &b)
{
	if (a.picY > b.picY) return true;
	if ((a.picY==b.picY) && (a.picX>b.picX)) return true;
	return false;
}

inline bool operator==(CChip &a, CChip &b)
{
	return (a.picY == b.picY) && (a.picX == b.picX);
}


void CChip::SetAoutOffset(double offset)
{
	int iOffset = int(offset);
	if (black.exist)
	{
		black.mean -= offset;
		black.min -= iOffset;
		black.max -= iOffset;
	}
	if (ublack.exist)
	{
		ublack.mean -= offset;
		ublack.min -= iOffset;
		ublack.max -= iOffset;
	}

	for (int i=0; i<6; i++) if (l[i].exist)
	{
		l[i].mean -= offset;
		l[i].min -= iOffset;
		l[i].max -= iOffset;
	}
}


void CChip::Calculate()
{
	// set pic coordinates
	if (mapX!=0 || mapY!=0)
	{
		picX = 2*mapX + mapPos%2 + 1;
		picY = 2*mapY - mapPos/2 + 2;
	}
	else { picX = picY = 0; }


	double blackLevel = black.mean;
	addressStep = (black.exist && ublack.exist) ? (black.mean-ublack.mean)/4 : -1.0;

	n    = 0;
	pm   = 0.0;
	pm_col_max = 0.0;
	pstd = 0.0;
	pmin = 100;
	pmax = 0;

	if (bin==13) bin = 0;
	nPixDefect     = 0;
	nPixNoSignal   = 0;
	nPixNoisy      = 0;
	nPixUnmaskable = 0;
	nPixAddrDefect = 0;
	nPixNoTrim     = 0;
	nPixThrOr      = 0;

	int col, row, sum = 0, sum2 = 0;
	for (col=0; col<52; col++) for (row=0; row<80; row++)
	{
		if (pixmap.GetMaskedCount(col,row) > 0) nPixUnmaskable++;
		if (pixmap.GetUnmaskedCount(col,row) == 0) nPixNoSignal++;
		else if (pixmap.GetUnmaskedCount(col,row) > 1) nPixNoisy++;
		if (pixmap.GetDefectAddrCode(col,row)) nPixAddrDefect++;
		if (pixmap.GetDefectTrimBit(col,row)) nPixNoTrim++;
		if (pixmap.IsDefect(col,row)) { nPixDefect++; continue; }

		int y = pixmap.GetRefLevel(col,row);

		if (y < 100)
		{
			n++;
			sum  += y;
			sum2 += y*y;
			if (y<pmin) pmin = y;
			if (y>pmax) pmax = y;
		}
	}

	if (n>0)
	{
		pm    = double(sum)/n;
		pstd  = sqrt(double(sum2)/n - pm*pm);

		int n_col;
		double pm_col[52];
		for (col=0; col<52; col++)
		{
			n_col = 0;
			pm_col[col] = 0.0;
			int pcolsum = 0;
			for (row=0; row<80; row++)
				if (!pixmap.IsDefect(col,row))
				{
					n_col++;
					pcolsum += pixmap.GetRefLevel(col,row);
				}
			if (n_col>0) pm_col[col] = (double)pcolsum/n_col; else break;
		}
		if (n_col>0)
			for (col=1; col<52; col++)
			{
				double pm_col_diff = fabs(pm_col[col] - pm_col[col-1]);
				if ((col==1) || (col==51)) pm_col_diff /= 3.0;
				if (pm_col_diff > pm_col_max) pm_col_max = pm_col_diff;
			}

#define PMAX 15
		for (col=0; col<52; col++) for (row=0; row<80; row++)
		{
			int y = pixmap.GetRefLevel(col,row);
			if (y == 100) nPixThrOr++;
			else if (y < (pm-PMAX) || (pm+PMAX) < y) nPixThrOr++;
		}

	}

	nColDefect = 0;
	for (col=0; col<26; col++) if (dcol.get(col)) nColDefect++;

	// pulse height
#define PH1TOL   90
#define PH21TOL  60
	nPh = 0;
	nPhFail = 0;
	int sum1=0, sum1_2=0, sum21 = 0, sum21_2=0;
	if (pixmap.pulseHeight1Exist && pixmap.pulseHeight2Exist)
	{
		for (col=0; col<52; col++) for (row=0; row<80; row++)
		{
			int ph1 = pixmap.GetPulseHeight1(col,row);
			int ph2 = pixmap.GetPulseHeight2(col,row);
			if (ph1<10000 && ph2<10000)
			{
				nPh++;
				sum1    += ph1;
				sum1_2  += ph1*ph1;
				sum21   += ph2-ph1;
				sum21_2 += (ph2-ph1)*(ph2-ph1);
			}
		}
		if (nPh>0)
		ph1mean  = double(sum1)/nPh;
		ph21mean = double(sum21)/nPh;
		ph1std  = sqrt(double(sum1_2)/nPh  - ph1mean*ph1mean);
		ph21std = sqrt(double(sum21_2)/nPh - ph21mean*ph21mean);

		for (col=0; col<52; col++) for (row=0; row<80; row++)
		{
			int ph1 = pixmap.GetPulseHeight1(col,row);
			int ph2 = pixmap.GetPulseHeight2(col,row);
			if (ph1<10000 && ph2<10000)
			{
				int phdiff = ph2-ph1;
				if (ph1<(ph1mean-PH1TOL) || ph1>(ph1mean+PH1TOL) ||
				    phdiff<(ph21mean-PH21TOL) || phdiff>(ph21mean+PH21TOL))
					nPhFail++;
			}
		}
	}

	if (nPixThrOr > nPixDefect) nPixDefect = nPixThrOr;
	if (nPhFail   > nPixDefect) nPixDefect = nPhFail;

	// === chip classification ===========================================

#define CHIPFAIL(code) { failCode = (code); return; }

	failCode = FAIL_NOFAIL;

	// --- class 5 -------------------------------------------------------
	chipClass = pickClass = 5;

	if (bin == 1) CHIPFAIL(FAIL5_BIN1)
	if (bin == 2) CHIPFAIL(FAIL5_BIN2)
	if (bin == 3) CHIPFAIL(FAIL5_BIN3)
	if (bin == 4) CHIPFAIL(FAIL5_BIN4)
	if ((bin == 5) && (nPixDefect>=4000)) CHIPFAIL(FAIL5_DEF)

	// --- class 4 -------------------------------------------------------
	chipClass = pickClass = 4;

	if (nColDefect     >  0) CHIPFAIL(FAIL4_COL)
	if (nPixDefect     > 40) CHIPFAIL(FAIL4_PIXNUM)
	if (nPixThrOr      > 40) CHIPFAIL(FAIL4_PIXNUM)
	if (nPixUnmaskable > 0)  CHIPFAIL(FAIL4_MASK)
	if (nPixAddrDefect > 0)  CHIPFAIL(FAIL4_ADDR)

	if (                   65.0 < IdigOn) CHIPFAIL(FAIL4_IDON)
	if (                   65.0 < IanaOn) CHIPFAIL(FAIL4_IAON)
	if (IdigInit < 20.0 || 40.0 < IdigInit) CHIPFAIL(FAIL4_IDINIT)
	if (IanaInit < 10.0 || 55.0 < IanaInit) CHIPFAIL(FAIL4_IAINIT)

	// --- class 3 -------------------------------------------------------
	chipClass = pickClass = 3;

	if (nPixDefect>4) CHIPFAIL(FAIL3_1PC)

	if (pm    < 30.0 || 80.0 < pm)    CHIPFAIL(FAIL3_TMEAN)
	if (pstd  <  0.5 ||  4.0 < pstd)  CHIPFAIL(FAIL3_TSTD)
//	if (pdiff <  5   || 30   < pdiff) CHIPFAIL(FAIL3_TDIFF)
	if (pm_col_max > 2.5)             CHIPFAIL(FAIL3_TCOL)

	if (ph1mean  < -200.0 || 150.0 < ph1mean)  CHIPFAIL(FAIL3_PHOFFS)
	if (ph1std   >   25)                       CHIPFAIL(FAIL3_PHOFFS)
	if (ph21mean <  100.0 || 300.0 < ph21mean) CHIPFAIL(FAIL3_PHGAIN)
	if (ph21std  >   20)                       CHIPFAIL(FAIL3_PHGAIN)

	if (addressStep <  70.0 || 110.0 < addressStep) CHIPFAIL(FAIL3_ASTEP)
	if (blackLevel  < -12.0 ||  12.0 < blackLevel)  CHIPFAIL(FAIL3_BL)

	// --- class 2 -------------------------------------------------------
	chipClass = 2; pickClass = 1;

	if (nPixDefect>0) CHIPFAIL(FAIL2_1PM)

	// --- class 1 -------------------------------------------------------
	chipClass = pickClass = 1;
}


void CChip::PrintFailString(FILE *f)
{
	switch (failCode)
	{
	case FAIL_NOFAIL: return;

	// --- class 5 -------------------------------------------------------
	case FAIL5_BIN1:
		fprintf(f,"shortcut"); break;

	case FAIL5_BIN2:
		fprintf(f,"Id < 5 mA"); break;

	case FAIL5_BIN3:
		fprintf(f,"no token"); break;

	case FAIL5_BIN4:
		fprintf(f,"I2C"); break;

	case FAIL5_DEF:
		fprintf(f,"%i Pixel defect", nPixDefect); break;

	// --- class 4 -------------------------------------------------------
	case FAIL4_COL:
		fprintf(f,"%i dcol defect", nColDefect); break;

	case FAIL4_PIXNUM:
		fprintf(f,"%i pixel defect", nPixDefect); break;

	case FAIL4_MASK:
		fprintf(f,"%i pixel not maskable", nPixUnmaskable); break;

	case FAIL4_ADDR:
		fprintf(f,"%i pixel address defect", nPixAddrDefect); break;

	case FAIL4_IDON:
		fprintf(f,"IdigOn = %0.1f mA (<65 mA)", IdigOn); break;

	case FAIL4_IAON:
		fprintf(f,"IanaOn = %0.1f mA (<65 mA)", IanaOn); break;

	case FAIL4_IDINIT:
		fprintf(f,"IdigInit = %0.1f mA (20...65 mA)", IdigInit); break;

	case FAIL4_IAINIT:
		fprintf(f,"IanaInit = %0.1f mA (10...55 mA)", IanaInit); break;

	// --- class 3 -------------------------------------------------------
	case FAIL3_1PC:
		fprintf(f,"%i pixel defect (<=1%%)", nPixDefect); break;

	case FAIL3_TMEAN:
		fprintf(f,"Thrshold(mean) = %0.1f (30...80)", pm); break;

	case FAIL3_TSTD:
		fprintf(f,"Threshold(rms) = %0.2f (0.5...4.0)", pstd); break;

	case FAIL3_TDIFF:
		fprintf(f,"Threshold(max-min) = %i (5...30)", pmax-pmin); break;

	case FAIL3_TCOL:
		fprintf(f,"Threshold Col-Col = %0.2f (<2.5)", pm_col_max);
		break;

	case FAIL3_PHOFFS:
		fprintf(f,"Pulse height offset = %0.1f(+/-%0.1f)\n",
			ph1mean, ph1std);
		break;

	case FAIL3_PHGAIN:
		fprintf(f,"Pulse height gain = %0.1f(+/-%0.1f)\n",
			ph21mean, ph21std);
		break;

	case FAIL3_ASTEP:
		fprintf(f,"addr step = %0.1f (70...100)", addressStep); break;

	case FAIL3_BL:
		fprintf(f,"AOUT offset = %0.1f (-10...+10)", black.mean); break;

	// --- class 2 -------------------------------------------------------
	case FAIL2_1PM:
		fprintf(f,"%i pixel defect", nPixDefect); break;

	default:
		fprintf(f,"undefined fail code");
	}
}


bool CDcol::read(CScanner &Log)
{
	int col;
	for (col=0; col<26; col++) dcol[col]=0;
	while (Log.getNextLine()[0] != 0)
	{
		if (strlen(Log.getLine())< 6) return false;
		if (sscanf(Log.getLine()+4,"%i", &col)!=1) return false;
		if (col<0 || col>=26) return false;
		switch(Log.getLine()[0])
		{
		case 'w': dcol[col] |= 0x01; break;
		case 't': dcol[col] |= 0x02; break;
		case 'd': dcol[col] |= 0x04; break;
		default: return false;
		}
	}
	exist = true;
	return true;
}





CChip* CWaferDataBase::GetFirst()
{
	CChip *p = first;
	while (p)
	{
		if (p->multi <= 1) return p;
		p = p->next;
	}
	return NULL;
}

CChip* CWaferDataBase::GetPrev(CChip *chip)
{
	CChip *p = chip ? chip->prev : NULL;
	while (p)
	{
		if (p->multi <= 1) return p;
		p = p->prev;
	}
	return NULL;
}


CChip* CWaferDataBase::GetNext(CChip *chip)
{
	CChip *p = chip ? chip->next : NULL;
	while (p)
	{
		if (p->multi <= 1) return p;
		p = p->next;
	}
	return NULL;
}


bool CWaferDataBase::Add(CChip *chip)
{
	chip->prev = last;
	chip->next = NULL;
	if (last) last->next = chip;
	last = chip;
	if (!first) first = chip;
	return true;
}


void CWaferDataBase::DeleteAll()
{
	CChip *p = first;
	while (p)
	{
		CChip *q = p;
		p = p->next;
		delete q;
	}
	first = last = NULL;
}


void CWaferDataBase::Swap(CChip *entry)
{
	if (entry->next == NULL) return;

	CChip *p = entry->next;
	if (entry->prev) entry->prev->next = p; else first = p;
	if (p->next) p->next->prev = entry; else last = entry;
	entry->next = p->next;
	p->prev = entry->prev;
	entry->prev = p;
	p->next = entry;
}


void CWaferDataBase::SortPicOrder()
{
	if (first == last) return;
	bool swapped;
	do
	{
		swapped = false;
		CChip *p = first;
		while (p && p->next)
		{
			if (*p > *(p->next)) { Swap(p); swapped = true; }
			p = p->next;
		}
	} while (swapped);
}


// chips must be sorted
void CWaferDataBase::CalculateMulti()
{
	CChip *p, *s, *best;
	p = first;
	while (p)
	{
		best = p;
		s = p->next;
		while (s)
		{
			if (!(*s == *p)) break;
			if (s->failCode > best->failCode) best = s;
			s = s->next;
		}
		while(p != s && p)
		{
			p->multi = (p == best) ? 0 : 2;
			p = p->next;
		}
	}
}


void CWaferDataBase::SetPicGroups()
{
	if (first == NULL) return;

	// count groups
	int nGroups = 0;
	int nClass;
	for (nClass=1; nClass<=5; nClass++)
	{
		int gchip = 0;
		CChip *p = first;
		while (p)
		{
			if (p->pickClass == nClass)
			{
				p->pickGroup = nGroups+1;
				gchip++;
				if (gchip>=16) { nGroups++;	gchip = 0; }
			}
			p = p->next;
		}
		if (gchip > 0) nGroups++;
	}
}


double CWaferDataBase::CorrectAoutOffset()
{
	int n = 0;
	double sum = 0.0;
	CChip *p = GetFirstM();
	while (p)
	{
		if (p->black.exist)
		{
			sum += p->black.mean;
			n++;
		}
		p = GetNextM(p);
	}
	if (n == 0) return 0.0;

	aoutOffset = sum/n;

	p = GetFirstM();
	while (p)
	{
		p->SetAoutOffset(aoutOffset);
		p = GetNextM(p);
	}
	return aoutOffset;
}


void CWaferDataBase::Calculate()
{
	CChip *p = GetFirstM();
	while (p)
	{
		p->Calculate();
		p = GetNextM(p);
	}
}


// -----------------------------------------------------------------------

bool CWaferDataBase::GeneratePickFile(char filename[])
{
	if (first == NULL) return false;
	FILE *f = fopen(filename, "wt");
	if (f == NULL) return false;

	// count groups
	int nGroups = 0;
	int nClass;
	for (nClass=1; nClass<=5; nClass++)
	{
		int gchip = 0;
		CChip *p = GetFirst();
		while (p)
		{
			if (p->pickClass == nClass)
			{
				gchip++;
				if (gchip>=16) { nGroups++;	gchip = 0; }
			}
			p = GetNext(p);
		}
		if (gchip > 0) nGroups++;
	}

	time_t t;
	struct tm *dt;
	time(&t);
	dt = localtime(&t);

	fprintf(f, "Wafer: %s\n", first->waferId);
	fprintf(f,"Datum: %i.%i.%i\n", int(dt->tm_mday), int(dt->tm_mon+1), int(dt->tm_year+1900));
	fprintf(f,"Gruppen: %i\n", nGroups);
	fprintf(f,"Kommentar: none\n\n");

	fputs("	Chip#1	Chip#2	Chip#3	Chip#4	Chip#5	Chip#6	Chip#7	Chip#8"
		"	Chip#9	Chip#10	Chip#11	Chip#12	Chip#13	Chip#14	Chip#15	Chip#16 Klasse\n"
		"Gruppe:  X/Y     X/Y     X/Y     X/Y     X/Y     X/Y     X/Y     X/Y"
		    "     X/Y     X/Y     X/Y     X/Y     X/Y     X/Y     X/Y     X/Y\n",f);


	int group = 1;
	for (nClass=1; nClass<=5; nClass++)
	{
		int gchip = 0;
		CChip *p = GetFirst();
		while (p)
		{
			if (p->pickClass == nClass)
			{
				if (gchip == 0) fprintf(f,"  %3i", group);

				fprintf(f,"   %2i/%-2i", p->picX, p->picY);
				gchip++;

				if (gchip>=16)
				{
					group++;
					gchip = 0;
					fprintf(f,"  %4i\n", nClass);
				}
			}
			p = GetNext(p);
		}
		if (gchip > 0)
		{
			while (gchip<16) { fputs("        ",f);  gchip++; }
			fprintf(f,"  %4i\n", nClass);
			group++;
		}
	}

	fclose(f);
	return true;
}


bool CWaferDataBase::WriteXML_File(char path[], CChip &chip)
{
	char filename[80];
	char datetime[24];
	if (!chip.ConvertDate(datetime)) return false;
	sprintf(filename, "%s\\%s_%i%i%c.xml", path, chip.waferId,
		chip.mapY, chip.mapX, "ABCD"[chip.mapPos]);
	FILE *f = fopen(filename,"wt");
	if (f==NULL) return false;

	fputs(
		"<?xml version='1.0' encoding='UTF-8'?>\n"
		"<!DOCTYPE root []>\n"
		" <ROOT>\n"
		" <HEADER>\n"
		"  <TYPE>\n"
		"   <EXTENSION_TABLE_NAME>BRL_ROC_ON_WAFER_TEST</EXTENSION_TABLE_NAME>\n"
		"   <NAME>BRL ROC On-Wafer Test</NAME>\n"
		"  </TYPE>\n"
		"  <RUN>\n", f);
	fprintf(f,
		"   <RUN_NAME>PSI46V2 %s</RUN_NAME>\n"
		"   <RUN_BEGIN_TIMESTAMP>%s</RUN_BEGIN_TIMESTAMP>\n",
		chip.waferId, datetime);
	fputs(
		"   <COMMENT_DESCRIPTION>ROC On-Wafer Test</COMMENT_DESCRIPTION>\n"
		"   <INITIATED_BY_USER>Beat Meier</INITIATED_BY_USER>\n"
		"  </RUN>\n"
		" </HEADER>\n"
		" <DATA_SET>\n", f);
	fprintf(f,
		"  <COMMENT_DESCRIPTION>Test results from PSI for %s</COMMENT_DESCRIPTION>\n"
		"  <DATA_FILE_NAME>%s.txt</DATA_FILE_NAME>\n"
		"  <PART_ASSEMBLY>\n"
		"   <PARENT_PART>\n"
		"    <NAME_LABEL>%s</NAME_LABEL>\n"
		"  </PARENT_PART>\n"
		"   <CHILD_UNIQUELY_IDENTIFIED_BY>\n"
		"     <ATTRIBUTE>\n"
		"      <NAME>BRL ROC Number</NAME>\n"
		"      <VALUE>%i%i%c</VALUE>\n"
		"     </ATTRIBUTE>\n"
		"    </CHILD_UNIQUELY_IDENTIFIED_BY>\n"
		"  </PART_ASSEMBLY>\n", chip.waferId, chip.waferId, chip.waferId,
			chip.mapY, chip.mapX, "ABCD"[chip.mapPos]);
	fputs(
		"  <DATA>\n",f);

	// limit values for data base
	if (chip.chipClass < 0) chip.chipClass = 0;
	if (chip.chipClass > 9) chip.chipClass = 9;

	if (chip.IdigInit > 999.0) chip.IdigInit = 999.0;
	if (chip.IanaInit > 999.0) chip.IanaInit = 999.0;

	if (chip.InitVana > 255) chip.InitVana = 255;

	if (chip.black.exist)
	{
		if      (chip.black.mean < -99.0) chip.black.mean = -99.0;
		else if (chip.black.mean >  99.0) chip.black.mean =  99.0;
	}

	if      (chip.nPixDefect<0)        chip.nPixDefect = 0;
	else if (chip.nPixDefect>4160)     chip.nPixDefect = 4160;

	if      (chip.nPixNoSignal<0)      chip.nPixNoSignal = 0;
	else if (chip.nPixNoSignal>4160)   chip.nPixNoSignal = 4160;

	if      (chip.nPixUnmaskable<0)    chip.nPixUnmaskable = 0;
	else if (chip.nPixUnmaskable>4160) chip.nPixUnmaskable = 4160;

	if      (chip.nPixNoisy<0)         chip.nPixNoisy = 0;
	else if (chip.nPixNoisy>4160)      chip.nPixNoisy = 4160;

	if      (chip.nPixNoTrim<0)        chip.nPixNoTrim = 0;
	else if (chip.nPixNoTrim>4160)     chip.nPixNoTrim = 4160;

	if      (chip.nPixAddrDefect<0)    chip.nPixAddrDefect = 0;
	else if (chip.nPixAddrDefect>4160) chip.nPixAddrDefect = 4160;

	if (chip.bin<0) chip.bin = 0; else if (chip.bin>99) chip.bin=99;

	// begin of data set
	fprintf(f,"   <CHIPCLASS>%u</CHIPCLASS>\n",             chip.chipClass);
	fprintf(f,"   <CURRENT_DIG>%0.1f</CURRENT_DIG>\n",      chip.IdigInit>=0 ? chip.IdigInit : 0.0);
	fprintf(f,"   <CURRENT_ANA>%0.1f</CURRENT_ANA>\n",      chip.IanaInit>=0 ? chip.IanaInit : 0.0);
	fprintf(f,"   <VOLTAGE_ANA>%u</VOLTAGE_ANA>\n",         chip.InitVana>=0 ? chip.InitVana : 0);
	fprintf(f,"   <AOUT_OFFSET>%0.2f</AOUT_OFFSET>\n",      chip.black.exist ? double(chip.black.mean/8) : 0.0);
	fprintf(f,"   <PIXEL_DEFECT>%u</PIXEL_DEFECT>\n",       chip.nPixDefect);
	fprintf(f,"   <NO_SIGNAL>%i</NO_SIGNAL>\n",             chip.nPixNoSignal);
	fprintf(f,"   <MASK_DEFECT>%i</MASK_DEFECT>\n",         chip.nPixUnmaskable);
	fprintf(f,"   <NOISY>%i</NOISY>\n",                     chip.nPixNoisy);
	fprintf(f,"   <TRIM_BIT_DEFECT>%i</TRIM_BIT_DEFECT>\n", chip.nPixNoTrim);
	fprintf(f,"   <ADDRESS_DEFECT>%u</ADDRESS_DEFECT>\n",   chip.nPixAddrDefect);
	fprintf(f,"   <FLAG>%u</FLAG>\n",                       chip.bin);
	// end of data set

	fputs(
		"  </DATA>\n"
		" </DATA_SET>\n"
		"</ROOT>\n", f);

	fclose(f);
	return true;
}


bool CWaferDataBase::GenerateXML(char path[])
{
	CChip *p = GetFirst();
	while (p)
	{
		if (!WriteXML_File(path, *p)) return false;
		p = GetNext(p);
	}
	return true;
}


bool CWaferDataBase::GenerateErrorReport(char filename[])
{
	FILE *f = fopen(filename, "wt");
	if (f==NULL) return false;

	CChip *p = GetFirst();
	while (p)
	{
		if (p->chipClass > 1)
		{
			fprintf(f,"Chip %i%i%c Class %i: ",
				p->mapY, p->mapX, "ABCD"[p->mapPos], p->chipClass);
			p->PrintFailString(f);
			fputs("\n", f);
		}
		p = GetNext(p);
	}

	fclose(f);
	return true;
}


bool CWaferDataBase::GenerateDataTable(char filename[])
{
	FILE *f = fopen(filename, "wt");
	if (f==NULL) return false;

	fprintf(f, "WAFER     POS  PX PY BIN C GR  IDIG0 IANA0 IDIGI IANAI VDREG VDAC  IANA V24  BLLVL ADSTP  DC DD WB TS DB DP  DPIX ADDR TRIM MASK NSIG NOIS THRO T2F T2P  PCNT PMEAN  PSTD PMCOL PMI PMA   NPH PHFAIL PHOMEAN PHOSTD PHGMEAN PHGSTD  FAIL  FAILSTRING\n");
	//          XY4L6GT   03C  11 11 11  4 12  123.5 123.4 137.5 130.0  2.23 2.00  24.0 123   -7.5  68.2  26 26 26 26 26 26  4000 4000 4000 4000 4000 4000 4000   9   9  4000  49.3 12.00 10.56  45  64  4000   4000   100.0   20.0   100.0  101.0    10  xxxxxxxxxxxxxxxx
	CChip *p = GetFirst();
	while (p)
	{
		fprintf(f, "%-9s %i%i%c  %2i %2i %2i  %i %2i  %5.1f %5.1f",
			p->waferId, p->mapY, p->mapX, "ABCD"[p->mapPos], p->picX, p->picY, p->bin,
			p->pickClass, p->pickGroup, p->IdigOn, p->IanaOn);
		if (p->IdigInit>=0.0)  fprintf(f," %5.1f", p->IdigInit);    else fputs("      ",f);
		if (p->IanaInit>=0.0)  fprintf(f," %5.1f", p->IanaInit);    else fputs("      ",f);

		if (p->probecard.isValid)
		{
			fprintf(f," %5.2f %4.2f", p->probecard.vd_reg, p->probecard.v_dac);
		} else fputs("           ",f);

		if (p->InitVana >= 0)  fprintf(f,"  %4.1f %3i", p->InitIana, p->InitVana);
		else fputs("          ",f);
		if (p->black.exist)    fprintf(f,"  %5.1f", p->black.mean); else fputs("       ",f);
		if (p->addressStep>=0) fprintf(f," %5.1f", p->addressStep); else fputs("      ",f);

		if (p->dcol.IsValid())
		{
			int nColDead = 0, nColWBC = 0, nColTS = 0, nColDB = 0, nColNoPix = 0;
			for (int i=0; i<26; i++)
			{
				int res = p->dcol.get(i);
				if (res & 0x01) nColDead++;
				if (res & 0x02) nColWBC++;
				if (res & 0x04) nColTS++;
				if (res & 0x08) nColDB++;
				if (res & 0x10) nColNoPix++;
			}
			fprintf(f,"  %2i %2i %2i %2i %2i %2i",
				p->nColDefect, nColDead, nColWBC, nColTS, nColDB, nColNoPix);
		} else fputs("                   ",f);

		if (p->failCode > CChip::FAIL5_BIN4)
		{
			fprintf(f,"  %4i", p->nPixDefect);
			fprintf(f," %4i",  p->nPixAddrDefect);
			fprintf(f," %4i",  p->nPixNoTrim);
			fprintf(f," %4i",  p->nPixUnmaskable);
			fprintf(f," %4i",  p->nPixNoSignal);
			fprintf(f," %4i",  p->nPixNoisy);
			fprintf(f," %4i",  p->nPixThrOr);

			if (p->pixtest2)
				fprintf(f," %3i %3i", (p->pixtest2 >> 8)&0xff, p->pixtest2 & 0xff);
			else fputs("        ",f);

			fprintf(f,"  %4i", p->n);
			if (p->n > 0)
				fprintf (f," %5.1f %5.2f %5.2f %3i %3i",
					p->pm, p->pstd, p->pm_col_max, p->pmin, p->pmax);
			else fputs("                          ",f);

		} else fputs("                                                                            ",f);

		if (p->chipClass < 5 && p->nPh)
			fprintf(f,"  %4i   %4i %7.1f %6.1f %7.1f %6.1f",
				p->nPh, p->nPhFail, p->ph1mean, p->ph1std,
				p->ph21mean, p->ph21std);
			else fputs("                                           ",f);

		fprintf(f, "   %3i  ", p->failCode);
		p->PrintFailString(f);
		fputs("\n", f);
		p = GetNext(p);
	}
	fclose(f);
	return true;
}



/*
   wafer #chips 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
*/
bool CWaferDataBase::GenerateStatistics(const char filename[])
{
	FILE *f = fopen(filename, "wt");
	if (f==NULL) return false;

	CChip *p = GetFirst();
	if (!p) { fclose(f); return false; }

	int i;
	int n = 0;
	int fail[24] = { 0 };
	int cl[5] = { 0 };

	fprintf(f,"wafer:  %s\n", p->waferId);

	while (p)
	{
		fail[p->failCode]++;
		cl[p->chipClass-1]++;
		n++;
		p = GetNext(p);
	}

	fprintf(f,"#Chips: %4i\n#fail: ", n);
	for (i=0; i<24; i++) fprintf(f," %4i", fail[i]);
	fputs("\n%fail: ",f);
	for (i=0; i<24; i++) fprintf(f," %4.1f", fail[i]*100.0/n);
	fprintf(f,"\n#Class:  ");
	for (i=0; i<5; i++) fprintf(f," %4i", cl[i]);
	fputs("\n%Class:  ",f);
	for (i=0; i<5; i++) fprintf(f," %4.1f", cl[i]*100.0/n);
	fputs("\n",f);
	fclose(f);
	return true;
}


#define MAXBINCOUNT 25

#define WMAPX  12
#define WMAPY  12
#define WMAPOFFSX  0
#define WMAPOFFSY  0


bool CWaferDataBase::GenerateWaferMap(char filename[], unsigned int mode)
{
	int bincount;
	switch (mode)
	{
		case 0: // mode 0; bin
			bincount = 13; break;
		case 1: // mode 1: fail
			bincount = 24; break;
		case 2: // mode 2: class
			bincount = 5; break;
		default: return false;
	}

	CPostscript ps;

	int bin, yield[MAXBINCOUNT];
	for (bin=0; bin<bincount; bin++) yield[bin] = 0;

	if (!ps.open(filename)) return false;
	ps.putTempl("");
	ps.putTempl("prolog_begin.tmpl");
	ps.putTempl("wmap.tmpl");
	ps.putTempl("prolog_end.tmpl");

	CChip *p = GetFirst();
	if (p)
	{
		ps.addPage();
		ps.puts("LOCAL begin\n");
		switch (mode)
		{
			case 1:  ps.printf("/plotType(fail code)def\n"); break;
			case 2:  ps.printf("/plotType(class)def\n"); break;
			default: ps.printf("/plotType(bin)def\n"); break;
		}
		ps.printf("/productId(%s)def\n", p->productId);
		ps.printf("/waferId(%s)def\n",   p->waferId);
		ps.printf("/testDate(%s)def\n",  p->startTime);
		switch (mode)
		{
			case 1:  ps.printf("/legend {legend_fail} def\n");  break;
			case 2:  ps.printf("/legend {legend_class} def\n"); break;
			default: ps.printf("/legend {legend_bin} def\n");   break;
		}
	}

	while (p)
	{
		switch (mode)
		{
			case 1:  bin = p->failCode;  break;
			case 2:  bin = p->chipClass-1; break;
			default: bin = p->bin;
		}
		if (0<=bin && bin<bincount) yield[bin]++;
		p = GetNext(p);
	}
	ps.printf("/yield [");
	for (bin=0; bin<bincount; bin++) ps.printf(" %i", yield[bin]);
	ps.printf("]def\n[\n");

	p = GetFirst();
	while (p)
	{
		switch (mode)
		{
			case 1:  bin = COLOR_FAIL[p->failCode];  break;
			case 2:  bin = COLOR_CLASS[p->chipClass-1]; break;
			default: bin = COLOR_BIN[p->bin];
		}
		ps.printf("[%i %i %i %i]\n",
		bin, p->mapPos, p->mapX-WMAPOFFSX, p->mapY-WMAPOFFSY);
		p = GetNext(p);
	}

	ps.printf("]wafermapPage\nend showpage\n");
	ps.close();
	return true;
}
