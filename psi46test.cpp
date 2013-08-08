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



void help()
{
	printf("psi46test <log file name>\n");
}


char filename[256];


int main(int argc, char* argv[])
{
	string usbId;
	printf(VERSIONINFO "\n");

	if (argc != 2) { help(); return 1; }
	strncpy(filename, argv[1], sizeof(filename));

	// --- load settings ----------------------------------
	if (!settings.read("psi46test.ini"))
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
	if (!Log.open(filename))
	{
		printf("log: error creating file\n");
		return 3;
	}

	// --- open test board --------------------------------
	Log.section("DTB");

	try
	{
		if (!tb.FindDTB(usbId)) {}
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
			catch(CRpcError e)
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
			printf("ATB: could not open port to device %s\n", settings.port_tb);
			printf("Connect testboard and try command 'scan' to find connected devices.\n");
			printf("Make sure you have permission to access USB devices.\n");
		}

		// --- open prober ------------------------------------
		if (settings.port_prober>=0)
			if (!prober.open(settings.port_prober))
			{
				printf("Prober: could not open port %i\n",
					settings.port_prober);
				Log.puts("Prober: could not open port\n");
				return 4;
			}

		Log.flush();

		// --- call command interpreter -----------------------
		nEntry = 0;

		cmd();
	}
	catch (CRpcError e)
	{
		e.What();
	}

	return 0;
}
