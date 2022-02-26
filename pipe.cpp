// pipe.cpp

#include "pipe.h"


// === pipe server ==========================================================

bool CPipeServer::Create(const char *name)
{
	if (hPipe != INVALID_HANDLE_VALUE) return false;
	hPipe = CreateNamedPipe(name, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 2, 512, 512, 10000 /* ms */, NULL);
	return hPipe != INVALID_HANDLE_VALUE;
}


void CPipeServer::WaitConnection()
{
	if (!ConnectNamedPipe(hPipe, NULL)) throw int(3);
}


void CPipeServer::Close()
{
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}


// === pipe client ==========================================================


bool CPipeClient::Open(const char *name)
{
	if (hPipe != INVALID_HANDLE_VALUE) return false;
	hPipe = CreateFile(name, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	return hPipe != INVALID_HANDLE_VALUE;
}


void CPipeClient::Close()
{
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

