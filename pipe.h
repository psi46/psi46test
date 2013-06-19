// pipe.h

#pragma once
#include "rpc.h"
#include <unistd.h>
#include <WinTypes.h>
#include <sys/uio.h>

class CPipeServer : public CRpcIo
{
	int hPipe;
public:
	CPipeServer() : hPipe(-1) {}
	~CPipeServer() {}
	bool Create(const char *name);
	void WaitConnection();
	void Close();
	void Read(void *buffer, unsigned int size)
	{
		ssize_t  n;
        if ( (n = read(hPipe,buffer,size)) < 0 ) throw int(1);
	}
	void Flush() {}
	void Clear() {}
	void Write(const void *buffer, unsigned int size)
	{
		ssize_t n;
        if ((n = write(hPipe, buffer, size)) < 0) throw int(2);
	}
};


class CPipeClient : public CRpcIo
{
	int hPipe;
public:
	CPipeClient() : hPipe(-1) {}
	~CPipeClient() {}
	bool Open(const char *name);
	void Close();
	void Read(void *buffer, unsigned int size)
	{
		ssize_t n;
        if ( (n = read(hPipe, buffer, size)) < 0 ) throw int(1);
	}
	void Flush() {}
	void Clear() {}
	void Write(const void *buffer, unsigned int size)
	{ 
		ssize_t n;
        if ((n = write(hPipe, buffer, size)) < 0) throw int(2);
	}
};

