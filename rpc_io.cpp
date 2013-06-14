// rpc_io.cpp

#include "rpc_io.h"
#include <stdio.h>

void CRpcIo::Dump(const char *msg, const void *buffer, unsigned int size)
{
	printf("%s(", msg);
	for (unsigned int i=0; i<size; i++)
		printf("%02X ", int(((unsigned char *)buffer)[i]));
	printf(")\n");
}
