/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#include "utils.h"

// print error then exit
void perr(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	printf("%s: ", pname);
	vprintf(msg, args);
	printf("\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

// logging
void msg(const char *msg, ...)
{
	if (logfile==NULL && !output) return;
	time_t rawtime;
	struct tm * timeinfo;
	va_list args;
	char buf[80];
	time( &rawtime );
	timeinfo = localtime(&rawtime);
	strftime(buf, 80, "[%d/%m/%Y %H:%M:%S]",timeinfo);
	if (output)
	{
		va_start(args, msg);
		printf("%s ", buf);
		vprintf(msg, args);
		printf("\n");
		va_end(args);
	}
	if (logfile!=NULL)
	{
		va_start(args, msg);
		FILE *f = fopen(logfile, "a");
		fprintf(f, "%s ", buf);
		vfprintf(f, msg, args);
		fprintf(f, "\n");
		fclose(f);
		va_end(args);
	}
}


/* standart functions */

// convert char array to long long int
inline uchar *strcatd(uchar *buf, uchar *str, int b, int l)
{
	int i = 0;
	if (b >= l) return buf;
	while (i < l) buf[b+i] = str[i++];
	return buf;
}

inline bool in_str(uchar *str, uchar c, int l)
{
	int i = 0;
	while (i < l) if (str[i++]==c) return TRUE;
	return FALSE;
}
