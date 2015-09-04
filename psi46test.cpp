/* -------------------------------------------------------------
 *
 *  file:        psi46test.cpp
 *
 *  description: main program for PSI46V2 Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    6.2.2006
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#include <string>
#include <vector>
#include "psi46test.h"

#include "profiler.h"

using namespace std;


// --- globals ---------------------------------------
int nEntry; // counts the chip tests

CTestboard tb;
CSettings settings;  // global settings
CProber prober;
CProtocol Log;

CWaferList waferList;


void help()
{
	printf("psi46test <log file name>\n");
}


string filename;


int main(int argc, char* argv[])
{
	string usbId;
	printf(VERSIONINFO "\n");

	if (argc != 2) { help(); return 1; }
		
	filename = argv[1];

	// --- load settings ----------------------------------
	if (!settings.Read("psi46test.ini"))
	{
		printf("error reading \"psi46test.ini\"\n");
		return 2;
	}

	// --- open log file ----------------------------------
/*	FILE *f = fopen(filename,"rb");
	if (f)
	{
		printf("Log file \"%s\" exist!\n", filename);
		fclose(f);
		return 1;
	} */


	if (settings.IsWaferList())
	{ // wafer test mode with wafer list
		if (!waferList.Read(settings.waferList))
		{
			printf("No waferlist \"%s\" found!\n", settings.waferList.c_str());
			return 5;
		}

		switch (waferList.SelectWafer(filename))
		{
		case 1: printf("Wafer %s not in wafer list!\n", filename.c_str());
				return 5;
		case 2: printf("Wafer %s already tested!\n", filename.c_str());
				return 5;
		}

		filename = filename + ".log";
	}

	if (!Log.open(filename.c_str()))
	{	
		printf("log: error creating file\n");
		return 3;
	}

	// --- open test board --------------------------------
	Log.section("DTB");

	try
	{
		//--new to allow 'dummy' mode
		if(settings.dtbId == -2)
		{
			printf("\n warning, no DTB connected (dtb_id = -2)! \n");			
			Log.puts(" debug mode - no DTB connected (dtb_id = -2). \n");
		}
		//------------
		else if (!tb.FindDTB(usbId)) {}
		else if (tb.Open(usbId))
		{	
			printf("\nDTB %s opened\n", usbId.c_str());
			string info;
			try
			{
				tb.GetInfo(info);
				printf("--- DTB info-------------------------------------\n"
					   "%s"
					   "-------------------------------------------------\n", info.c_str());
				Log.puts(info.c_str());
				tb.Welcome();
				tb.Flush();
			}
			catch(CRpcError &e)
			{
				e.What();
				printf("ERROR: DTB software version could not be identified, please update it!\n");
				tb.Close();
				printf("Connection to Board %s has been cancelled\n", usbId.c_str());
			}
		}
		else
		{
			printf("USB error: %s\n", tb.ConnectionError());
			printf("Connect testboard and try command 'scan' to find connected devices.\n");
			printf("Make sure you have permission to access USB devices.\n");
		}
		
		// --- open prober ------------------------------------
		if (settings.proberPort>=0)
			if (!prober.open(settings.proberPort))
			{
				printf("Prober: could not open port %i\n",
					settings.proberPort);
				Log.puts("Prober: could not open port\n");
				return 4;
			}

		Log.flush();

		// --- call command interpreter -----------------------
		nEntry = 0;

		cmd();
		tb.Close();
	}
	catch (CRpcError &e)
	{
		e.What();
	}

	if (settings.IsWaferList()) waferList.Write(settings.waferList);

	return 0;
}
