/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#include "socks.h"

// get proxy packet and respond
Socks get_proxy(int sock, struct sockaddr_in cli_addr)
{
	Socks res;
	Address serv;
	res.addr.server = 0;
	serv.type = FALSE;
	uchar buf[4], hostname[255], *buff;
	int n = recvall(sock, buf, 1), server;
	res.version = buf[0]; // get SOCKS version
	if (buf[0]==4 && (ptype==PROXY_ALL || ptype==PROXY_S4)) // SOCKS4
	{
#ifdef SOCKS4
		n = recvall(sock, buf, 1); // get command code (only connect, ignore binding)
		// get port number
		n = recvall(sock, buf, 2);
		serv.port = buf[0]*256+buf[1];
		// get ip address
		n = recvall(sock, buf, 4);
		serv.ip = ((buf[3]*256+buf[2])*256+buf[1])*256+buf[0];
		// get user id (if exist) ignore this
		buf[0] = 1;
		while (buf[0]!=0) recvall(sock, buf, 1);
		// if SOCKS4a, there may be hostname
		if (serv.ip==0x1000000)
		{
			int i = 0;
			recvall(sock, buf, 1);
			memset(hostname, 0, 255);
			while (buf[0]!=0)
			{
				hostname[i++] = buf[0];
				recvall(sock, buf, 1);
			}
			struct hostent *ad = gethostbyname(hostname);
			if (ad==NULL)
			{
				msg("ERROR: No such host '%s'", hostname);
				free(buff);
				return res;
			}
			serv.ip = ((struct in_addr*)ad->h_addr)->s_addr;
		}
		if (clients > max_clients)
		{
			n = send(sock, "\0\x5b\0\0\0\0\0\0", 8, 0);
			free(buff);
			msg("Too many connections");
			return res;
		}
		// then connect
		server = connection(serv);
		if (server)
		{
			n = send(sock, "\0\x5a\0\0\0\0\0\0", 8, 0);
			res.addr.server = server;
		}
		else n = send(sock, "\0\x5b\0\0\0\0\0\0", 8, 0);
#else
		msg("ERROR: Server does not support SOCKS4 protocol");
		free(buff);
		return res;
#endif
	}
	else if (buf[0]==5 && (ptype==PROXY_ALL || ptype==PROXY_S5)) // SOCKS5
	{
#ifdef SOCKS5
		// get auth methods
		n = recvall(sock, buf, 1);
		int l = buf[0], g = 0;
		uchar m = NO_METHOD;
		while (g++ < l)
		{
			n = recvall(sock, buf, 1);
			if (m==NO_METHOD && buf[0]==SOCKS5_AUTH_LOGINPASS) m = SOCKS5_AUTH_LOGINPASS;
			else if (m==NO_METHOD && buf[0]==SOCKS5_AUTH_NO && strlen(socks5_login)==0 && strlen(socks5_password)==0) m = SOCKS5_AUTH_NO;
		}
		// send selected method
		buf[0] = 5;
		buf[1] = m;
		n = send(sock, buf, 2, 0);
		// authorization
		if (m==NO_METHOD)
		{
			buff = myinet_ntoa(cli_addr.sin_addr.s_addr);
			msg("ERROR: Server does not support methods that used by client %s:%d", buff, cli_addr.sin_port);
			free(buff);
			return res;
		}
		if (m==SOCKS5_AUTH_LOGINPASS) // login/password
		{
			char username[255], password[255];
			n = recvall(sock, buf, 2);
			l = buf[1];
			n = recvall(sock, username, l);
			username[l] = 0;
			n = recvall(sock, buf, 1);
			l = buf[0];
			n = recvall(sock, password, l);
			password[l] = 0;
			buf[0]=1;
			if (strcmp(socks5_login, username)==0 && strcmp(socks5_password, password)==0) buf[1]=0;
			else buf[1]=1;
			n = send(sock, buf, 2, 0);
			if (buf[1]!=0)
			{
				buff = myinet_ntoa(cli_addr.sin_addr.s_addr);
				msg("ERROR: Failed authorization %s:%d", buff, cli_addr.sin_port);
				free(buff);
				return res;
			}
		}
		// get client packet
		n = recvall(sock, buf, 4);
		if (buf[0]!=5 || buf[1]>3 || buf[2]!=0)
		{
			buff = myinet_ntoa(cli_addr.sin_addr.s_addr);
			msg("ERROR: Wrong SOCKS5 data got from %s:%d", buff, cli_addr.sin_port);
			free(buff);
			return res;
		}
		// address type
		if (buf[3]==1) // ip address
		{
			n = recvall(sock, buf, 4);
			serv.ip = ((buf[3]*256+buf[2])*256+buf[1])*256+buf[0];
		}
		else if (buf[3]==3) // hostname
		{
			n = recvall(sock, buf, 1);
			l = buf[0];
			memset(hostname, 0, 255);
			n = recvall(sock, hostname, l);
			struct hostent *ad = gethostbyname(hostname);
			if (ad==NULL)
			{
				msg("ERROR: No such host '%s'", hostname);
				free(buff);
				return res;
			}
			serv.ip = ((struct in_addr*)ad->h_addr)->s_addr;
		}
		else if (buf[3]==4) // I will add IPv6 support in future
		{
			buff = myinet_ntoa(cli_addr.sin_addr.s_addr);
			msg("ERROR: Server does not support IPv6 (%s:%d)", buff, cli_addr.sin_port);
			n = recvall(sock, hostname, 18);
			free(buff);
			return res;
		}
		// get port number
		n = recvall(sock, buf, 2);
		serv.port = buf[0]*256+buf[1];
		if (clients > max_clients)
		{
			n = send(sock, "\x05\x04\0\x01\0\0\0\0\0\0", 10, 0);
			msg("Too many connections");
			free(buff);
			return res;
		}
		server = connection(serv); // then connect
		if (server)
		{
			n = send(sock, "\x05\0\0\x01\0\0\0\0\0\0", 10, 0);
			res.addr.server = server;
		}
		else n = send(sock, "\x05\x04\0\x01\0\0\0\0\0\0", 10, 0);
#else
		msg("ERROR: Server does not support SOCKS5 protocol");
		free(buff);
		return res;
#endif
	}
	else // unknown
	{
		buff = myinet_ntoa(cli_addr.sin_addr.s_addr);
		msg("ERROR: Not SOCKS format packet got from %s:%d", buff, cli_addr.sin_port);
		free(buff);
		return res;
	}
	buff = myinet_ntoa(cli_addr.sin_addr.s_addr);
	char *buf2 = myinet_ntoa(serv.ip);
	msg("New SOCKS%d connection from %s:%d to %s:%d", res.version, myinet_ntoa(cli_addr.sin_addr.s_addr), cli_addr.sin_port, buff = myinet_ntoa(serv.ip), serv.port);
	res.addr.client = sock;
	free(buff);
	free(buf2);
	return res;
}
