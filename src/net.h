/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#ifndef _net_h_
#define _net_h_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.h"

#pragma pack(push, 1)
typedef struct ADDR
{
	uint ip;
	ushort port;
	bool type;
} Address;
#pragma pack(pop)

char *myinet_ntoa(uint); // int ip to string

unsigned int myinet_aton(char*); // string ip to int

int recvall(int, uchar*, int); // recieve precise number of bytes 

Address get_host(char*); // return address converted from string type 'xxx.xxx.xxx.xxx:port' to Address structure

/* create/connect to server */
int create_server(Address);

int connection(Address);
#endif
