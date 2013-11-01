// psi46_tb.cpp

#include "pixel_dtb.h"
#include <stdio.h>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#endif

void print_device_name(int n, const std::string &name){
	
	if(name.compare(4,3,"ETH") == 0){
		printf("%2u: DTB_ETH ", n);
		for(unsigned int i = 7; i < (7+6);i++){
			printf("%02X ", (unsigned char)name[i]);
		}
	} else{
		printf("%2u: %s", n, name.c_str());
	}
	printf("\n");
}


bool CTestboard::EnumFirst(unsigned int &nDevices, CRpcIo* io) 
{
	return io->EnumFirst(nDevices); 
}

bool CTestboard::EnumNext(string &name, CRpcIo* io)
{
	char s[64];
	if (!io->EnumNext(s)) return false;
	name = s;
	return true;
}

bool CTestboard::Enum(unsigned int pos, string &name, CRpcIo* io)
{
	char s[64];
	if (!io->Enum(s, pos)) return false;
	name = s;
	return true;
}

bool CTestboard::FindDTB(string &rpcId)
{	
	vector<CRpcIo*> ifList;
	if(ethernet == NULL){
		try{
			ethernet = new CEthernet();
			ifList.push_back(ethernet);
			printf("opened ethernet\n");
		} catch(CRpcError e){
			printf("Error initiating ethernet. Please ensure \
			       proper permissions are granted.\n");
		}
	} else{ifList.push_back(ethernet);}
	
	if(usb == NULL){
		try{
			usb = new CUSB();
			ifList.push_back(usb);
			printf("opened usb\n");
		} catch(CRpcError e){
			printf("Error initiating usb. Please ensure \
			       proper permissions are granted.\n");
		}
	} else{ifList.push_back(usb);}
	
	if(ifList.size() == 0){
		printf("Could not open any interfaces.\n");
		return false;
	}
	
	string name;	
	vector<string> devList;
	unsigned int nDev;
	unsigned int nr;
	
	
	for(unsigned int i = 0; i < ifList.size(); i++){
		CRpcIo *interface = ifList[i];
		try{
			if (!EnumFirst(nDev,interface)) continue;
			printf("Found %d devices on interface %s \n", nDev, interface->Name());
			for (nr=0; nr<nDev; nr++)
			{
				if (!EnumNext(name,interface)) continue;
				if (name.size() < 4) continue;
				if (name.compare(0, 4, "DTB_") == 0) devList.push_back(name);
			}
		}catch(CRpcError e){
				printf("Error querying on interface %s", interface->Name());
		}
	}
	
	if (devList.size() == 0)
	{
		printf("No DTB connected.\n");
		return false;
	}
	
	if (devList.size() == 1)
	{
		rpcId = devList[0];
		if(devList[0].compare(4,3,"ETH") == 0){
			rpc_io = ethernet;
		} else{
			rpc_io = usb;
		}
		return true;
	}

	// If more than 1 connected device list them
	printf("\nConnected DTBs:\n");
	for (nr=0; nr<devList.size(); nr++)
	{
		print_device_name(nr+1, devList[nr]);
		CRpcIo* interface = (devList[nr].compare(4,3,"ETH") ? (CRpcIo*)usb : (CRpcIo*)ethernet);
		if (Open(devList[nr], false, interface))
		{
			try
			{
				unsigned int bid = GetBoardId();
				printf("  BID=%2u\n", bid);
			}
			catch (...)
			{
				printf("  Not identifiable\n");
			}
			Close(interface);
		}
		else printf(" - in use\n");
	}

	printf("Please choose DTB (1-%u): ", (unsigned int)devList.size());
	char choice[8];
	fgets(choice, 8, stdin);
	sscanf (choice, "%d", &nr);
	if (nr > devList.size() || nr <= 0)
	{
		nr = 0;
		printf("No DTB opened\n");
		return false;
	}

	rpcId = devList[nr-1];
	rpc_io = (rpcId.compare(4,3,"ETH") ? (CRpcIo*)usb : (CRpcIo*)ethernet);
	return true;
}

bool CTestboard::Open(string &rpcId, bool init, CRpcIo* io)
{
	rpc_Clear();
	if (!io->Open(&(rpcId[0]))) return false;

	if (init) Init();
	return true;
}


bool CTestboard::Open(string &rpcId, bool init)
{
	return Open(rpcId, init, rpc_io);
}

void CTestboard::Close(CRpcIo* io)
{
	rpc_io->Close();
	rpc_Clear();
}

void CTestboard::Close()
{
	Close(rpc_io);
}


void CTestboard::mDelay(uint16_t ms)
{
	Flush();
#ifdef _WIN32
	Sleep(ms);			// Windows
#else
	usleep(ms*1000);	// Linux
#endif
}
