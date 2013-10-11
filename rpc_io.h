// rpc_io.h

#pragma once

#include "rpc_error.h"

class CRpcIo
{
protected:
	void Dump(const char *msg, const void *buffer, unsigned int size);
public:
	virtual ~CRpcIo() {}
	virtual const char* Name() = 0;
	// IO methods
	virtual void Write(const void *buffer, unsigned int size) = 0;
	virtual void Flush() = 0;
	virtual void Clear() = 0;
	virtual void Read(void *buffer, unsigned int size) = 0;
	virtual void Close() = 0;
	// Error processing
	virtual int GetLastError() = 0;
	virtual const char* GetErrorMsg(int error) = 0;
	// Connection establishment
	virtual bool EnumFirst(unsigned int &nDevices) = 0;
	virtual bool EnumNext(char name[]) = 0;
	virtual bool Enum(char name[], unsigned int pos) = 0;
	virtual bool Open(char serialNumber[]) = 0;
	virtual bool Connected() = 0;
};


class CRpcIoNull : public CRpcIo
{
public:
	const char* Name() {return "NULL"; }
	// IO methods
	void Write(const void *buffer, unsigned int size) { throw CRpcError(CRpcError::WRITE_ERROR); }
	void Flush() {}
	void Clear() {}
	void Read(void *buffer, unsigned int size) { throw CRpcError(CRpcError::READ_ERROR); }
	void Close() {}
	// Error processing
	int GetLastError(){return 0;}
	const char* GetErrorMsg(int error){return NULL;}
	// Connection establishment
	bool EnumFirst(unsigned int &nDevices){ return false;}
	bool EnumNext(char name[]){return false;}
	bool Enum(char name[], unsigned int pos){return false;}
	bool Open(char serialNumber[]){return false;}
	bool Connected(){return false;}
};

