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
#define VERSION      "V7.2"
#define TIMESTAMP    "24.08.2010"
// ---------------------------------------------------

#define VERSIONINFO TITLE " " VERSION " (" TIMESTAMP ")"


#include "pixel_dtb.h"
#include "settings.h"
#include "protocol.h"
#include "test.h"


// global variables
extern int nEntry; // counts the entries in the log file

extern CTestboard tb;
extern CSettings settings;  // global settings
extern CProtocol Log;  // log file

void cmd();


#endif
