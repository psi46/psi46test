// rpc.h

#pragma once

#include <vector>
#include <string>

#include "rpc_io.h"
#include "rpc_error.h"

using namespace std;


#define RPC_TYPE_ATB 0x8F
#define RPC_TYPE_DTB 0xC0

#define DTB_EXPORT(x)

extern const uint64_t rpc_MainVersion, rpc_UserVersion;
extern const char rpc_timestamp[];


#define CMDGRP_LIST_SIZE 32

class rpcMessage;
void CmdGrpUnknown(CRpcIo &rpc_io, rpcMessage &msg);
void CmdUnknown(CRpcIo &rpc_io, rpcMessage &msg);


class CBuffer
{
	uint8_t *p;
public:
	CBuffer(uint16_t size) { p = new uint8_t[size]; }
	~CBuffer() { delete[] p; }
	uint8_t* operator&() { return p; }
};


// === message ==============================================================

class rpcMessage
{
	uint8_t m_type;
	uint8_t m_cgrp;
	uint8_t m_cmd;
	uint8_t m_size;

	uint8_t m_pos;
	uint8_t m_par[256];
public:
	uint8_t GetCgrp() { return m_cgrp; }
	uint8_t GetCheckedCmd(uint8_t cmdCnt)
	{ if (m_cmd < cmdCnt) return m_cmd; throw RpcError(RpcError::UNKNOWN_CMD); }

	void Create(uint8_t cgrp, uint8_t cmd);
	void Put_INT8(int8_t x) { m_par[m_pos++] = int8_t(x); m_size++; }
	void Put_UINT8(uint8_t x) { m_par[m_pos++] = x; m_size++; }
	void Put_BOOL(bool x) { Put_UINT8(x ? 1 : 0); }
	void Put_INT16(int16_t x) { Put_UINT8(uint8_t(x>>8)); Put_UINT8(uint8_t(x)); }
	void Put_UINT16(uint16_t x) { Put_UINT8(uint8_t(x>>8)); Put_UINT8(uint8_t(x)); }
	void Put_INT32(int32_t x) { Put_UINT16(uint16_t(x>>16)); Put_UINT16(uint16_t(x)); }
	void Put_UINT32(int32_t x) { Put_UINT16(uint16_t(x>>16)); Put_UINT16(uint16_t(x)); }
	void Put_INT64(int64_t x) { Put_UINT32(uint32_t(x>>32)); Put_UINT32(uint32_t(x)); }
	void Put_UINT64(uint64_t x) { Put_UINT32(uint32_t(x>>32)); Put_UINT32(uint32_t(x)); }
	void Send(CRpcIo &rpc_io);

	void Receive(CRpcIo &rpc_io);
	void Check(uint8_t cgrp, uint8_t cmd, uint8_t size);
	void CheckSize(uint8_t size) { if (m_size != size) throw RpcError(RpcError::CMD_PAR_SIZE); }

	int8_t Get_INT8() { return int8_t(m_par[m_pos++]); }
	uint8_t Get_UINT8() { return uint8_t(m_par[m_pos++]); }
	bool Get_BOOL() { return Get_UINT8() != 0; }
	int16_t Get_SHORT() { int16_t x = Get_UINT8(); x = (x << 8) + Get_UINT8(); return x; }
	uint16_t Get_UINT16() { uint16_t x = Get_UINT8(); x = (x << 8) + Get_UINT8(); return x; }
	int32_t Get_INT32() { int32_t x = Get_UINT16(); x = (x << 16) + Get_UINT16(); return x; }
	uint32_t Get_UINT32() { uint32_t x = Get_UINT16(); x = (x << 16) + Get_UINT16(); return x; }
 	int64_t Get_INT64() { int64_t x = Get_UINT32(); x = (x << 32) + Get_UINT32(); return x; }
	uint64_t Get_UINT64() { uint64_t x = Get_UINT32(); x = (x << 32) + Get_UINT32(); return x; }
};


// === data =================================================================

#define vectorR vector
#define stringR string


class CDataHeader
{
public:
	uint8_t m_type;
	uint8_t m_cgrp;
	uint16_t m_size;

	void RecvHeader(CRpcIo &rpc_io);
	void RecvRaw(CRpcIo &rpc_io, void *x)
	{ if (m_size) rpc_io.Read(x, m_size); }
};

void rpc_SendRaw(CRpcIo &rpc_io, uint8_t channel, const void *x, uint16_t size);

void rpc_DataSink(CRpcIo &rpc_io, uint16_t size);


template <class T>
inline void rpc_Send(CRpcIo &rpc_io, const vector<T> &x)
{
	rpc_SendRaw(rpc_io, 0, &(x[0]), sizeof(T)*x.size());
}


template <class T>
void rpc_Receive(CRpcIo &rpc_io, vector<T> &x)
{
	CDataHeader msg;
	msg.RecvHeader(rpc_io);
	if ((msg.m_size % sizeof(T)) != 0)
	{
		rpc_DataSink(rpc_io, msg.m_size);
		throw RpcError(RpcError::WRONG_DATA_SIZE);
	}
	x.assign(msg.m_size/sizeof(T), 0);
	rpc_io.Read(&(x[0]), msg.m_size);
}


inline void rpc_Send(CRpcIo &rpc_io, const string &x)
{
	rpc_SendRaw(rpc_io, 0, x.c_str(), x.length());
}


void rpc_Receive(CRpcIo &rpc_io, string &x);
