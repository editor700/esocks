/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#ifndef _utils_h_
#define _utils_h_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define TRUE	((uchar)'\x01')
#define FALSE	((uchar)'\x00')

#ifdef SOCKS5
char *socks5_login, *socks5_password;
#endif

int clients;

typedef unsigned char bool;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

char *pname, *logfile;
bool output, daem;
uchar ptype;
int max_clients;

void perr(const char*, ...); // print error then exit

void msg(const char*, ...); // logging


/* standart functions */
inline uchar *strcatd(uchar*, uchar*, int, int);

inline bool in_str(uchar*, uchar, int);
#endif
