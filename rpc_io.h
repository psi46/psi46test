// rpc_io.h

#pragma once

#include "rpc_error.h"


class CRpcIo
{
protected:
	void Dump(const char *msg, const void *buffer, unsigned int size);
public:
	virtual ~CRpcIo() {}
	virtual void Write(const void *buffer, unsigned int size) = 0;
	virtual void Flush() = 0;
	virtual void Clear() = 0;
	virtual void Read(void *buffer, unsigned int size) = 0;
	virtual void Close() = 0;
};


class CRpcIoNull : public CRpcIo
{
public:
	void Write(const void *buffer, unsigned int size) { throw CRpcError(CRpcError::WRITE_ERROR); }
	void Flush() {}
	void Clear() {}
	void Read(void *buffer, unsigned int size) { throw CRpcError(CRpcError::READ_ERROR); }
	void Close() {}

};

