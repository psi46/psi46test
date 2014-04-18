// cmd.h

#ifndef CMD_H
#define CMD_H


#include <math.h>
#include <time.h>
#include <fstream>
#include "psi46test.h"
#include "plot.h"
#include "datastream.h"

#include "command.h"
#include "defectlist.h"
#include "rpc.h"


#define DO_FLUSH  if (par.isInteractive()) tb.Flush();


#endif // CMD_H
