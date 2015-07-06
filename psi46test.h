/* -------------------------------------------------------------
 *
 *  file:        psi46test.h
 *
 *  description: globals for PSI46V2 Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    7.8.2013
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#ifndef PSI46TEST_H
#define PSI46TEST_H

#include "config.h"
#include "pixel_dtb.h"
#include "settings.h"
#include "prober.h"
#include "waferlist.h"
#include "protocol.h"
#include "pixelmap.h"
#include "test.h"
#include "chipdatabase.h"

#define VERSIONINFO TITLE " " VERSION " (" TIMESTAMP ")"


// global variables
extern int nEntry; // counts the entries in the log file

extern CTestboard tb;
extern CSettings settings;  // global settings
extern CProber prober; // prober
extern CProtocol Log;  // log file
extern CWaferList waferList; // wafer list for wafer test option

extern CChip g_chipdata;

void cmd();


#endif
