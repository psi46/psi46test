// error.h

#ifndef ERROR_H
#define ERROR_H

#define ERROR_ABORT(nr) { errnr = (nr); return false; }

extern int errnr;

enum
{
	ERROR_OK = 0,
	ERROR_OPEN,
	ERROR_NO_LOGFILE,
	ERROR_WAFERID,
	ERROR_CHIP,
	ERROR_BEGIN,
	ERROR_FREQ,
	ERROR_PON,
	ERROR_TOKEN,
	ERROR_VANA,
	ERROR_ITRIM,
	ERROR_PROBECARD,
	ERROR_AOUT,
	ERROR_PIXEL,
	ERROR_PH,
	ERROR_PH1,
	ERROR_PH2,
	ERROR_DCOL,
	ERROR_PUC,
	ERROR_CLASS,
	ERROR_POFF,
	ERROR_END,
	ERROR_CLOSE
};

const char* errormsg();
bool isError();


#endif
