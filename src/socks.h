/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#ifndef _socks_h_
#define _socks_h_

#include "net.h"
#include "config.h"

#define PROXY_ALL				1
#define PROXY_S4				4
#define PROXY_S5				5

// SOCKS5 auth methods
#define SOCKS5_AUTH_NO			0	// no auth
#define SOCKS5_AUTH_LOGINPASS	2	// login/password
#define NO_METHOD				255	// no selected method

#pragma pack(push, 1)
typedef struct TUNNEL
{
	uint client;
	uint server;
} Tunnel;

typedef struct SESSION
{
	int sock;
	struct sockaddr_in addr;
} Session;

typedef struct SOCKS
{
	Tunnel addr;
	char version;
} Socks;
#pragma pack(pop)

Socks get_proxy(int, struct sockaddr_in); // get proxy packet and respond
#endif
