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


char usbId[64];

bool FindDTB()
{
	unsigned int nDev;
	unsigned int nr;
	if (tb.EnumFirst(nDev))
	{
		if (nDev == 0)
		{
			printf("No devices connected.\n");
			return false;
		}

		if (nDev == 1)	// If only 1 connected device -> open it
		{
			if (!tb.EnumNext(usbId))
			{
				printf("Cannot read name of connected device\n");
				return false;
			}
			return true;
		}
		
		// If more than 1 connected device list them
		printf("\nConnected devices:\n");
		for (nr=0; nr<nDev; nr++)
		{
			if (tb.EnumNext(usbId))
			{
				printf("%2u: %s", nr, usbId);
				if (tb.Open(usbId, false))
				{
					try
					{
						unsigned int bid = tb.GetBoardId();
						printf("  BID=%2u\n", bid);
						tb.Close();
					}
					catch (...)
					{
						printf("  Not identifiable\n");
						tb.Close();
					}
				}
				else printf(" - in use\n");	
			}
			else printf("Cannot read name of device\n");
		}
		
		printf("Please choose device (0-%u): ", (nDev-1));
		char choice[8];
		fgets(choice, 8, stdin);
		sscanf (choice, "%d", &nr);
		if (nr >= nDev)
		{
			nr = 0;
			printf("No DTB opened\n");
			return false;
		}
		if (!tb.Enum(usbId, nr))
		{
			printf("Cannot read name of device\n");
			return false;
		}
	} else printf("Cannot access the USB driver\n");
	return true;
}


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

	char *name = FindDTB() ? usbId : settings.port_tb;
	try
	{
		if (tb.Open(name))
		{
			printf("\nBoard %s opened\n", usbId);
			string info;
			try
			{
				tb.GetInfo(info);
				printf("---------------------------------------------\n"
					   "%s"
					   "---------------------------------------------\n", info.c_str());
				tb.Welcome();
				tb.Flush();
			}
			catch(CRpcError e)
			{
				e.What();
				printf("ERROR: DTB software version could not be identified, please update it!\n");
				tb.Close();
				printf("Connection to Board %s has been cancelled\n", name);
			}
		}
		else
		{
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
	catch (CRpcError e)
	{
		e.What();
	}
	return 0;
}
