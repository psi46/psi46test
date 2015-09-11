/* -------------------------------------------------------------
 *
 *  file:        command.cpp
 *
 *  description: command line interpreter for Chip/Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    25.11.2014
 *  modified:    15.01.2015 Martino Dall'Osso --new
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */


#include "cmd.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <stdlib.h>


// =======================================================================
//  chip/wafer test commands
// =======================================================================


int chipPos = 0;

//  23
//  01

char chipPosChar[] = "CDAB";

struct coordinates
	{
	int N, X, Y;
	char Letter;
	float posX, posY;
	};

coordinates * crd = new coordinates [500]; //--new
int NROC;

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
	Log.section("CENTER", false);
	// CenterID
	if (strlen(settings.centerId.c_str())<=3)
	{
		printf("missing [CENTER]. set it into '.ini' file\n");
		Log.printf("centerId?\n");
		return false;
	}
	Log.printf("%s ", settings.centerId.c_str());
	strcpy(g_chipdata.centerId, settings.centerId.c_str());

	Log.section("WAFER", false);

	// ProductID
	if (strlen(settings.rocName.c_str())<=3)
	{
		printf("missing product id! set it into '.ini' file\n");
		Log.printf("productId?\n");
		return false;
	}
	Log.printf("%s ", settings.rocName.c_str());
	strcpy(g_chipdata.productId, settings.rocName.c_str());

	// WaferID
	if (strlen(settings.waferId.c_str())<=3)
	{
		printf("missing waferID! set it into '.ini' file\n");
		Log.printf("waferId?\n");
		return false;
	}
	Log.printf("%s", settings.waferId.c_str());
	strcpy(g_chipdata.waferId, settings.waferId.c_str());

	// Wafer Number
	if (strlen(settings.waferNum.c_str())>=1)
	{
		Log.printf(" %s\n", settings.waferNum.c_str());
		strcpy(g_chipdata.waferNr, settings.waferNum.c_str());
		return true;
	}
	printf("missing waferNum! set it into '.ini' file\n");
	Log.printf("waferNum?\n");
	return false;	
}

bool ReportChip(int &x, int &y) //old - not for Alessi
{
	char *pos = prober.printf("ReadMapPosition");
	int len = strlen(pos);
	if (len<3) return false;
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

bool ReadPosition(float &posx, float &posy)  //---new
{
	char st1[] = "QP D";
	char *psx = prober.printf("%s", st1);
	int lenx = strlen(psx);
	if (lenx<12)
	{ 
		printf(" error reading chip information - strlen<12 \n");
		return false; 
	}
	stringstream sss;
	sss << psx;
	char aa;
	sss.ignore(12,'X');
	sss >> posx >> aa >> posy;
	return true;
}

bool ReportChip_Alessi(int &x, int &y, char &letter)  //---new
{
	float posx, posy;
	if(!ReadPosition(posx, posy)) return false;

	nEntry++;
	printf("posX %f   posY %f \n", posx, posy);
	printf("#%03i: %i%i%c -> ", NROC, y, x, letter);
	fflush(stdout);
	Log.section("CHIP", false);
	Log.printf(" %i %i %c %9.1f %9.1f\n",
		x, y, letter, posx, posy);
	g_chipdata.mapX   = x;
	g_chipdata.mapY   = y;
	g_chipdata.mapPos = letter;
	return true;
}


bool TestSingleChip(int &bin, bool &repeat)
{
	int x = crd[NROC].X;
	int y = crd[NROC].Y;
	char letter = crd[NROC].Letter;

	g_chipdata.Invalidate();

	if (!ReportChip_Alessi(x,y,letter)) return false;  //DEBUG - only for Alessi..
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

bool test_dicedwafer()  //---new 
{
	if(crd[0].posX == 0) 
	{
		printf("ERROR: no coordinates stored - please run 'initdiced' before \n");
		return false;
	}

	cout  << "ROCn " << NROC << endl;
	prober.printf("MZ D C");
	tb.mDelay(200);

	while(true)
	{
		int bin = 0;
		bool repeat;
		
		//WARNING - look for 'official' changes in this function
		if (!TestSingleChip(bin,repeat)) break;  //return always true except when chip infos not found

		/*if (keypressed())
		{
			prober.printf("MZ D S");
			printf(" wafer test interrupted!\n");
			printf(" to continue run again 'testdiced'\n");
			break;
		}*/

		if(repeat)
		{
			int nRep = settings.errorRep;
			cout << "nRep = " << nRep << endl;
			for (nRep; nRep > 0; nRep--)  //added 
			{
				printf(" test will be automatically repeated %i time \n", nRep+1);
				printf(" going to separation \n");
				prober.printf("MZ D S");
				tb.mDelay(400);
				printf(" going to contact \n");
				prober.printf("MZ D C");
				tb.mDelay(400);
				if (!TestSingleChip(bin,repeat)) break;
				if(!repeat) break;
			}
		}	

   		// prober step  
		printf(" test result: %i \n", bin);
		prober.printf("MZ D S");
		tb.mDelay(100);
			
		// check if last chip  
		if (NROC < settings.totRocs-1) // ok -> next chip
		{
			printf(" test ended, press Enter to move on next die or 'r' to stay on this die\n");
			char c;		
			cin.get(c);
			if(c=='r') return true;
			else {
  			  NROC ++;
			  std::stringstream sstr1, sstr2;
			  sstr1 << "MP D X " << crd[NROC].posX;
			  sstr2 << " Y " << crd[NROC].posY;
			  string cmdxy = sstr1.str() + sstr2.str();
  		      prober.printf(cmdxy.c_str());

			  tb.mDelay(100);

			  printf(" align this chip and run 'testdiced'\n");
			  return true;
			}
		}
		else // end of wafer -> return
		{
		   prober.printf("MZ D S"); //redundant
		   printf(" End of Wafer Test - Good Job!\n");
		   printf(" press Enter to move to load position or 'r' to stay on this die\n");
			char c;		
			cin.get(c);
   			if(c=='r') return true;
			else {
		      prober.printf("ML D");  // move to load
			  return true;
			}
		}
	}
	prober.printf("MZ D S");
	return true;
}

bool test_wafer() //old!! (not for Alessi)
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

	//printf(" RSP %s\n", prober.printf("BinMapDie %i", bin));

	return true;
}

bool test_chip(char chipid[])
{
	if (settings.proberPort != -2)  //---new. 'if' added (to manual Alessi)
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
		test_dicedwafer(); //--new to Alessi
		//test_wafer(); //old
	}
	else if (settings.proberPort == -2) //---new. 'if' added (to manual Alessi)
	{
		// to add chip number in log file
		char id[42];
		PAR_STRINGEOL(id,40);
									
		g_chipdata.Invalidate();
		nEntry++;
		g_chipdata.nEntry = nEntry;

		int x, y;
		float posx, posy;
		if (sscanf(id, "%i %i %c %f %f", &y, &x, &chipPosChar[chipPos], &posx, &posy) != 5)
		{
		printf("%i %i %c %9.1f %9.1f\n", y, x, chipPosChar[chipPos], posx, posy);
		printf(" error reading chip information\n");
		return;
		}
		printf("#%05i: %i%i%c -> ", nEntry, y, x, chipPosChar[chipPos]);
		fflush(stdout);
		Log.section("CHIP", false); //xy in the log!!!
		Log.printf(" %i %i %c %9.1f %9.1f\n", x, y, chipPosChar[chipPos], posx, posy);
		g_chipdata.mapX   = x;
		g_chipdata.mapY   = y;
		g_chipdata.mapPos = chipPos;
		
		test_chip(id);
	}
	else
	{
		char id[42];
		PAR_STRINGEOL(id,40);
		test_chip(id);
	}
}


//---new offset
#define CSX   8610  //old 8050
#define CSY  10840  //old 10451

const int CHIPOFFSET[4][4][2] =
{	// from -> to  0           1           2           3
	/*   0  */ { {   0,   0},{-CSX,   0},{   0,-CSY},{-CSX,-CSY} },
	/*   1  */ { { CSX,   0},{   0,   0},{ CSX,-CSY},{   0,-CSY} },
	/*   2  */ { {   0, CSY},{-CSX, CSY},{   0,   0},{-CSX,   0} },
	/*   3  */ { { CSX, CSY},{   0, CSY},{ CSX,   0},{   0,   0} },
};

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

		tb.mDelay(100);
		i++;
	} while (goto_def(i));

	prober.printf("MoveChuckSeparation");

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

		/*if (keypressed())
		{
			prober.printf("BinMapDie %i", bin);
			printf(" wafer test interrupted!\n");
			break;
		}*/

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

bool ReadMap(bool offset)  //---new to get wafer coordinates.
{
	offset = 0; //debug - never compute offset

	int k = 0;
	string filename;
	if(offset)	filename = settings.coord_filename + "_offset.dat";
	else	 	filename = settings.coord_filename + ".dat";
	ifstream infile(filename);
	if(!infile)
	{
		printf( "ERROR: no input file %s \n", filename);
		return false;
	}
    while (!infile.eof())
	{
	  infile >> crd[k].N >> crd[k].X >> crd[k].Y >> crd[k].Letter >> crd[k].posX >> crd[k].posY;
	  k++;
    }
    infile.close();  
	printf(" default wafer coordinates read from file %s \n", filename.c_str());
	printf(" %i ROCs found \n", k-1);
	settings.totRocs = k-1;

  return true;
}

bool OffsetMap()  //---new to add offset to wafer coordinates wrt the new home.
{
	//leggere home die
	bool match = false;
	coordinates home;

	string sst;
    sst = settings.homedie[0];
	home.Y = atoi(sst.c_str());
	sst = settings.homedie[1];
	home.X = atoi(sst.c_str());
	home.Letter = settings.homedie[2];

	for(int i=0; i < settings.totRocs; i++)
	{
	 if(crd[i].X == home.X && crd[i].Y==home.Y && crd[i].Letter == home.Letter)
	 {
		 match = true;
		 home.posX = crd[i].posX;
 		 home.posY = crd[i].posY;
  		 home.N = crd[i].N;
		 break;		
	 }
	}
	if(!match) 
	{
		printf("ERROR: no match coordinates for HOME DIE %i %i %c \n", home.X, home.Y, home.Letter);
		return false;
	}

	cout << "ROCn " << home.N << endl;
	float newposx, newposy;
	float oldposx, oldposy;

	oldposx = home.posX;
	oldposy = home.posY;	
	if(!ReadPosition(newposx, newposy)) return false;
	
	//-- offset calculation  -- DEBUG, ok like this if new and old are both negative (HOME = 04A!).
	float offx = newposx - oldposx; 
	float offy = newposy - oldposy;
	
	printf("old %f %f \n", oldposx, oldposy);
	printf("new %f %f \n", newposx, newposy);
	printf("offset %f %f \n", offx, offy);

	//rewriting coord --debug
	for(int i= 0; i < 89; i++)  //89 == home ROC
	{
		crd[i].posX += offx;
		crd[i].posY -= offy;
	}
	for(int i= 89; i < settings.totRocs; i++)
	{
		crd[i].posX += offx;
		crd[i].posY += offy; //debug
	}
		
	//save in new file
	ofstream outfile;
	string filename = settings.coord_filename + "_offset.dat";
    outfile.open(filename.c_str());
	printf( " writing new coordinates on %s \n", filename.c_str());
	int k = 0;
	for (int i = 0; i < settings.totRocs; i++)
	{
		outfile << crd[i].N << "    " << crd[i].X << " " << crd[i].Y << " " << crd[i].Letter << "    " << crd[i].posX << "    " << crd[i].posY << endl;
		k = i;
	}
	printf(" %i ROCs coordinates printed \n", k+1);
	outfile.close();

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

CMD_PROC(goSep)
{
	printf(" RSP %s\n", prober.printf("MZ D S"));
}

CMD_PROC(setSep)
{
	printf(" RSP %s\n", prober.printf("SZ D S"));
}

CMD_PROC(goCont)
{
	printf(" RSP %s\n", prober.printf("MZ D C"));
}

CMD_PROC(setCont)
{
	printf(" RSP %s\n", prober.printf("SZ D C"));
}

CMD_PROC(goH)
{
	printf(" RSP %s\n", prober.printf("MH D"));
}

CMD_PROC(setH)
{
	printf(" RSP %s\n", prober.printf("SH D"));
}

CMD_PROC(goL)
{
	printf(" RSP %s\n", prober.printf("ML D"));
}

CMD_PROC(setL)
{
	printf(" RSP %s\n", prober.printf("SL D"));
}

CMD_PROC(createmap)  //---new to extract coordinates from  previous test
{
	coordinates * c = new coordinates [500];
	coordinates * cOrd = new coordinates [500];
	coordinates * rocOrd = new coordinates [500];
	
  //READ COORDINATES FROM LOG FILE
	string filename;
    filename = settings.oldlog_filename + ".dat";
	cout << filename << endl;
	ifstream infile(filename);
	if(!infile)
	{
		printf( "input file %s is missing \n", filename);
		return;
	}
	printf( "reading coordinates from %s \n", filename);
	string line, chip;
	int nCord = 0;
	while(!infile.eof())
	{
	  infile >> chip;
	    	  cout << infile << endl;
	  if(chip.compare("[CHIP]") == 0)
	  {
		infile >> c[nCord].X >> c[nCord].Y >> c[nCord].Letter >> c[nCord].posX >> c[nCord].posY;
		c[nCord].N = nCord;
		nCord++;
	  }
  	  else getline(infile, line);
    }
	printf("%i ROCs found (double test included) \n", nCord-1),
    infile.close();  

  //READ ROCS SORTED FROM FILE
	string fname;
    fname = settings.rocsorted_filename + ".dat";
	ifstream inf(fname);
	if(!inf)
	{
		printf( "input file %s is missing \n", fname);
		return;
	}
	printf( "reading ROC order from %s \n", fname);
	int nSort = 0;
	while(!inf.eof()) //
	{
      inf >> rocOrd[nSort].N >> rocOrd[nSort].X >> rocOrd[nSort].Y >> rocOrd[nSort].Letter;
	  rocOrd[nSort].posX = rocOrd[nSort].posY = 0.; //variables not needed now
	  nSort++;
    }
    inf.close();
	printf("%i ROCs found \n", nSort-1); //last term null
    if (nCord<nSort-1) 
	{
		printf("ERROR: coordinates less than default roc sorted - please check the input files \n");
		return;
	}

  //SORTED COORDINATES
	int z = 0;
	for(int j= 0; j < nSort-1; j++)
	{
		bool match = false;
		for(int i= 0; i < nCord; i++)
		{
			if(c[i].X==rocOrd[j].X && c[i].Y==rocOrd[j].Y && c[i].Letter==rocOrd[j].Letter) 
			{
					cOrd[z].N = z;
					cOrd[z].X  = rocOrd[j].X ;
					cOrd[z].Y  = rocOrd[j].Y ;
					cOrd[z].Letter  = rocOrd[j].Letter ;
					cOrd[z].posX  = c[i].posX ;
					cOrd[z].posY  = c[i].posY ;
					match = true;
					z++;
					break;
			}
		}
		if(!match)
		{
			printf("WARNING: no matching coordinates for ROC %i %i %c \n", rocOrd[j].X, rocOrd[j].Y, rocOrd[j].Letter);
		}
	}
	
  //WRITE SORTED COORDINATES ON FILE
	FILE *f;
	fname.clear();
	fname = settings.coord_filename + ".dat";
	f = fopen(fname.c_str(),"wt");    

	printf( "writing sorted coordinates on %s \n", fname.c_str());
	for (int i = 0; i < nSort-1; i++)
	{
		fprintf(f,"%-i\t%i %i %c  %8.1f  %8.1f\n", cOrd[i].N, cOrd[i].X, cOrd[i].Y, cOrd[i].Letter, cOrd[i].posX, cOrd[i].posY);
		nCord = i;
	}
	printf("%i ROCs coordinates printed \n", nCord+1);
	fclose(f);
	return;	
}

CMD_PROC(initdiced)  //---new
{
	coordinates cc;
	printf("Welcome!\n");
	printf("Enter 'h' to start from the beginning \n");
	printf("or enter the ROC ID (yxL) do you want to start from. \n");
	
	string line, s;
	std::getline(std::cin,line);
	bool home = false;
	int cnt = 0, i = 0;
	for (std::string::iterator it = line.begin(); it != line.end(); ++it)
	{
		s.assign(line,i,1);
		i++;
		if(s[0]==' ') continue;
		else if (s[0]=='H' || s[0]=='h') {
			cc.X = cc.Y = 0;
			cc.Letter = 'x';
			home = true;
			break;
		}
		else {
			if(cnt == 0) cc.Y = atoi(s.c_str());
   			else if(cnt == 1) cc.X = atoi(s.c_str());
			else if(cnt == 2) cc.Letter = s[0];
			else break;
			cnt++;
		}		
	}
	//changes caps
	if (cc.Letter >= 'a') cc.Letter -= 'a' - 'A'; 
	else if (cc.Letter >= 'b') cc.Letter -= 'b' - 'B';
	else if (cc.Letter >= 'c') cc.Letter -= 'c' - 'C';
	else if (cc.Letter >= 'd') cc.Letter -= 'd' - 'D';
		
	if(home)
	{
		printf("moving to default home position. \n");
		prober.printf("MH D");
		tb.mDelay(100);

		printf(" please, align home die (%s) \n", settings.homedie.c_str());
		printf(" press two times Enter when ok ('s' to stop)");
		char s;
		cin.get(s);
		if(s=='\n')  cin.get(s); //bug ?
		if(s=='s') return;

		//read coordinates from file
		if(!ReadMap(0)) return;		
		//scaling coordinates to the new home
		//if(!OffsetMap()) return;

   	    //save new home
		prober.printf("SH D");
		printf(" new home set \n");		
		printf(" init done -> press Enter to move on first die ('s' to stop)\n");
			char c;		
			cin.get(c);
			if(c=='s') return;

		printf("moving on first die \n");
		NROC = 0;
		std::stringstream sstr1, sstr2;
		sstr1 << "MP D X " << crd[NROC].posX;
		sstr2 << " Y " << crd[NROC].posY;
		//cout <<  "MP D X " << crd[NROC].posX << " Y " << crd[NROC].posY; //debug
		string cmdxy = sstr1.str() + sstr2.str();
		prober.printf(cmdxy.c_str());
		tb.mDelay(100);

		if (!ReportWafer())	return;
		tb.mDelay(200);

		printf(" align this chip and run 'testdiced'\n");		
	}
	else{
		if(!ReadMap(0)) return;	

		//if(!ReadMap(1))
		//{
        //			printf("Please, run initdiced and start from home die to set offset.\n");
		//	return;
		//}
		tb.mDelay(100);
		bool match = false;
		for(int i= 0; i < settings.totRocs; i++)
		{
			if(crd[i].X==cc.X && crd[i].Y==cc.Y && crd[i].Letter==cc.Letter) 
			{
				NROC = crd[i].N;
				match = true;
				break;
			}
		}
		if(!match)
		{
			printf("ERROR: no match coordinates for this ROC %i %i %c \n", cc.X, cc.Y, cc.Letter);
			return;
		}
		printf("moving on ROC %i %i %c \n", crd[NROC].Y, crd[NROC].X, crd[NROC].Letter);
		std::stringstream sstr1, sstr2;
		sstr1 << "MP D X " << crd[NROC].posX;
		sstr2 << " Y " << crd[NROC].posY;
		string cmdxy = sstr1.str() + sstr2.str();
		prober.printf(cmdxy.c_str());
		tb.mDelay(100);
		printf(" align this chip and run 'testdiced'\n");	
	}	
	return;
}

CMD_PROC(testdiced)  //---new 
{
	if(!ReadMap(1)) return;
	if(!test_dicedwafer()) return;

	return;	
}

CMD_PROC(first)
{
	if(crd[0].posX == 0.)	{ if(!ReadMap(1)) return; }
	tb.mDelay(100);
	printf("WARNING: to start wafer test run 'initdiced' \n");
	printf("moving on first die \n");
	NROC = 0;
	std::stringstream sstr1, sstr2;
	sstr1 << "MP D X " << crd[NROC].posX;
	sstr2 << " Y " << crd[NROC].posY;
	string cmdxy = sstr1.str() + sstr2.str();
	prober.printf(cmdxy.c_str());
	tb.mDelay(100);
}


CMD_PROC(next)
{
	if(crd[0].posX == 0.)	{ if(!ReadMap(1)) return; }
	tb.mDelay(100);
	printf("WARNING: to start wafer test run 'initdiced' \n");
	printf("moving on next die \n");
	NROC++;
	std::stringstream sstr1, sstr2;
	sstr1 << "MP D X " << crd[NROC].posX;
	sstr2 << " Y " << crd[NROC].posY;
	string cmdxy = sstr1.str() + sstr2.str();
	prober.printf(cmdxy.c_str());
	tb.mDelay(100);
	printf("NROC = %i \n", NROC);
}


CMD_PROC(goto)
{
	int y, x;
	char l[1];
	PAR_INT(y, -100, 100);
	PAR_INT(x, -100, 100);
	PAR_STRING(l, 3);

	if(crd[0].posX == 0.)	{ if(!ReadMap(1)) return; }
	tb.mDelay(100);
	bool match = false;
	for(int i= 0; i < settings.totRocs; i++)
	{
		if((int)crd[i].X==x && (int)crd[i].Y==y && crd[i].Letter==l[0]) 
		{
			NROC = crd[i].N; //new struct
			match = true;
			break;
		}
	}
	if(!match)
	{
		printf("ERROR: no match coordinates for this ROC %i %i %c\n", y, x, l[0]);
		return;
	}
	printf("moving on ROC %i %i %c \n", crd[NROC].Y, crd[NROC].X, crd[NROC].Letter);
	std::stringstream sstr1, sstr2;
	sstr1 << "MP D X " << crd[NROC].posX;
	sstr2 << " Y " << crd[NROC].posY;
	string cmdxy = sstr1.str() + sstr2.str();
	prober.printf(cmdxy.c_str());

	printf("WARNING: to start wafer test run 'initdiced' \n");
	tb.mDelay(100);
}
