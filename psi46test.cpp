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


string usbId;

bool FindDTB()
{
	string name;
	vector<string> devList;
	unsigned int nDev;
	unsigned int nr;

	try
	{
		if (!tb.EnumFirst(nDev)) throw int(1);
		for (nr=0; nr<nDev; nr++)
		{
			if (!tb.EnumNext(name)) throw int(2);
			if (name.size() < 3) continue;
			if (name.compare(0, 3, "DTB") == 0)	devList.push_back(name);
		}
	}
	catch (int e)
	{
		switch (e)
		{
		case 1: printf("Cannot access the USB driver\n"); return false;
		case 2: printf("Cannot read name of connected device\n"); return false;
		default: return false;
		}
	}

	if (devList.size() == 0)
	{
		printf("No devices connected.\n");
		return false;
	}

	if (devList.size() == 1)
	{
		usbId = devList[0];
		return true;
	}

	// If more than 1 connected device list them
	printf("\nConnected devices:\n");
	for (nr=0; nr<devList.size(); nr++)
	{
		printf("%2u: %s", nr, devList[nr].c_str());
		if (tb.Open(&(devList[nr][0]), false))
		{
			try
			{
				unsigned int bid = tb.GetBoardId();
				printf("  BID=%2u\n", bid);
			}
			catch (...)
			{
				printf("  Not identifiable\n");
			}
			tb.Close();
		}
		else printf(" - in use\n");
	}

	printf("Please choose device (0-%u): ", (nDev-1));
	char choice[8];
	fgets(choice, 8, stdin);
	sscanf (choice, "%d", &nr);
	if (nr >= devList.size())
	{
		nr = 0;
		printf("No DTB opened\n");
		return false;
	}

	usbId = devList[nr];
	return true;
}


// --- main ------------------------------------------

void help()
{
	printf("psi46test <log file name>\n");
}


char filename[256];


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
	Log.section("DTB");

	if (!FindDTB()) usbId = settings.port_tb;
	try
	{
		if (tb.Open(&(usbId[0])))
		{
			printf("\nBoard %s opened\n", usbId.c_str());
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
