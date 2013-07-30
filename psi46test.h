/* -------------------------------------------------------------
 *
 *  file:        psi46test.h
 *
 *  description: globals for PSI46V2 Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    31.1.2007
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#ifndef PSI46TEST_H
#define PSI46TEST_H


// ---------------------------------------------------
#define TITLE        "PSI46V2 Wafer Tester"
#define VERSION      "V1.0"
#define TIMESTAMP    "26.07.2013"
// ---------------------------------------------------

#define VERSIONINFO TITLE " " VERSION " (" TIMESTAMP ")"


#include "pixel_dtb.h"
#include "settings.h"
#include "prober.h"
#include "protocol.h"
#include "pixelmap.h"
#include "test.h"
#include "chipdatabase.h"


// global variables
extern int nEntry; // counts the entries in the log file

extern CTestboard tb;
extern CSettings settings;  // global settings
extern CProber prober; // prober
extern CProtocol Log;  // log file

extern CChip g_chipdata;

extern const int delayAdjust;
extern const int deserAdjust;


void cmd();


#endif
