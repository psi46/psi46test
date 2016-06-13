/* -------------------------------------------------------------
 *
 *  file:        command.cpp
 *
 *  description: command line interpreter for Chip/Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    13.6.2016
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

//  23
//  01


CMD_PROC(roctype)
{
	char s[256];
	PAR_STRING(s,250);

	if (strcmp(s, "ana") == 0) settings.rocType = 0;
	else if (strcmp(s, "dig") == 0) settings.rocType = 1;
	else if (strcmp(s, "proc600") == 0) settings.rocType = 2;
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
	int rsp;
	Log.section("WAFER", false);

	// ProductID
	rsp = prober.SendCmd("GetProductID");
	if (rsp != 0)
	{
		printf("missing wafer product id!\n");
		Log.printf("productId?\n");
		return false;
	}
	Log.printf("%s", prober.GetParamString());
	strcpy(g_chipdata.productId, prober.GetParamString());

	// WaferID
	const char *msg;
	if (settings.IsWaferList()) msg = waferList.GetId().c_str();
	else
	{
		rsp = prober.SendCmd("GetWaferID");
		if (rsp != 0)
		{
			printf(" missing wafer id!\n");
			Log.printf(" waferId?\n");
			return false;
		}
	}
	Log.printf(" %s", prober.GetParamString());
	strcpy(g_chipdata.waferId, prober.GetParamString());

	// Wafer Number
	int num;
	rsp = prober.SendCmd("GetWaferNum");
	if (rsp == 0) if (sscanf(prober.GetParamString(), "%i", &num) == 1)
	{
		Log.printf(" %i\n", num);
		strcpy(g_chipdata.waferNr, prober.GetParamString());
		return true;
	}

	printf(" missing wafer number!\n");
	Log.printf(" wafernum?\n");
	return false;
}


bool ReportChip(int &x, int &y)
{
	if (prober.SendCmd("ReadMapPosition") != 0) return false;

	float posx, posy;
	if (sscanf(prober.GetParamString(),
		"%i %i %f %f", &x, &y, &posx, &posy) != 4)
	{
		printf(" error reading chip information\n");
		return false;
	}
	nEntry++;
	CChipPos map(x, y, CChipPos::Id2Pos(chipPos));
	std::string s;
	map.WriteString(s);
	printf("#%05i: %s -> ", nEntry, s.c_str());
	fflush(stdout);
	Log.section("CHIP", false);
	Log.printf(" %i %i %c %9.1f %9.1f\n",
		map.GetX(), map.GetY(), map.GetPos(), posx, posy);
	g_chipdata.map = map;
	return true;
}


CMD_PROC(pr)
{
	char s[256];
	PAR_STRINGEOL(s,250);

	printf(" REQ %s\n", s);
	int rsp = prober.SendCmd("%s", s);
	if (rsp == 0) printf(" RSP %s\n", prober.GetRspString());
}


CMD_PROC(sep)
{
	prober.SendCmd("MoveChuckSeparation");
}


CMD_PROC(contact)
{
	prober.SendCmd("MoveChuckContact");
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
	int bin = test_roc(repeat);
	tb.SetLed(0x00);
	tb.Flush();
	GetTimeStamp(g_chipdata.endTime);
	Log.timestamp("END");
	Log.puts("\n");
	Log.flush();
	printf("%3i\n", bin);

	prober.SendCmd("BinMapDie %i", bin);

	return true;
}


bool test_chip(char chipid[])
{
	nEntry++;

	g_chipdata.Invalidate();
	g_chipdata.nEntry = nEntry;
	printf("#%05i: %s -> ", nEntry, chipid);
	fflush(stdout);
	Log.section("CHIP1", false);
	Log.printf(" %s\n", chipid);
	strcpy(g_chipdata.chipId, chipid);

	GetTimeStamp(g_chipdata.startTime);
	Log.timestamp("BEGIN");

	tb.SetLed(0x10);
	bool repeat;
	int bin = test_roc(repeat);
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
	else
	{
		char id[42];
		PAR_STRINGEOL(id,40);
		test_chip(id);
	}

//	FILE *f = fopen("g_chipdata.txt", "wt");
//	if (f) { g_chipdata.Save(f);  fclose(f); }
}


// PROC600_V2 Wafer (13.6.2016)
#define CSX   8050
#define CSY  10723

const int CHIPOFFSET[4][4][2] =
{	// from -> to  0           1           2           3
	/*   0  */ { {   0,   0},{-CSX,   0},{   0,-CSY},{-CSX,-CSY} },
	/*   1  */ { { CSX,   0},{   0,   0},{ CSX,-CSY},{   0,-CSY} },
	/*   2  */ { {   0, CSY},{-CSX, CSY},{   0,   0},{-CSX,   0} },
	/*   3  */ { { CSX, CSY},{   0, CSY},{ CSX,   0},{   0,   0} },
};

bool ChangeChipPos(int pos)
{
	if (prober.SendCmd("MoveChuckSeparation") != 0) return false;

	int x = CHIPOFFSET[chipPos][pos][0];
	int y = CHIPOFFSET[chipPos][pos][1];
	if (prober.SendCmd("MoveChuckPosition %i %i H", x, y) != 0) return false;

	if (prober.SendCmd("SetMapHome") != 0) return false;

	chipPos = pos;
	return true;
}


CMD_PROC(chippos)
{
	char s[4];
	PAR_STRING(s,2);
	if (s[0] >= 'a') s[0] -= 'a' - 'A';

	if (!(s[0] && strchr("ABCD", s[0])))
	{
		printf("Wrong chip pos!\n");
		return;
	}

	ChangeChipPos(CChipPos::Pos2Id(s[0]));
}


CDefectList deflist[4];


bool goto_def(int i)
{
	int x, y;
	if (!deflist[chipPos].get(i, x, y)) return false;
	
	return prober.SendCmd("StepNextDie %i %i", x, y) == 0;
}


bool go_TestDefects()
{
	if (deflist[chipPos].size() == 0) return true;

	printf(" Begin Defect Chip %c Test\n", CChipPos::Id2Pos(chipPos));

	// goto first position
	int i = 0;
	if (!goto_def(i)) return false;

	prober.SendCmd("MoveChuckContact");

	do
	{
		int x, y;
		g_chipdata.Invalidate();

		if (!ReportChip(x,y)) break;
		GetTimeStamp(g_chipdata.startTime);
		Log.timestamp("BEGIN");
		bool repeat;
		int bin = test_roc(repeat);
		GetTimeStamp(g_chipdata.endTime);
		Log.timestamp("END");
		Log.puts("\n");
		Log.flush();
		printf("%3i\n", bin);
		prober.SendCmd("BinMapDie %i", bin);

		if (keypressed())
		{
			printf(" wafer test interrupted!\n");
			break;
		}

		tb.mDelay(100);
		i++;
	} while (goto_def(i));

	prober.SendCmd("MoveChuckSeparation");

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
	bin = test_roc(repeat);
	tb.SetLed(0x00);
	tb.Flush();

	GetTimeStamp(g_chipdata.endTime);
	Log.timestamp("END");
	Log.puts("\n");
	Log.flush();
	printf("%3i\n", bin);
	return true;
}


bool IsExcludedChip(CChipPos p)
{
	for (unsigned int i = 0; i < settings.waferExclude.size(); i++)
	{
		if (p == settings.waferExclude[i]) return true;
	}
	return false;
}


bool go_TestChips()
{
	int x, y;
	int rsp;

	prober.SendCmd("MoveChuckSeparation");

	// --- read actual map position
	if (prober.SendCmd("ReadMapPosition")) return false;
	
	if (sscanf(prober.GetParamString(), "%i %i", &x, &y) != 2) return false;
	CChipPos map(x, y, CChipPos::Id2Pos(chipPos));

	// --- skip excluded chips
	while (IsExcludedChip(map))
	{
		rsp = prober.SendCmd("StepNextDie");
		if (rsp == 703) return true; // end of wafer
		if (rsp != 0) return false; // error

		if (sscanf(prober.GetParamString(), "%i %i", &x, &y) != 2)
			return false;
		map.Set(x, y, CChipPos::Id2Pos(chipPos));
	}

	// --- loop all chips -------------------------------------------
	printf(" Begin Chip %c Test\n", CChipPos::Id2Pos(chipPos));

	while (true)
	{
		int bin = 0;
		bool repeat;
		// --- test chip
		prober.SendCmd("MoveChuckContact");
		if (!TestSingleChip(bin,repeat)) break;
		prober.SendCmd("MoveChuckSeparation");

		// --- retest chip if not working
		int nRep = settings.errorRep;
		while (nRep > 0 && repeat)
		{
			prober.SendCmd("BinMapDie %i", bin);
			tb.mDelay(100);
			prober.SendCmd("MoveChuckContact");
			tb.mDelay(300);
			if (!TestSingleChip(bin,repeat)) repeat = false;
			prober.SendCmd("MoveChuckSeparation");
			nRep--;
		}

		// --- manual interruption
		if (keypressed())
		{
			prober.SendCmd("BinMapDie %i", bin);
			printf(" wafer test interrupted!\n");
			break;
		}

		// --- step to the next chip
		int rsp = prober.SendCmd("BinStepDie %i", bin);
		if (rsp == 703) return true; // end of wafer
		if (rsp != 0) break; // error

		sscanf(prober.GetParamString(), "%i %i", &x, &y);
		map.Set(x, y, CChipPos::Id2Pos(chipPos));

		// --- skip excluded chips
		while (IsExcludedChip(map))
		{
			rsp = prober.SendCmd("StepNextDie");
			if (rsp == 703) return true;  // end of wafer
			if (rsp != 0) return false;   // prober error

			if (sscanf(prober.GetParamString(), "%i%i", &x, &y) != 2)
			{
				printf(" RSP no response\n");
				return false;
			}

			map.Set(x, y, CChipPos::Id2Pos(chipPos));
		}
	} // while

	return false;
}


bool GotoFirstChipPos()
{
	for (int i = 0; i < 4; i++)
	{
		char p = CChipPos::Id2Pos(i);
		if (settings.waferMask.find(p) == std::string::npos)
		{
			ChangeChipPos(i);
			return true;
		}
	}
	return false;
}


bool GotoNextChipPos()
{
	for (int i = chipPos + 1; i < 4; i++)
	{
		char p = CChipPos::Id2Pos(i);
		if (settings.waferMask.find(p) == std::string::npos)
		{
			ChangeChipPos(i);
			return true;
		}
	}
	return false;
}


CMD_PROC(go)
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
		if (!GotoFirstChipPos())
		{
			printf("Nothing to test.\n");
			return;
		};
		for (int k=0; k<4; k++) deflist[k].clear();
		prober.SendCmd("StepFirstDie");
		isRunning = true;
	}

	printf(" wafer test running\n");
	if (!ReportWafer()) return;

	while (true)
	{
		// test chips
		if (!go_TestChips()) break;

		// test defect chips
		prober.SendCmd("StepFirstDie");
		if (!go_TestDefects()) break;

		// next chip position
		if (GotoNextChipPos())
		{
			if (prober.SendCmd("StepFirstDie") != 0) break;
		}
		else
		{
			ChangeChipPos(0);
			isRunning = false;
			if (settings.IsWaferList()) waferList.SetTested();
			break;
		}
	}
}


CMD_PROC(first)
{
	prober.SendCmd("StepFirstDie");
}


CMD_PROC(next)
{
	prober.SendCmd("StepNextDie");
}


CMD_PROC(goto)
{
	int x, y;
	PAR_INT(x, -100, 100);
	PAR_INT(y, -100, 100);

	prober.SendCmd("StepNextDie %i %i", x, y);
}
