// rpc_io.h

#pragma once

class CRpcIo
{
protected:
	void Dump(const char *msg, const void *buffer, unsigned int size);
public:
	virtual void Write(const void *buffer, unsigned int size) = 0;
	virtual void Flush() = 0;
	virtual void Clear() = 0;
	virtual void Read(void *buffer, unsigned int size) = 0;
	virtual void Close() = 0;
};
