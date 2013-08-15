/*
 * ethernet.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: caleb
 */

#include "ethernet.h"

#include<cstdio> //for printf

#include<errno.h> //For errno - the error number
#include<string.h>

#include<sys/socket.h>
#include<sys/ioctl.h>

#include<unistd.h>
#include<netinet/in.h>
#include<netpacket/packet.h>
#include<net/ethernet.h>
#include<net/if.h>


Ethernet::Ethernet(){
	std::string eth_if("eth0");
	init_connection(eth_if);
}
Ethernet::Ethernet(std::string interface) {
	init_connection(interface);
}

Ethernet::~Ethernet(){}

void Ethernet::Write(const void *buffer, unsigned int size){
	for(unsigned int i = 0 ; i < size; i++){
		if(tx_payload_size == MAX_TX_DATA){
			Flush();
		}
		tx_frame[ETH_DATA_OFFSET + tx_payload_size] = ((char*)buffer)[i];
		tx_payload_size++;
	}
}
void Ethernet::Flush(){
	tx_frame[12] = tx_payload_size >> 8;
	tx_frame[13] = tx_payload_size;
	write(s,tx_frame,tx_payload_size + ETH_DATA_OFFSET);
	tx_payload_size = 0;
}
void Ethernet::Clear(){
	tx_payload_size = 0;
	rx_buffer.clear();
}
void Ethernet::Read(void *buffer, unsigned int size){
	for(unsigned int i = 0; i < size; i++){
		if(!rx_buffer.empty()){
			((unsigned char*)buffer)[i] = rx_buffer.front();
			rx_buffer.pop_front();
		} else{
				size_t bytesRead = read(s,rx_frame,RX_FRAME_SIZE);
				if(!bytesRead) printf("ZERO BYTES READ!!!");
				if(bytesRead < 0){
					perror("Error reading from ethernet");
					throw CRpcError(CRpcError::READ_ERROR);
				}
				unsigned int rx_payload_size = rx_frame[12];
				rx_payload_size = (rx_payload_size << 8) | rx_frame[13];
				for(int j =0; j < rx_payload_size;j++){
					rx_buffer.push_back(rx_frame[j + ETH_DATA_OFFSET]);
				}
				i--;
		}
	}
}


void Ethernet::Close(){
	close(s);
}

void Ethernet::init_connection(std::string interface){
	rx_buffer.resize(0);
	is_open = false;
	for(int i =0; i < RX_FRAME_SIZE; i++){
		rx_frame[i] = 0;
	}	
	for(int i =0; i < TX_FRAME_SIZE; i++){
		tx_frame[i] = 0;
	}
	tx_payload_size = 0;
	
	//Create a raw socket
    s = socket (AF_PACKET, SOCK_RAW, ETH_P_ALL);
     
    if(s == -1)
    {
        perror("Failed to create socket"); return;
    }
    int protocol = ETH_P_ALL;

    struct ifreq ifr;
    strncpy((char*)ifr.ifr_name, interface.c_str(), IFNAMSIZ);
    ioctl(s,SIOCGIFINDEX, &ifr);

    struct sockaddr_ll sll;
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);
    bind(s,(struct sockaddr*) &sll,sizeof(sll));

    struct packet_mreq mr;
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ifr.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if(setsockopt(s,SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0){
        perror("problem configuring socket"); return;
    }
    is_open = true;
    
    init_tx_frame();
}

void Ethernet::init_tx_frame(){
	
	//TODO: find MACs dynamically
	dst_addr[0] = 0xFF;
	dst_addr[1] = 0xFF;
	dst_addr[2] = 0xFF;
	dst_addr[3] = 0xFF;
	dst_addr[4] = 0xFF;
	dst_addr[5] = 0xFF;
	
	src_addr[0] = 0x00;
	src_addr[1] = 0x90;
	src_addr[2] = 0xf5;
	src_addr[3] = 0xc3;
	src_addr[4] = 0xba;
	src_addr[5] = 0x7f;
	for(int i = 0; i < 6; i++){
		tx_frame[i] = dst_addr[i];
		tx_frame[i+6] = src_addr[i];
	}
}

