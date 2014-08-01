/*
 * Copyright (c) 1990 Jan-Simon Pendry
 * Copyright (c) 1990 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)get_args.c	5.4 (Berkeley) 2/9/92
 *
 * $Id: get_args.c,v 5.2.2.1 1992/02/09 15:08:23 jsp beta $
 *
 */

/*
 * Argument decode
 */

#include "am.h"
#ifdef HAS_SYSLOG
#include <syslog.h>
#endif /* HAS_SYSLOG */
#include <sys/stat.h>

extern int optind;
extern char *optarg;

#if defined(DEBUG) && defined(PARANOID)
char **gargv;
#endif /* defined(DEBUG) && defined(PARANOID) */
int restart_existing_mounts;
int print_pid;
int normalize_hosts;
char *karch;			/* Kernel architecture */
char *cluster;			/* Cluster name */
#ifdef HAS_NIS_MAPS
char *domain;			/* YP domain */
#endif /* HAS_NIS_MAPS */
#ifdef UPDATE_MTAB
char *mtab;
#endif /* UPDATE_MTAB */
int afs_timeo = -1;
int afs_retrans = -1;
int am_timeo = AM_TTL;
int am_timeo_w = AM_TTL_W;

#ifdef DEBUG
/*
 * List of debug options.
 */
static struct opt_tab dbg_opt[] = {
	{ "all", D_ALL },		/* All */
	{ "amq", D_AMQ },		/* Register for AMQ program */
	{ "daemon", D_DAEMON },		/* Enter daemon mode */
	{ "full", D_FULL },		/* Program trace */
	{ "mem", D_MEM },		/* Trace memory allocations */
	{ "mtab", D_MTAB },		/* Use local mtab file */
	{ "str", D_STR },		/* Debug string munging */
	{ "test", D_TEST },		/* Full debug - but no daemon */
	{ "trace", D_TRACE },		/* Protocol trace */
	{ 0, 0 }
};

int debug_flags = D_AMQ			/* Register AMQ */
		 |D_DAEMON		/* Enter daemon mode */
		 ;

/*
 * Switch on/off debug options
 */
int debug_option(opt)
char *opt;
{
	return cmdoption(opt, dbg_opt, &debug_flags);
}
#endif /* DEBUG */

void get_args(c, v)
int c;
char *v[];
{
	int opt_ch;
	int usage = 0;
	char *logfile = 0;
	char *sub_domain = 0;

	while ((opt_ch = getopt(c, v, "mnprva:c:d:h:k:l:t:w:x:y:C:D:")) != EOF)
	switch (opt_ch) {
	case 'a':
		if (*optarg != '/') {
			fprintf(stderr, "%s: -a option must begin with a '/'\n",
					progname);
			exit(1);
		}
		auto_dir = optarg;
		break;

	case 'c':
		am_timeo = atoi(optarg);
		if (am_timeo <= 0)
			am_timeo = AM_TTL;
		break;

	case 'd':
		sub_domain = optarg;
		break;

	case 'h':
#if defined(HAS_HOST) && defined(HOST_EXEC)
		host_helper = optarg;
#else
		plog(XLOG_USER, "-h: option ignored.  HOST_EXEC is not enabled.");
		break;
#endif /* defined(HAS_HOST) && defined(HOST_EXEC) */

	case 'k':
		karch = optarg;
		break;

	case 'l':
		logfile = optarg;
		break;

	case 'm':
		plog(XLOG_USER, "The -m option is no longer supported.");
		plog(XLOG_USER, "... Use `ypcat -k am.master` on the command line instead");
		break;

	case 'n':
		normalize_hosts = 1;
		break;

	case 'p':
		print_pid = 1;
		break;

	case 'r':
		restart_existing_mounts = 1;
		break;

	case 't':
		/* timeo.retrans */
		{ char *dot = strchr(optarg, '.');
		  if (dot) *dot = '\0';
		  if (*optarg) {
			afs_timeo = atoi(optarg);
		  }
		  if (dot) {
		  	afs_retrans = atoi(dot+1);
			*dot = '.';
		  }
		}
		break;

	case 'v':
		fprintf(stderr, "%s%s (%s-endian).\n", copyright, version, endian);
		fputs("Map support for: ", stderr);
		mapc_showtypes(stderr);
		fputs(".\nFS: ", stderr);
		ops_showfstypes(stderr);
		fputs(".\n", stderr);
		fprintf(stderr, "Primary network is %s.\n", wire);
		exit(0);
		break;

	case 'w':
		am_timeo_w = atoi(optarg);
		if (am_timeo_w <= 0)
			am_timeo_w = AM_TTL_W;
		break;

	case 'x':
		usage += switch_option(optarg);
		break;

	case 'y':
#ifdef HAS_NIS_MAPS
		domain = optarg;
#else
		plog(XLOG_USER, "-y: option ignored.  No NIS support available.");
#endif /* HAS_NIS_MAPS */
		break;

	case 'C':
		cluster = optarg;
		break;

	case 'D':
#ifdef DEBUG
		usage += debug_option(optarg);
#else
		fprintf(stderr, "%s: not compiled with DEBUG option -- sorry.\n", progname);
#endif /* DEBUG */
		break;

	default:
		usage = 1;
		break;
	}

	if (xlog_level_init == ~0) {
		(void) switch_option("");
#ifdef DEBUG
		usage += switch_option("debug");
#endif /* DEBUG */
	} else {
#ifdef DEBUG
		usage += switch_option("debug");
#endif /* DEBUG */
	}

	if (usage)
		goto show_usage;

	while (optind <= c-2) {
		char *dir = v[optind++];
		char *map = v[optind++];
		char *opts = "";
		if (v[optind] && *v[optind] == '-')
			opts = &v[optind++][1];

		root_newmap(dir, opts, map);
	}

	if (optind == c) {
#ifdef hpux
		/*
		 * HP-UX can't handle ./mtab
		 * That system is sick - really.
		 */
#ifdef	DEBUG
		debug_option("nomtab");
#endif	/* DEBUG */
#endif	/* hpux */

		/*
		 * Append domain name to hostname.
		 * sub_domain overrides hostdomain
		 * if given.
		 */
		if (sub_domain)
			hostdomain = sub_domain;
		if (*hostdomain == '.')
			hostdomain++;
		strcat(hostd,  ".");
		strcat(hostd, hostdomain);

#ifdef UPDATE_MTAB
#ifdef DEBUG
		if (debug_flags & D_MTAB)
			mtab = DEBUG_MTAB;
		else
#endif /* DEBUG */
		mtab = MOUNTED;
#else
#ifdef DEBUG
		{ if (debug_flags & D_MTAB) {
			dlog("-D mtab option ignored");
		} }
#endif /* DEBUG */
#endif /* UPDATE_MTAB */

		if (switch_to_logfile(logfile) != 0)
			plog(XLOG_USER, "Cannot switch logfile");

		/*
		 * If the kernel architecture was not specified
		 * then use the machine architecture.
		 */
		if (karch == 0)
			karch = arch;

		if (cluster == 0)
			cluster = hostdomain;

		if (afs_timeo <= 0)
			afs_timeo = AFS_TIMEO;
		if (afs_retrans <= 0)
			afs_retrans = AFS_RETRANS;
		if (afs_retrans <= 0)
			afs_retrans = 3;	/* XXX */
		return;
	}

show_usage:
	fprintf(stderr,
"Usage: %s [-mnprv] [-a mnt_point] [-c cache_time] [-d domain]\n\
\t[-k kernel_arch] [-l logfile|\"syslog\"] [-t afs_timeout]\n\
\t[-w wait_timeout] [-C cluster_name]", progname);

#if defined(HAS_HOST) && defined(HOST_EXEC)
	fputs(" [-h host_helper]\n", stderr);
#endif /* defined(HAS_HOST) && defined(HOST_EXEC) */

#ifdef HAS_NIS_MAPS
	fputs(" [-y nis-domain]\n", stderr);
#else
	fputc('\n', stderr);
#endif /* HAS_NIS_MAPS */

	show_opts('x', xlog_opt);
#ifdef DEBUG
	show_opts('D', dbg_opt);
#endif /* DEBUG */
	fprintf(stderr, "\t{directory mapname [-map_options]} ...\n");
	exit(1);
}
