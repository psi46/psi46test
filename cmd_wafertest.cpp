/* -------------------------------------------------------------
 *
 *  file:        command.cpp
 *
 *  description: command line interpreter for Chip/Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    31.8.2007
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */


#include "cmd.h"


// =======================================================================
//  chip/wafer test commands
// =======================================================================


int chipPos = 0;

char chipPosChar[] = "ABCD";

CMD_PROC(roctype)
{
	char s[256];
	PAR_STRING(s,250);

	if (strcmp(s, "ana") == 0) settings.rocType = 0;
	else if (strcmp(s, "dig") == 0) settings.rocType = 1;
	else printf("choose ana or dig\n");
}


void GetTimeStamp(char datetime[])
{
	time_t t;
	struct tm *dt;
	time(&t);
	dt = localtime(&t);
	strcpy(datetime, asctime(dt));
}


bool ReportWafer()
{
	char *msg;
	Log.section("WAFER", false);

	// ProductID
	msg = prober.printf("GetProductID");
	if (strlen(msg)<=3)
	{
		printf("missing product id!\n"); //--new  tolto 'wafer'.
		Log.printf("productId?\n");
		return false;
	}
	Log.printf("%s", msg+3);
	strcpy(g_chipdata.productId, msg+3);

	// WaferID
	msg = prober.printf("GetWaferID");
	if (strlen(msg)<=3)
	{
		printf(" missing wafer id!\n");
		Log.printf(" waferId?\n");
		return false;
	}
	Log.printf(" %s", msg+3);
	strcpy(g_chipdata.waferId, msg+3);

	// Wafer Number
	int num;
	msg = prober.printf("GetWaferNum");
	if (strlen(msg)>3) if (sscanf(msg+3, "%i", &num) == 1)
	{
		Log.printf(" %i\n", num);
		strcpy(g_chipdata.waferNr, msg+3);
		return true;
	}

	printf(" missing wafer number!\n");
	Log.printf(" wafernum?\n");
	return false;
}


bool ReportChip(int &x, int &y)
{
	char *pos = prober.printf("ReadMapPosition");
	int len = strlen(pos);
	if (len<3)
	{ 
		printf(" error reading chip information - strlen<3 \n"); //---new
		return false; 
	}

	pos += 3;

	float posx, posy;
	if (sscanf(pos, "%i %i %f %f", &x, &y, &posx, &posy) != 4)
	{
		printf(" error reading chip information\n");
		return false;
	}
	nEntry++;
	printf("#%05i: %i%i%c -> ", nEntry, y, x, chipPosChar[chipPos]);
	fflush(stdout);
	Log.section("CHIP", false);
	Log.printf(" %i %i %c %9.1f %9.1f\n",
		x, y, chipPosChar[chipPos], posx, posy);
	g_chipdata.mapX   = x;
	g_chipdata.mapY   = y;
	g_chipdata.mapPos = chipPos;
	return true;
}


CMD_PROC(pr)
{
	char s[256];
	PAR_STRINGEOL(s,250);

	printf(" REQ %s\n", s);
	char *answer = prober.printf("%s", s);
	printf(" RSP %s\n", answer);
}


CMD_PROC(sep)
{
	prober.printf("MoveChuckSeparation");
}


CMD_PROC(contact)
{
	prober.printf("MoveChuckContact");
}


bool test_wafer()
{
	int x, y;

	g_chipdata.Invalidate();

	if (!ReportWafer()) return true;
	if (!ReportChip(x,y)) return true;
	g_chipdata.nEntry = nEntry;

	GetTimeStamp(g_chipdata.startTime);
	Log.timestamp("BEGIN");
	tb.SetLed(0x10);
	bool repeat;
	int bin = settings.rocType == 0 ? TestRocAna::test_roc(repeat) : TestRocDig::test_roc(repeat);
	tb.SetLed(0x00);
	tb.Flush();
	GetTimeStamp(g_chipdata.endTime);
	Log.timestamp("END");
	Log.puts("\n");
	Log.flush();
	printf("%3i\n", bin);

	printf(" RSP %s\n", prober.printf("BinMapDie %i", bin));

	return true;
}


bool test_chip(char chipid[])
{
	if (settings.proberPort != -2)  //---new to alessi. aggiunto questo if!
	{
		nEntry++;

		g_chipdata.Invalidate();
		g_chipdata.nEntry = nEntry;
		printf("#%05i: %s -> ", nEntry, chipid);
		fflush(stdout);
		Log.section("CHIP1", false);
		Log.printf(" %s\n", chipid);
		strcpy(g_chipdata.chipId, chipid);
	}

	GetTimeStamp(g_chipdata.startTime);
	Log.timestamp("BEGIN");

	tb.SetLed(0x10);
	bool repeat;
	int bin = settings.rocType == 0 ? TestRocAna::test_roc(repeat) : TestRocDig::test_roc(repeat);
	tb.SetLed(0x00);
	tb.Flush();

	GetTimeStamp(g_chipdata.endTime);
	Log.timestamp("END");
	Log.puts("\n");
	Log.flush();

	printf("%3i\n", bin);

	return true;
}


CMD_PROC(test)
{

	if (settings.proberPort >= 0)
	{
		test_wafer();
	}
		//::::::::::::::::::::::::::::::::::::::::::::::::::
	else if (settings.proberPort == -2) //---new to Alessi
	{
		// per inserire numero chip nel log file
		char id[42];
		PAR_STRINGEOL(id,40);
									
		//va bene qui?
		g_chipdata.Invalidate();
		nEntry++;
		g_chipdata.nEntry = nEntry;

		int x, y;
		float posx, posy;
		if (sscanf(id, "%i %i %c %f %f", &x, &y, &chipPosChar[chipPos], &posx, &posy) != 5)
		{
		printf("%i %i %c %9.1f %9.1f\n", x, y, chipPosChar[chipPos], posx, posy);
		printf(" error reading chip information\n");
		return; //false
		}
		printf("#%05i: %i%i%c -> ", nEntry, x, y, chipPosChar[chipPos]);
		fflush(stdout);
		Log.section("CHIP", false);
		Log.printf(" %i %i %c %9.1f %9.1f\n", x, y, chipPosChar[chipPos], posx, posy);
		g_chipdata.mapX   = x;
		g_chipdata.mapY   = y;
		g_chipdata.mapPos = chipPos;
		
		test_chip(id);
	}
	//::::::::::::::::::::::::::::::::::::::::::::::::::
	else
	{
		char id[42];
		PAR_STRINGEOL(id,40);
		test_chip(id);
	}

//	FILE *f = fopen("g_chipdata.txt", "wt");
//	if (f) { g_chipdata.Save(f);  fclose(f); }
}


//---new
#define CSX   8610  //old 8050
#define CSY  10840  //old 10451

const int CHIPOFFSET[4][4][2] =
{	// from -> to  0           1           2           3
	/*   0  */ { {   0,   0},{-CSX,   0},{   0,-CSY},{-CSX,-CSY} },
	/*   1  */ { { CSX,   0},{   0,   0},{ CSX,-CSY},{   0,-CSY} },
	/*   2  */ { {   0, CSY},{-CSX, CSY},{   0,   0},{-CSX,   0} },
	/*   3  */ { { CSX, CSY},{   0, CSY},{ CSX,   0},{   0,   0} },
};

bool ChangeChipPos(int pos)
{
	int rsp;
	char *answer = prober.printf("MoveChuckSeparation");
	if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
	if (rsp != 0) { printf(" RSP %s\n", answer); return false; }

	int x = CHIPOFFSET[chipPos][pos][0];
	int y = CHIPOFFSET[chipPos][pos][1];

	answer = prober.printf("MoveChuckPosition %i %i H", x, y);
	if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
	if (rsp != 0) { printf(" RSP %s\n", answer); return false; }

	answer = prober.printf("SetMapHome");
	if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
	if (rsp != 0) { printf(" RSP %s\n", answer); return false; }

	chipPos = pos;
	return true;
}


CMD_PROC(chippos)
{
	char s[4];
	PAR_STRING(s,2);
	if (s[0] >= 'a') s[0] -= 'a' - 'A';
	//---new if (s[0] == 'B') return; // chip B not existing

	int i;
	for (i=0; i<4; i++)
	{
		if (s[0] == chipPosChar[i])
		{
			ChangeChipPos(i);
			return;
		}
	}
}


CDefectList deflist[4];


bool goto_def(int i)
{
	int x, y;
	if (!deflist[chipPos].get(i, x, y)) return false;
	char *answer = prober.printf("StepNextDie %i %i", x, y);

	int rsp;
	if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
	if (rsp!=0) printf(" RSP %s\n", answer);

	return rsp == 0;
}


bool go_TestDefects()
{
	if (deflist[chipPos].size() == 0) return true;

	printf(" Begin Defect Chip %c Test\n", chipPosChar[chipPos]);

	// goto first position
	int i = 0;
	if (!goto_def(i)) return false;

	prober.printf("MoveChuckContact");

	do
	{
		int x, y;
		g_chipdata.Invalidate();

		if (!ReportChip(x,y)) break;
		GetTimeStamp(g_chipdata.startTime);
		Log.timestamp("BEGIN");
		bool repeat;
		int bin = settings.rocType == 0 ? TestRocAna::test_roc(repeat) : TestRocDig::test_roc(repeat);
		GetTimeStamp(g_chipdata.endTime);
		Log.timestamp("END");
		Log.puts("\n");
		Log.flush();
		printf("%3i\n", bin);
		prober.printf("BinMapDie %i", bin);

		if (keypressed())
		{
			printf(" wafer test interrupted!\n");
			break;
		}

		tb.mDelay(100);
		i++;
	} while (goto_def(i));

	prober.printf("MoveChuckSeparation");

	return true;
}


bool TestSingleChip(int &bin, bool &repeat)
{
	int x, y;
	g_chipdata.Invalidate();

	if (!ReportChip(x,y)) return false;
	GetTimeStamp(g_chipdata.startTime);
	Log.timestamp("BEGIN");
	tb.SetLed(0x10);
	bin = settings.rocType == 0 ? TestRocAna::test_roc(repeat) : TestRocDig::test_roc(repeat);
	tb.SetLed(0x00);
	tb.Flush();

	//		if (0<bin && bin<13) deflist[chipPos].add(x,y);
	GetTimeStamp(g_chipdata.endTime);
	Log.timestamp("END");
	Log.puts("\n");
	Log.flush();
	printf("%3i\n", bin);
	return true;
}


bool go_TestChips()
{
	printf(" Begin Chip %c Test\n", chipPosChar[chipPos]);
	prober.printf("MoveChuckContact");
	tb.mDelay(200);

	while (true)
	{
		int bin = 0;
		bool repeat;
		if (!TestSingleChip(bin,repeat)) break;

		int nRep = settings.errorRep;
		if (nRep > 0 && repeat)
		{
			prober.printf("BinMapDie %i", bin);
			prober.printf("MoveChuckSeparation");
			tb.mDelay(100);
			prober.printf("MoveChuckContact");
			tb.mDelay(200);
			if (!TestSingleChip(bin,repeat)) break;
			nRep--;
		}

		if (keypressed())
		{
			prober.printf("BinMapDie %i", bin);
			printf(" wafer test interrupted!\n");
			break;
		}

		// prober step
		int rsp;
		char *answer = prober.printf("BinStepDie %i", bin);
		if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
		if (rsp != 0) printf(" RSP %s\n", answer);
		tb.mDelay(100);

		// last chip ?
		if (rsp == 0)   // ok -> next chip
			continue;
		if (rsp == 703) // end of wafer -> return
		{
			prober.printf("MoveChuckSeparation");
			return true;
		}

		printf(" prober error! test stopped\n");
		break;
	}

	prober.printf("MoveChuckSeparation");
	return false;
}

//:::::::::::::::::::::::::::::::
CMD_PROC(testdiced)  //---new
{
	prober.printf("MoveChuckContact");
	tb.mDelay(200);  //controllare i delay...

	while(true)
	{
		int bin = 0;
		bool repeat;
		
		if (!TestSingleChip(bin,repeat)) break;  //torna sempre true a parte quando non trova info del Chip.

		if(repeat)
		{
			int nRep = settings.errorRep;
			for (nRep; nRep > 0; nRep--)  //inserito da me.
			{
				prober.printf("BinMapDie %i", bin);
				printf(" test will be repeated\n");
				prober.printf("MoveChuckSeparation");
				tb.mDelay(100);
				prober.printf("MoveChuckContact");
				tb.mDelay(200);
				if (!TestSingleChip(bin,repeat)) break;   //torna sempre true a parte quando non trova info del Chip.
				if(!repeat) break;
			}
		}

		if (keypressed())  //OK. funziona solo dopo fine test
		{
			prober.printf("BinMapDie %i", bin);
			prober.printf("MoveChuckSeparation");
			printf(" wafer test interrupted!\n");
			printf(" to continue run again 'testdiced'\n");
			break;
		}

   		// prober step  
		int rsp;
		prober.printf("MoveChuckSeparation");
		tb.mDelay(100); //-- controllare delay
		printf(" test ended, moving on next die\n");
		char *answer = prober.printf("BinStepDie %i", bin);
		if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
		if (rsp != 0) printf(" RSP %s\n", answer);  //è risposta attesa?
		tb.mDelay(100);

		// check if last chip
		if (rsp == 0) // ok -> next chip
		{
			prober.printf("MoveChuckSeparation"); //-- ridondante.....
		    printf(" align this chip and run 'testdiced'\n");
			return; // true
		}
		if (rsp == 703) // end of wafer -> return
		{
			if (chipPos < 3) //funziona
			{
			   prober.printf("MoveChuckSeparation"); //-- ridondante .....
			   printf(" WARNING! chip %c test completed\n", chipPosChar[chipPos]);
			   printf(" RUN: initestdiced %c\n", chipPosChar[chipPos+1]);
			   return; // true
			}
			else
			{
			   prober.printf("MoveChuckSeparation"); //-- ridondante........
			   printf(" WARNING! chip %c test completed\n", chipPosChar[chipPos]);
			   printf(" End of Wafer Test - Good Job!\n");
			   prober.printf("MoveChuckLoad");
			   return; // true
			 }
		}

		printf(" prober error! rsp not valid\n");
		break;
	}
	prober.printf("MoveChuckSeparation");
	return; // true
}

CMD_PROC(initestdiced)  //---new
{
	char s[4];
	PAR_STRING(s,2);
	if (s[0] >= 'a') s[0] -= 'a' - 'A'; //cambia minuscole in maiuscole..
	
	int i;
	bool sgood = false;
	for (i=0; i<4; i++)
	{
		if (s[0] == chipPosChar[i])
		{
			printf(" probes on new home die\n"); //vedere se fermarlo qui...
			ChangeChipPos(i);
			chipPos = i;
			sgood = true;
		}
	}

	if(!sgood) 
	{
		printf(" ERROR - invalid argument, it must be: A,B,C or D\n"); 
		return; // false
	}

	//far fare align manuale e poi far partire con un tasto?...
	tb.mDelay(100);
	prober.printf("StepFirstDie");
	printf(" probes on first die\n");
	
	if (s[0] == 'A')
	{  
		printf(" wafer test running\n");
		if (!ReportWafer())	return; // true
	}

	tb.mDelay(200); //non so se ha senso..
	printf(" Begin Chip %c Test\n", chipPosChar[chipPos]);
	printf(" initialization done -> align this chip and run 'testdiced'\n");
	
	return; // true
}
//:::::::::::::::::::::::::

CMD_PROC(go) //---new disabled
{
	static bool isRunning = false;

	char s[12];
	if (PAR_IS_STRING(s, 10))
	{
		if (strcmp(s,"init") == 0) { isRunning = false; }
		else if (strcmp(s,"cont") == 0) { isRunning = true; }
		else { printf(" illegal parameter");  return; }
	}

	if (!isRunning)
	{
		ChangeChipPos(0);
		for (int k=0; k<4; k++) deflist[k].clear();
		prober.printf("StepFirstDie");
		isRunning = true;
	}

	printf(" wafer test running\n");
	if (!ReportWafer()) return;

	while (true)
	{
		// test chips
		if (!go_TestChips()) break;

		// test defect chips
		prober.printf("StepFirstDie");
		if (!go_TestDefects()) break;

		// next chip position
		if (chipPos < 3)
		{
			if (chipPos != 0) // exclude chip B (1)
			{
				if (!ChangeChipPos(chipPos+1)) break;
			}
			else
			{
				if (!ChangeChipPos(chipPos+1)) break;
//				if (!ChangeChipPos(chipPos+2)) break; // exclude chip B (1)
			}
			char *answer = prober.printf("StepFirstDie");
			int rsp;
			if (sscanf(answer, "%i", &rsp)!=1) rsp = -1;
			if (rsp != 0)
			{
				printf(" RSP %s\n", answer);
				break;
			}
		}
		else
		{
			ChangeChipPos(0);
			isRunning = false;
			break;
		}
	}
}


CMD_PROC(first)
{
	printf(" RSP %s\n", prober.printf("StepFirstDie"));
}


CMD_PROC(next)
{
	printf(" RSP %s\n", prober.printf("StepNextDie"));
}


CMD_PROC(goto)
{
	int x, y;
	PAR_INT(x, -100, 100);
	PAR_INT(y, -100, 100);

	char *msg = prober.printf("StepNextDie %i %i", x, y);
	printf(" RSP %s\n", msg);
}



// -- Wafer Test Adapter commands ----------------------------------------
/*
CMD_PROC(vdreg)    // regulated VD
{
	double v = tb.GetVD_Reg();
	printf("\n VD_reg = %1.3fV\n", v);
}

CMD_PROC(vdcap)    // unregulated VD for contact test
{
	double v = tb.GetVD_CAP();
	printf("\n VD_cap = %1.3fV\n", v);
}

CMD_PROC(vdac)     // regulated VDAC
{
	double v = tb.GetVDAC_CAP();
	printf("\n V_dac = %1.3fV\n", v);
}
*/
