/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * Client program to send "watchdog info" command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "utils/getopt_long.h"
#endif

#include "pcp/pcp.h"

static void usage(void);
static void myexit(PCPConnInfo* pcpConn);

int
main(int argc, char **argv)
{
	char host[MAX_DB_HOST_NAMELEN];
	int port;
	char user[MAX_USER_PASSWD_LEN];
	char pass[MAX_USER_PASSWD_LEN];
	int wd_index;
	WdInfo *watchdog_info;
	int ch;
	int optindex;
	bool verbose = false;
	bool debug = false;
	PCPConnInfo* pcpConn;
	PCPResultInfo* pcpResInfo;


	static struct option long_options[] = {
		{"debug", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{"verbose", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	
	while ((ch = getopt_long(argc, argv, "hdv", long_options, &optindex)) != -1) {
		switch (ch) {
		case 'd':
				debug = true;
			break;

		case 'v':
			verbose = true;
			break;

		case 'h':
		case '?':
		default:
			usage();
			exit(0);
		}
	}
	argc -= optind;
	argv += optind;

	if (!(argc == 4 || argc == 5))
		myexit(NULL);

	if (strlen(argv[0]) >= MAX_DB_HOST_NAMELEN)
		myexit(NULL);
	strcpy(host, argv[0]);

	port = atoi(argv[1]);
	if (port <= 1024 || port > 65535)
		myexit(NULL);

	if (strlen(argv[2]) >= MAX_USER_PASSWD_LEN)
		myexit(NULL);
	strcpy(user, argv[2]);

	if (strlen(argv[3]) >= MAX_USER_PASSWD_LEN)
		myexit(NULL);
	strcpy(pass, argv[3]);

	if (argc == 5)
	{
		wd_index = atoi(argv[4]);
		if (wd_index < 0 || wd_index > MAX_WATCHDOG_NUM)
			myexit(NULL);
		wd_index++;
	}
	else
	{
		wd_index = 0;
	}

	pcpConn = pcp_connect(host, port, user, pass, debug?stdout:NULL);
	if(PCPConnectionStatus(pcpConn) != PCP_CONNECTION_OK)
		myexit(pcpConn);

	pcpResInfo = pcp_watchdog_info(pcpConn,wd_index);
	if(PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
		myexit(pcpConn);

	watchdog_info = (WdInfo *)pcp_get_binary_data(pcpResInfo,0);
	if (verbose)
	{
		printf("Hostname     : %s\nPgpool port  : %d\nWatchdog port: %d\nStatus       : %d\n",
			   watchdog_info->hostname,
			   watchdog_info->pgpool_port,
			   watchdog_info->wd_port,
			   watchdog_info->status);
	}
	else
	{
		printf("%s %d %d %d\n",
			   watchdog_info->hostname,
			   watchdog_info->pgpool_port,
			   watchdog_info->wd_port,
			   watchdog_info->status);
	}

	pcp_disconnect(pcpConn);
	pcp_free_connection(pcpConn);
	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_watchdog_info - display a pgpool-II watchdog's information\n\n");
	fprintf(stderr, "Usage: pcp_watchdog_info [-d] hostname port# username password [watchdogID]\n");
	fprintf(stderr, "  -d, --debug    : enable debug message (optional)\n");
	fprintf(stderr, "  hostname       : pgpool-II hostname\n");
	fprintf(stderr, "  port#          : PCP port number\n");
	fprintf(stderr, "  username       : username for PCP authentication\n");
	fprintf(stderr, "  password       : password for PCP authentication\n");
	fprintf(stderr, "  watchdogID     : ID of a other pgpool to get information for\n");
	fprintf(stderr, "                   If omitted then get one's self information\n\n");
	fprintf(stderr, "Usage: pcp_watchdog_info [options]\n");
	fprintf(stderr, "  Options available are:\n");
	fprintf(stderr, "  -h, --help     : print this help\n");
	fprintf(stderr, "  -v, --verbose  : display one line per information with a header\n");
}

static void
myexit(PCPConnInfo* pcpConn)
{
	if (pcpConn == NULL)
	{
		usage();
	}
	else
	{
		fprintf(stderr, "%s\n",pcp_get_last_error(pcpConn)?pcp_get_last_error(pcpConn):"Unknown Error");
		pcp_disconnect(pcpConn);
		pcp_free_connection(pcpConn);
	}
	exit(-1);
}
