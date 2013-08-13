/*
    ethernet packet sockets
*/
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
 

int main (void)
{
    //Create a raw socket
    int s = socket (AF_PACKET, SOCK_RAW, ETH_P_ALL);
     
    if(s == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket");
        exit(1);
    }
     
    char*  interface = (char*)"eth0";
    int protocol = ETH_P_ALL;

    struct ifreq ifr;
    strncpy((char*)ifr.ifr_name, interface, IFNAMSIZ);
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
        perror("problem configuring socket");
    }

 
	char myData[1024] = {
		0x40, 0xd8, 0x55, 0x11, 0x80, 0x08, //destination address
		0x00, 0x90, 0xf5, 0xc3, 0xba, 0x7f, //source address
		0x00, 0x05,                         //data length
		'H','E','L','L','O',                //data
		'\0'                                 //null termination
	  };

    write(s, myData, 1024);

    unsigned char rx_buf[1024];
    while(1){
        read(s, rx_buf,1024);
        int k;

	printf("\nDestination Address: ");
	for(k= 0; k<6; k++)
		printf("%02X:",rx_buf[k]);
	printf("\nSource Address:      ");
	for(k= 0; k<6; k++)
		printf("%02X:",rx_buf[k+6]);

	int length = rx_buf[12];
	length = (length << 8) | rx_buf[13];
	printf("\nLength of message: %d\n",length);

	printf("MESSAGE START--------------\n");
	for(k = 0; k < length; k++){
		printf("%c",rx_buf[14+k]);
	}
	printf("\nEND OF MESSAGE-----------\n");

    }

    return 0;
}
