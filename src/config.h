/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#ifndef _config_h_
#define _config_h_

#define PORT		1080	// default port

#define BUF_SIZE	1024	// for proxy tunneling

#define MAX_CLIENTS	0		// default max_clients (0 = no limit)

// SOCKS5 auth
#ifdef SOCKS5
#define SOCKS5_LOGIN		""

#define SOCKS5_PASSWORD		""
#endif

#endif
