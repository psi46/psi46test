// error.cpp

#include "error.h"

int errnr = 0;

const char* errormsg()
{
	switch (errnr)
	{
	case ERROR_OK:
		return ""; break;
	case ERROR_OPEN:
		return "ERROR: Could not open log file!";
	case ERROR_NO_LOGFILE:
		return "ERROR: incorrent log file!";
	case ERROR_WAFERID:
		return "ERROR: incorrent or missing wafer id!";
	case ERROR_CHIP:
		return "ERROR: incorrect or missing CHIP section!";
	case ERROR_BEGIN:
		return "ERROR: incorrect or missing BEGIN section!";
	case ERROR_FREQ:
		return "ERROR: incorrect or missing FREQ section!";
	case ERROR_PON:
		return "ERROR: incorrect or missing PON section!";
	case ERROR_TOKEN:
		return "ERROR: incorrect TOKEN section!";
	case ERROR_VANA:
		return "ERROR: incorrect VANA section!";
	case ERROR_ITRIM:
		return "ERROR: incorrect ITRIM section!";
	case ERROR_PROBECARD:
		return "ERROR: incorrect or missing probcard measurement sections!";
	case ERROR_AOUT:
		return "ERROR: incorrect AOUT section!";
	case ERROR_PIXEL:
		return "ERROR: incorrect PIXMAP section!";
	case ERROR_PH:
		return "ERROR: incorrect PULSE section!";
	case ERROR_PH1:
		return "ERROR: incorrect PH1 section!";
	case ERROR_PH2:
		return "ERROR: incorrect PH2 section!";
	case ERROR_DCOL:
		return "ERROR: incorrect DCOL section!";
	case ERROR_PUC:
		return "ERROR: incorrect PUC section!";
	case ERROR_CLASS:
		return "ERROR: incorrect CLASS section!";
	case ERROR_POFF:
		return "ERROR: incorrect or missing POFF section!";
	case ERROR_END:
		return "ERROR: incorrect or missing END section!";
	case ERROR_CLOSE:
		return "ERROR: missing CLOSE section!";
	}
	return "";
}


bool isError() { return errnr != ERROR_OK; }
