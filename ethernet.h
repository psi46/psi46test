// ethernet.h
//
// Author: Caleb Fangmeier
//
// Class provides basic functionalities to use the Ethernet interface
//

#ifndef ETHERNET_H
#define ETHERNET_H

#include "rpc_io.h"

#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 2048

#define MAX_TX_DATA 1500
#define TX_DATA_OFFSET 14

class Ethernet : public CRpcIo
{
public:
	Ethernet();
	virtual ~Ethernet();

	void Write(const void *buffer, unsigned int size);
	void Flush();
	void Clear();
	void Read(void *buffer, unsigned int size);
	bool IsOpen();
	
private:
	void init_tx_buffer();

	int s;
	unsigned char rx_buffer[RX_BUFFER_SIZE];
	unsigned char tx_buffer[TX_BUFFER_SIZE];
	unsigned char dst_addr[6];
	unsigned char src_addr[6];
	
	unsigned int tx_buffer_size;//data in tx buffer minus header(14 bytes)
	unsigned int rx_buffer_size;//data in rx buffer
	bool is_open;
};

#endif
