/*
 * Copyright (c) editor700
 * (c) 2015 <editor700@gmail.com>
 */

#include "esocks.h"

Address addr;

void print_version() { printf("%s version %s, by %s (%s)\n", NAME, VERSION, AUTHOR, ADDRESS); }

void parse_args(int argc, char *argv[])
{
	int i;
	pname = argv[0];
	for (i = 1; i < argc; i++)
	{
		char *arg = argv[i];
		if (*arg!='-')
		{
			perr("wrong option '%s'", arg);
			continue;
		}
		else *arg++;
		if (strcmp(arg, "help")==0 || in_str(arg, 'h', 1) || in_str(arg, '?', 1))
		{
			print_version();
			printf("Usage: %s [-?hvHpalodt] [--help] [--version] [--output] [--daemon] [--log filename] [--host host_address] [--address host_address:port] [--port port_num] [--type socks_type] [--password password] [--login login] [--max max_conns]\n\nOptions:\n", pname);
			printf("  -h, -?, --help                                 Print help.\n");
			printf("  -v, --version                                  Print version.\n");
			printf("  -o, --output                                   Print logging output in stdout.\n");
			printf("  -d, --daemon                                   Run server as daemon. (superuser access required)\n");
			printf("  -l, --log logle                                Logging into logle.\n");
			printf("  -H, -a, --host, --address host_address:port    Set server listen on host_address:port. If port omitted, server will listen 1080.\n");
			printf("  -p, --port port_num                            Set server listen port_num port.\n");
			printf("  -m, --max max_conns                            Set maximum connections number, 0 = no limit.\n");
#ifdef SOCKS5
			printf("  -L, --login, --username, --user login          Set username for socks5 connection.\n");
			printf("  -P, --pass, --password password                Set password for socks5 connection.\n");
#endif
#if defined SOCKS4  || defined SOCKS5
			printf("  -t, --type socks_type                          Set server use specified socks version (4 - socks4, 5 - socks4, all - socks4 and socks5).\n\n");
#endif
			exit(0);
		}
		else if (strcmp(arg, "version")==0 || in_str(arg, 'v', 1))
		{
			print_version();
			exit(0);
		}
		else if (strcmp(arg, "host")==0 || in_str(arg, 'H', 1) || strcmp(arg, "address")==0 || in_str(arg, 'a', 1))
		{
			if (i+1==argc) perr("no host was found");
			Address ad = get_host(argv[i+1]);
			if (ad.ip || ad.port)
			{
				if (ad.ip) addr.ip = ad.ip;
				if (ad.port > 1) addr.port = ad.port;
				i++;
			}
			else perr("wrong host address '%s'", argv[i+1]);
		}
		else if (strcmp(arg, "port")==0 || in_str(arg, 'p', 1))
		{
			if (i+1==argc) perr("no ports was found");
			ushort p = atoi(argv[i+1]);
			if (p)
			{
				addr.port = p;
				i++;
			}
			else perr("wrong port number '%s'", argv[i+1]);
		}
		else if (strcmp(arg, "log")==0 || in_str(arg, 'l', 1))
		{
			if (i+1==argc) perr("no log file");
			logfile = argv[++i];
		}
		else if (strcmp(arg, "output")==0 || in_str(arg, 'o', 1))
		{
			output = TRUE;
		}
		else if (strcmp(arg, "daemon")==0 || in_str(arg, 'd', 1))
		{
			output = FALSE;
			daem = TRUE;
		}
		else if (strcmp(arg, "max")==0 || in_str(arg, 'm', 1))
		{
			if (i+1==argc) perr("max_conn not specified");
			uint p = atoi(argv[i+1]);
			if (p)
			{
				max_clients = p;
				i++;
			}
			else perr("wrong max_conn number");
		}
#ifdef SOCKS5
		else if (strcmp(arg, "login")==0 || strcmp(arg, "username")==0 || strcmp(arg, "user")==0 || in_str(arg, 'L', 1))
		{
			if (i+1==argc) perr("no username specified");
			socks5_login = argv[++i];
		}
		else if (strcmp(arg, "password")==0 || strcmp(arg, "pass")==0 || in_str(arg, 'P', 1))
		{
			if (i+1==argc) perr("no password specified");
			socks5_password = argv[++i];
		}
#endif
#if defined SOCKS4 || defined SOCKS5
		else if (strcmp(arg, "type")==0 || in_str(arg, 't', 1))
		{
			if (i+1==argc) perr("socks type not specified");
			if (strcmp(argv[i+1], "all")==0) i++;
#ifdef SOCKS4
			else if (strcmp(argv[i+1], "4")==0)
			{
				ptype = PROXY_S4;
				i++;
			}
#endif
#ifdef SOCKS5
			else if (strcmp(argv[i+1], "5")==0)
			{
				ptype = PROXY_S5;
				i++;
			}
#endif
			else perr("incorrect socks proxy type '%s'", argv[i+1]);
		}
#endif
		else perr("wrong option '%s'", arg);
	}
}

// signal handlers
void sigpipe(int arg)
{
}


void sigint(int arg)
{
	printf("\n");
	msg("User sent Interrupt. Shutting down.");
	exit(0);
}

// Proxy handler (tunnel)
void handler(Socks serv)
{
	int act, len, server = serv.addr.server, client = serv.addr.client, max = MAX(client, server);
	char *buffer = (char*)malloc(BUF_SIZE);
	fd_set readfds;
	while (TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(client, &readfds);
		FD_SET(server, &readfds);
		act = select(max+1, &readfds, NULL, NULL, NULL);
		if (act < 0) break;
		if (FD_ISSET(client, &readfds)) if ((len = read(client, buffer, BUF_SIZE))<=0) break;
		else send(server, buffer, len, 0);
		if (FD_ISSET(server, &readfds)) if ((len = read(server, buffer, BUF_SIZE))<=0) break;
		else send(client, buffer, len, 0);
	}
	shutdown(client, SHUT_RDWR);
	close(client);
	free(buffer);
	buffer = NULL;
}

// SOCKS bootstrap
void *bootstrap(void *arg)
{
	Session sock = *(Session*)arg;
	Socks serv = get_proxy(sock.sock, sock.addr); // get proxy packet and respond
	if (serv.addr.server && serv.addr.server!=-1) handler(serv); // if connection valid, go to handler
	shutdown(serv.addr.server, SHUT_RDWR);
	close(serv.addr.server);
#if MAX_CLIENTS > 0
	clients--;
#endif
}

int main(int argc, char *argv[])
{
	int sock;

	output = FALSE;
	daem = FALSE;
	logfile = NULL;
	ptype = PROXY_ALL;
	socks5_login = SOCKS5_LOGIN;
	socks5_password = SOCKS5_PASSWORD;
	max_clients = MAX_CLIENTS;
	clients = 0;
	addr.ip = INADDR_ANY;
	addr.port = PORT;

	parse_args(argc, argv);

	signal(SIGPIPE, sigpipe);
	signal(SIGINT, sigint);
	signal(SIGTERM, sigint);

	sock = create_server(addr);

	if (!sock) perr("error creating server");
	if (daem) // make a daemon, if required
	{
		pid_t pid, sid;
		pid = fork();
		if (pid < 0) perr("error creating daemon");
		else if (pid > 0) exit(EXIT_SUCCESS);
		umask(0);
		sid = setsid();
		if (sid < 0) perr("error creating SID");
		if (getuid()) perr("error creating daemon (superuser access required)");
		char *buf = (char*)malloc(255);
		buf = getcwd(buf, 255);
		if ((chdir(buf)) < 0) perr("error chdir");
		free(buf);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	char *buf = myinet_ntoa(addr.ip);
	msg("Server started at %s:%d", buf, addr.port);
	free(buf);
	while (TRUE) // main loop
	{
		struct sockaddr_in cli_addr;
		int client, clilen = sizeof(cli_addr);
		if ((client = accept(sock, (struct sockaddr*)&cli_addr, &clilen)) > 0) // get new connection and go to bootstrap
		{
			if (max_clients > 0) clients++;
			Session serv;
			serv.sock = client;
			serv.addr = cli_addr;
			pthread_t thread;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, 64 * 1024);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_create(&thread, &attr, bootstrap, &serv);
			pthread_attr_destroy(&attr);
			pthread_detach(thread);
		}
	}
	return 0;
}
