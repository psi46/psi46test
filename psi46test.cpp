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

#include <string.h>
#include "psi46test.h"


// --- globals ---------------------------------------
int nEntry; // counts the chip tests

CTestboard tb;
CSettings settings;  // global settings
CProtocol Log;


// --- main ------------------------------------------

void help()
{
	printf("psi46test <log file name>\n");
}


char filename[512];


int main(int argc, char* argv[])
{
	printf(VERSIONINFO "\n");

	if (argc != 2) { help(); return 1; }
	strncpy(filename, argv[1], 508);

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

	Log.section("DTB", false);
	try
	{
		if (tb.Open(settings.port_tb))
		{
			unsigned char bid;
			string ver;
			tb.GetVersionString(ver);
			bid = tb.GetBoardId();
			printf("DTB %i: %s\n", (int)bid, ver.c_str());
			Log.printf("BID=%i %s", (int)bid, ver.c_str());
			if (!tb.CheckCompatibility())
				printf("WARNING: DTB not compatible this software version!\n");
			tb.Welcome();
//			tb.I2cAddr(0);
			tb.Flush();
		} else {
			printf("USB error: %s\n", tb.ConnectionError());
			printf("ATB: could not open port to device %s\n", settings.port_tb);
			printf("Connect testboard and try command 'scan' to find connected devices.\n");
			printf("Make sure you have permission to access USB devices.\n");
		}
		Log.puts("\n");

		Log.flush();

		// --- call command interpreter -----------------------
		nEntry = 0;
		cmd();
	}
	catch (RpcError e)
	{
		e.What();
	}
	return 0;
}
