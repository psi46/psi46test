/*
 * ethernet.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: caleb
 */

#include "ethernet.h"

#include<cstdio> //for printf
#include<cstdlib> //for exit(0);

#include<string.h> //memset
#include<errno.h> //For errno - the error number

#include<sys/socket.h>    //for socket ofcourse
#include<sys/ioctl.h>

#include<unistd.h>
#include<netinet/in.h>
#include<netpacket/packet.h>
#include<net/ethernet.h>
#include<net/if.h>


Ethernet::Ethernet(string interface) {
	is_open = false;
	for(int i =0; i < RX_BUFFER_SIZE; i++){
		rx_buffer[i] = 0;
	}	
	for(int i =0; i < TX_BUFFER_SIZE; i++){
		tx_buffer[i] = 0;
	}
	rx_buffer_size = 0;
	tx_buffer_size = 0;
	
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
}

Ethernet::~Ethernet(){
	close(s);
}

void Ethernet::Write(const void *buffer, unsigned int size){
	for(unsigned int i = 0 ; i < size; i++){
		if(tx_buffer_size == MAX_TX_DATA){
			Flush();
		}
		tx_buffer[TX_DATA_OFFSET + txPayloadSize] = ((char*)buffer)[i];
		tx_buffer_size++;
	}
}
void Ethernet::Flush(){
	write(s,tx_buffer,tx_buffer_size + TX_DATA_OFFSET);
}
void Ethernet::Clear(){

}
void Ethernet::Read(void *buffer, unsigned int size){
	
}

void Ethernet::init_tx_buffer(){
	
	//TODO: find MACs dynamically
	dst_addr[0] = 0xFF;
	dst_addr[1] = 0xFF;
	dst_addr[2] = 0xFF;
	dst_addr[3] = 0xFF;
	dst_addr[4] = 0xFF;
	dst_addr[5] = 0xFF;
	
	src_addr[0] = 0x00 
	src_addr[1] = 0x90 
	src_addr[2] = 0xf5 
	src_addr[3] = 0xc3 
	src_addr[4] = 0xba 
	src_addr[5] = 0x7f 
	for(int i = 0; i < 6; i++){
		tx_buffer[i] = dst_addr[i];
		tx_buffer[i+6] = src_addr[i];
	}
}

