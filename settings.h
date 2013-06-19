// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>


#define NUMSETTING 3

class CSettings
{
	FILE *f;
	bool read_int(int &value, int min, int max);
	bool read_string(char string[], int size);
public:
	bool read(const char filename[]);

// --- data --------------------------------------------------------------
	char port_tb[20];   // default USB serial number testboard
	char path[256];     // command path
	int  port_prober;	// prober serial port nr (-1 = no prober)
//	int  tct_wbc;		// tct - wbc offset
	int  sensor; 		// sensor mounted
	int  clock;         // clock frequency im MHz
	int  errorRep;      // # test rep if defect chip
	int  l1_bl_shift;   // level 1 - black level correction

	int vcomp;
	int vhlddel;
	int vthr;
	int caldel;
	int vcal;
};


#endif
