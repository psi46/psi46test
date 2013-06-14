// rpc.cpp

#include "rpc.h"


void rpcMessage::Create(uint8_t cgrp, uint8_t cmd)
{
	m_type = RPC_TYPE_DTB;
	m_cgrp = cgrp;
	m_cmd = cmd;
	m_size = 0;
	m_pos = 0;
}


void rpcMessage::Send(CRpcIo &rpc_io)
{
	rpc_io.Write(&m_type, 1);
	rpc_io.Write(&m_cgrp, 1);
	rpc_io.Write(&m_cmd,  1);
	rpc_io.Write(&m_size, 1);
	if (m_size) rpc_io.Write(m_par, m_size);
}


void rpcMessage::Receive(CRpcIo &rpc_io)
{
	m_pos = 0;
	rpc_io.Read(&m_type, 1);
	if (m_type != RPC_TYPE_DTB) throw RpcError(RpcError::WRONG_MSG_TYPE);
	rpc_io.Read(&m_cgrp, 1);
	if (m_cgrp > 63)
	{ // skip data
		uint16_t size;
		rpc_io.Read(&size, 2);
		rpc_DataSink(rpc_io, size);
		throw RpcError(RpcError::NO_CMD_MSG);
	}
	rpc_io.Read(&m_cmd, 1);
	rpc_io.Read(&m_size, 1);
	if (m_size) rpc_io.Read(m_par, m_size);
	if (m_cgrp > 31) throw RpcError(RpcError::UNKNOWN_CMD);
	if (m_cgrp == 31) // error message
	{
		if (m_size != 2) throw RpcError(RpcError::UNDEF);
		throw RpcError(RpcError::errorId(Get_UINT16()));
	}
}


void rpcMessage::Check(uint8_t cgrp, uint8_t cmd, uint8_t size)
{
	if (m_type != RPC_TYPE_DTB) throw RpcError(RpcError::WRONG_MSG_TYPE);
	if (m_cgrp != cgrp) throw RpcError(RpcError::WRONG_CGRP);
	if (m_cmd  != cmd)  throw RpcError(RpcError::UNKNOWN_CMD);
	if (m_size != size) throw RpcError(RpcError::CMD_PAR_SIZE);
}


// === data =================================================================


void CDataHeader::RecvHeader(CRpcIo &rpc_io)
{
	rpc_io.Read(&m_type, 1);
	if (m_type != RPC_TYPE_DTB) throw RpcError(RpcError::WRONG_MSG_TYPE);
	rpc_io.Read(&m_cgrp, 1);
	if (m_cgrp < 64)
	{
		uint8_t cmd, size;
		rpc_io.Read(&cmd, 1);
		rpc_io.Read(&size, 1);
		if (m_cgrp == 63)
		{ // error message
			uint16_t err;
			if (m_size != 2) throw RpcError(RpcError::UNDEF);
			rpc_io.Read(&err, 2);
			throw RpcError(RpcError::errorId(err));
		}
		else
		{ // skip data
			rpc_DataSink(rpc_io, size);
			throw RpcError(RpcError::NO_DATA_MSG);
		}
	}
	rpc_io.Read(&m_size, 2);
}



void rpc_SendRaw(CRpcIo &rpc_io, uint8_t channel, const void *x, uint16_t size)
{
	uint8_t value = RPC_TYPE_DTB;
	rpc_io.Write(&value, 1);
	value = 64 + channel;
	rpc_io.Write(&value, 1);
	rpc_io.Write(&size, 2);
	if (size) rpc_io.Write(x, size);
}


void rpc_DataSink(CRpcIo &rpc_io, uint16_t size)
{
	if (size == 0) return;
	CBuffer buffer(size);
	rpc_io.Read(&buffer, size);
}


void rpc_Receive(CRpcIo &rpc_io, string &x)
{
	CDataHeader msg;
	msg.RecvHeader(rpc_io);
	x = "";
	char ch;
	for (uint16_t i=0; i<msg.m_size; i++) { rpc_io.Read(&ch, 1); x += ch; }
}
