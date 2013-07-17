// rpc_error.h

#pragma once

#include <stdio.h>

class CRpcError
{
public:
	enum errorId
	{
		OK,
		TIMEOUT,
		WRITE_ERROR,
		READ_ERROR,
		READ_TIMEOUT,
		MSG_UNKNOWN,
		WRONG_MSG_TYPE,
		WRONG_DATA_SIZE,
		ATB_MSG,
		WRONG_CGRP,
		WRONG_CMD,
		CMD_PAR_SIZE,
		NO_DATA_MSG,
		NO_CMD_MSG,
		UNKNOWN_CMD,
		UNDEF
	} error;
	CRpcError() : error(CRpcError::OK) {}
	CRpcError(errorId e) : error(e) {}
	const char *GetMsg()
	{
		switch (error)
		{
			case OK:              return "OK";
			case TIMEOUT:         return "TIMEOUT";
			case WRITE_ERROR:     return "WRITE_ERROR";
			case READ_ERROR:      return "READ_ERROR";
			case READ_TIMEOUT:    return "READ_TIMEOUT";
			case MSG_UNKNOWN:     return "MSG_UNKNOWN";
			case WRONG_MSG_TYPE:  return "WRONG_MSG_TYPE";
			case WRONG_DATA_SIZE: return "WRONG_DATA_SIZE";
			case ATB_MSG:         return "ATB_MSG";
			case WRONG_CGRP:      return "WRONG_CGRP";
			case WRONG_CMD:       return "WRONG_CMD";
			case CMD_PAR_SIZE:    return "CMD_PAR_SIZE";
			case NO_DATA_MSG:     return "NO_DATA_MSG";
			case NO_CMD_MSG:      return "NO_CMD_MSG";
			case UNKNOWN_CMD:     return "UNKNOWN_CMD";
			case UNDEF:           return "UNDEF";
		}
		return "?";
	}
	void What()
	{
		printf("ERROR: %s\n", GetMsg());
	}
};
