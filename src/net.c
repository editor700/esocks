/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#include "net.h"

// int ip to string
char *myinet_ntoa(uint ip)
{
	char *res = (char*)malloc(16), *buf = (char*)malloc(4);
	memset(res, 0, 16);
	memset(buf, 0, 4);
	if (ip==0) strncpy(res, "0.0.0.0", 8);
	while (ip > 0)
	{
		sprintf(buf, "%d", (uint)ip%256);
		strcat(res, buf);
		memset(buf, 0, 4);
		ip /= 256;
		if (ip>0) strcat(res, ".");
	}
	res = (char*)realloc(res, strlen(res)+1);
	free(buf);
	buf = NULL;
	return res;
}

// string ip to int
unsigned int myinet_aton(char *ip)
{
	struct in_addr in;
	inet_aton(ip, &in);
	return in.s_addr;
}

// recieve precise number of bytes 
int recvall(int sock, uchar *buf, int n)
{
	int c = 0, l = 0;
	uchar *buff = (uchar*)malloc(sizeof(char)*n);
	memset(buf, 0, n);
	memset(buff, 0, n);
	while (c < n)
	{
		l = recv(sock, buff, n-c, 0);
		if (l<=0) 
		{
			free(buff);
			buff = NULL;
			return 0;
		}
		buf = strcatd(buf, buff, c, l);
		c += l;
	}
	free(buff);
	buff = NULL;
	return n;
}

// return address converted from string type 'xxx.xxx.xxx.xxx:port' to Address structure
Address get_host(char *str)
{
	Address addr;
	addr.ip = 0;
	addr.port = 0;
	int l = strlen(str), c = 0;
	char *buf = (char*)malloc(sizeof(char)*l);
	memset(buf, 0, 0);
	bool i = FALSE;
	while (*str)
	{
		if (i==FALSE && *str==':')
		{
			buf[c] = '\0';
			c = 0;
			addr.ip = myinet_aton(buf);
			memset(buf, 0, l);
			i = TRUE;
			*str++;
			continue;
		}
		buf[c++] = *str++;
	}
	buf[c] = '\0';
	if (i==TRUE) addr.port = atoi(buf);
	else
	{
		addr.ip = myinet_aton(buf);
		addr.port = 1;
	}
	free(buf);
	buf = NULL;
	return addr;
}

/* create/connect to server */
int create_server(Address addr)
{
	int sock, clilen;
	int opt = TRUE;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0))<=0) perr("error creating socket");
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) perr("error settings socket multiple connections");
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	if (addr.ip==0) addr.ip = INADDR_ANY;
	serv_addr.sin_addr.s_addr = addr.ip;
	serv_addr.sin_port = htons(addr.port);
	if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) perr("error on binding");
	if (listen(sock, 3) < 0) perr("error on listen");
	return sock;
}

int connection(Address addr)
{
	int sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in6 from;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		msg("ERROR: Cannot create socket");
		return -1;
	}
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = addr.ip;
	serv_addr.sin_port = htons(addr.port);
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		char *buf = myinet_ntoa(addr.ip);
		msg("ERROR: Cannot connect to assigned address '%s:%d'", buf, addr.port);
		free(buf);
		return -1;
	}
	return sock;
}
