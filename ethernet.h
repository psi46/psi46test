// ethernet.h
//
// Author: Caleb Fangmeier
//
// Class provides basic functionalities to use the Ethernet interface
//

#ifndef ETHERNET_H
#define ETHERNET_H

#include<deque>
#include<string>
#include "rpc_io.h"


#define RX_FRAME_SIZE 2048
#define TX_FRAME_SIZE 2048

#define MAX_TX_DATA 1500
#define ETH_DATA_OFFSET 14

class Ethernet : public CRpcIo
{
	void init_tx_frame();
	void init_connection(std::string eth_if);

	int s;
	unsigned char      rx_frame[RX_FRAME_SIZE];
	std::deque<unsigned char>    rx_buffer;
	unsigned char      tx_frame[TX_FRAME_SIZE];
	unsigned char      dst_addr[6];
	unsigned char      src_addr[6];
	
	unsigned int       tx_payload_size;//data in tx buffer minus header(14 bytes)
	bool                is_open;
public:
	Ethernet();
	Ethernet(std::string interface);
	~Ethernet();

	void Write(const void *buffer, unsigned int size);
	void Flush();
	void Clear();
	void Read(void *buffer, unsigned int size);
	void Close();
	bool IsOpen();
};

#endif
