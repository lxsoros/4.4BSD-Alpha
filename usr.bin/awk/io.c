/*
 * io.c - routines for dealing with input and output and records
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991, 1992 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"

#ifndef O_RDONLY
#include <fcntl.h>
#endif

#ifndef atarist
#define INVALID_HANDLE (-1)
#else
#define INVALID_HANDLE  (__SMALLEST_VALID_HANDLE - 1)
#endif

static IOBUF *nextfile P((int skipping));
static int inrec P((IOBUF *iop));
static int iop_close P((IOBUF *iop));
struct redirect *redirect P((NODE *tree, int *errflg));
static void close_one P((void));
static int close_redir P((struct redirect *rp));
#if (!defined(MSDOS)) && (!defined(atarist))
static int wait_any P((int interesting));
#endif
static IOBUF *gawk_popen P((char *cmd, struct redirect *rp));
static int gawk_pclose P((struct redirect *rp));
static int do_pathopen P((char *file));
static int checkopen P((char *file, int flag, int mode));

static struct redirect *red_head = NULL;

extern int output_is_tty;
extern NODE *ARGC_node;
extern NODE *ARGV_node;
extern NODE **fields_arr;

static jmp_buf filebuf;		/* for nextfile() */

void
do_nextfile()
{
	(void) nextfile(1);
	longjmp(filebuf, 1);
}

static IOBUF *
nextfile(skipping)
int skipping;
{
	static int i = 1;
	static int files = 0;
	NODE *arg;
	int fd = INVALID_HANDLE;
	static IOBUF *curfile = NULL;

	if (skipping) {
		if (curfile != NULL)
			iop_close(curfile);
		curfile = NULL;
		return NULL;
	}
	if (curfile != NULL) {
		if (curfile->cnt == EOF)
			(void) iop_close(curfile);
		else
			return curfile;
	}
	for (; i < (int) (ARGC_node->lnode->numbr); i++) {
		arg = *assoc_lookup(ARGV_node, tmp_number((AWKNUM) i));
		if (arg->stptr[0] == '\0')
			continue;
		arg->stptr[arg->stlen] = '\0';
		if (!arg_assign(arg->stptr)) {
			files++;
			fd = devopen(arg->stptr, "r");
			if (fd == INVALID_HANDLE)
				fatal("cannot open file `%s' for reading (%s)",
					arg->stptr, strerror(errno));
				/* NOTREACHED */
			/* This is a kludge.  */
			unref(FILENAME_node->var_value);
			FILENAME_node->var_value =
				dupnode(arg);
			FNR = 0;
			i++;
			break;
		}
	}
	if (files == 0) {
		files++;
		/* no args. -- use stdin */
		/* FILENAME is init'ed to "-" */
		/* FNR is init'ed to 0 */
		fd = 0;
	}
	if (fd == INVALID_HANDLE)
		return curfile = NULL;
	return curfile = iop_alloc(fd);
}

void
set_FNR()
{
	FNR = (int) FNR_node->var_value->numbr;
}

void
set_NR()
{
	NR = (int) NR_node->var_value->numbr;
}

/*
 * This reads in a record from the input file
 */
static int
inrec(iop)
IOBUF *iop;
{
	char *begin;
	register int cnt;
	int retval = 0;

	cnt = get_a_record(&begin, iop, *RS);
	if (cnt == EOF) {
		cnt = 0;
		retval = 1;
	} else {
		    NR += 1;
		    FNR += 1;
	}
	set_record(begin, cnt, 1);

	return retval;
}

static int
iop_close(iop)
IOBUF *iop;
{
	int ret;

	if (iop == NULL)
		return 0;
	errno = 0;

#ifdef _CRAY
	/* Work around bug in UNICOS popen */
	if (iop->fd < 3)
		ret = 0;
	else
#endif
	ret = close(iop->fd);
	if (ret == -1)
		warning("close of fd %d failed (%s)", iop->fd, strerror(errno));
	if (iop->buf)
		free(iop->buf);
	free((char *)iop);
	return ret == -1 ? 1 : 0;
}

void
do_input()
{
	IOBUF *iop;
	extern int exiting;

	if (setjmp(filebuf) != 0) {
	}
	while ((iop = nextfile(0)) != NULL) {
		if (inrec(iop) == 0)
			while (interpret(expression_value) && inrec(iop) == 0)
				;
		if (exiting)
			break;
	}
}

/* Redirection for printf and print commands */
struct redirect *
redirect(tree, errflg)
NODE *tree;
int *errflg;
{
	register NODE *tmp;
	register struct redirect *rp;
	register char *str;
	int tflag = 0;
	int outflag = 0;
	char *direction = "to";
	char *mode;
	int fd;
	char *what;

	switch (tree->type) {
	case Node_redirect_append:
		tflag = RED_APPEND;
	case Node_redirect_output:
		outflag = (RED_FILE|RED_WRITE);
		tflag |= outflag;
		if (tree->type == Node_redirect_output)
			what = ">";
		else
			what = ">>";
		break;
	case Node_redirect_pipe:
		tflag = (RED_PIPE|RED_WRITE);
		what = "|";
		break;
	case Node_redirect_pipein:
		tflag = (RED_PIPE|RED_READ);
		what = "|";
		break;
	case Node_redirect_input:
		tflag = (RED_FILE|RED_READ);
		what = "<";
		break;
	default:
		fatal ("invalid tree type %d in redirect()", tree->type);
		break;
	}
	tmp = tree_eval(tree->subnode);
	if (do_lint && ! (tmp->flags & STR))
		warning("expression in `%s' redirection only has numeric value",
			what);
	tmp = force_string(tmp);
	str = tmp->stptr;
	if (str == NULL || *str == '\0')
		fatal("expression for `%s' redirection has null string value",
			what);
	if (do_lint
	    && (STREQN(str, "0", tmp->stlen) || STREQN(str, "1", tmp->stlen)))
		warning("filename `%s' for `%s' redirection may be result of logical expression", str, what);
	for (rp = red_head; rp != NULL; rp = rp->next)
		if (strlen(rp->value) == tmp->stlen
		    && STREQN(rp->value, str, tmp->stlen)
		    && ((rp->flag & ~RED_NOBUF) == tflag
			|| (outflag
			    && (rp->flag & (RED_FILE|RED_WRITE)) == outflag)))
			break;
	if (rp == NULL) {
		emalloc(rp, struct redirect *, sizeof(struct redirect),
			"redirect");
		emalloc(str, char *, tmp->stlen+1, "redirect");
		memcpy(str, tmp->stptr, tmp->stlen);
		str[tmp->stlen] = '\0';
		rp->value = str;
		rp->flag = tflag;
		rp->fp = NULL;
		rp->iop = NULL;
		rp->pid = 0;	/* unlikely that we're worried about init */
		rp->status = 0;
		/* maintain list in most-recently-used first order */
		if (red_head)
			red_head->prev = rp;
		rp->prev = NULL;
		rp->next = red_head;
		red_head = rp;
	}
	while (rp->fp == NULL && rp->iop == NULL) {
		mode = NULL;
		errno = 0;
		switch (tree->type) {
		case Node_redirect_output:
			mode = "w";
			if (rp->flag & RED_USED)
				mode = "a";
			break;
		case Node_redirect_append:
			mode = "a";
			break;
		case Node_redirect_pipe:
			if ((rp->fp = popen(str, "w")) == NULL)
				fatal("can't open pipe (\"%s\") for output (%s)",
					str, strerror(errno));
			rp->flag |= RED_NOBUF;
			break;
		case Node_redirect_pipein:
			direction = "from";
			if (gawk_popen(str, rp) == NULL)
				fatal("can't open pipe (\"%s\") for input (%s)",
					str, strerror(errno));
			break;
		case Node_redirect_input:
			direction = "from";
			rp->iop = iop_alloc(devopen(str, "r"));
			break;
		default:
			cant_happen();
		}
		if (mode != NULL) {
			fd = devopen(str, mode);
			if (fd > INVALID_HANDLE) {
				if (fd == fileno(stdin))
					rp->fp = stdin;
				else if (fd == fileno(stdout))
					rp->fp = stdout;
				else if (fd == fileno(stderr))
					rp->fp = stderr;
				else	
					rp->fp = fdopen(fd, mode);
				if (isatty(fd))
					rp->flag |= RED_NOBUF;
			}
		}
		if (rp->fp == NULL && rp->iop == NULL) {
			/* too many files open -- close one and try again */
#ifdef atarist
			if (errno == EMFILE)
#else
			if (errno == ENFILE || errno == EMFILE)
#endif
				close_one();
			else {
				/*
				 * Some other reason for failure.
				 *
				 * On redirection of input from a file,
				 * just return an error, so e.g. getline
				 * can return -1.  For output to file,
				 * complain. The shell will complain on
				 * a bad command to a pipe.
				 */
				*errflg = 1;
				if (tree->type == Node_redirect_output
				    || tree->type == Node_redirect_append)
					fatal("can't redirect %s `%s' (%s)",
					    direction, str, strerror(errno));
				else {
					free_temp(tmp);
					return NULL;
				}
			}
		}
	}
	free_temp(tmp);
	return rp;
}

static void
close_one()
{
	register struct redirect *rp;
	register struct redirect *rplast = NULL;

	/* go to end of list first, to pick up least recently used entry */
	for (rp = red_head; rp != NULL; rp = rp->next)
		rplast = rp;
	/* now work back up through the list */
	for (rp = rplast; rp != NULL; rp = rp->prev)
		if (rp->fp && (rp->flag & RED_FILE)) {
			rp->flag |= RED_USED;
			errno = 0;
			if (fclose(rp->fp))
				warning("close of \"%s\" failed (%s).",
					rp->value, strerror(errno));
			rp->fp = NULL;
			break;
		}
	if (rp == NULL)
		/* surely this is the only reason ??? */
		fatal("too many pipes or input files open"); 
}

NODE *
do_close(tree)
NODE *tree;
{
	NODE *tmp;
	register struct redirect *rp;

	tmp = force_string(tree_eval(tree->subnode));
	for (rp = red_head; rp != NULL; rp = rp->next) {
		if (strlen(rp->value) == tmp->stlen
		    && STREQN(rp->value, tmp->stptr, tmp->stlen))
			break;
	}
	free_temp(tmp);
	if (rp == NULL) /* no match */
		return tmp_number((AWKNUM) 0.0);
	fflush(stdout);	/* synchronize regular output */
	tmp = tmp_number((AWKNUM)close_redir(rp));
	rp = NULL;
	return tmp;
}

static int
close_redir(rp)
register struct redirect *rp;
{
	int status = 0;

	if (rp == NULL)
		return 0;
	errno = 0;
	if ((rp->flag & (RED_PIPE|RED_WRITE)) == (RED_PIPE|RED_WRITE))
		status = pclose(rp->fp);
	else if (rp->fp)
		status = fclose(rp->fp);
	else if (rp->iop) {
		if (rp->flag & RED_PIPE)
			status = gawk_pclose(rp);
		else {
			status = iop_close(rp->iop);
			rp->iop = NULL;
		}
	}
	/* SVR4 awk checks and warns about status of close */
	if (status)
		warning("failure status (%d) on %s close of \"%s\" (%s).",
			status,
			(rp->flag & RED_PIPE) ? "pipe" :
			"file", rp->value, strerror(errno));
	if (rp->next)
		rp->next->prev = rp->prev;
	if (rp->prev)
		rp->prev->next = rp->next;
	else
		red_head = rp->next;
	free(rp->value);
	free((char *)rp);
	return status;
}

int
flush_io ()
{
	register struct redirect *rp;
	int status = 0;

	errno = 0;
	if (fflush(stdout)) {
		warning("error writing standard output (%s).", strerror(errno));
		status++;
	}
	errno = 0;
	if (fflush(stderr)) {
		warning("error writing standard error (%s).", strerror(errno));
		status++;
	}
	for (rp = red_head; rp != NULL; rp = rp->next)
		/* flush both files and pipes, what the heck */
		if ((rp->flag & RED_WRITE) && rp->fp != NULL) {
			errno = 0;
			if (fflush(rp->fp)) {
				warning("%s flush of \"%s\" failed (%s).",
				    (rp->flag  & RED_PIPE) ? "pipe" :
				    "file", rp->value, strerror(errno));
				status++;
			}
		}
	return status;
}

int
close_io ()
{
	register struct redirect *rp;
	register struct redirect *next;
	int status = 0;

	for (rp = red_head; rp != NULL; rp = next) {
		next = rp->next;
		if (close_redir(rp))
			status++;
		rp = NULL;
	}
	return status;
}

/* devopen --- handle /dev/std{in,out,err}, /dev/fd/N, regular files */

int
devopen (name, mode)
char *name, *mode;
{
	int openfd = INVALID_HANDLE;
	FILE *fdopen ();
	char *cp, *ptr;
	int flag = 0;
	struct stat buf;

	switch(mode[0]) {
	case 'r':
		flag = O_RDONLY;
		break;

	case 'w':
		flag = O_WRONLY|O_CREAT|O_TRUNC;
		break;

	case 'a':
		flag = O_WRONLY|O_APPEND|O_CREAT;
		break;
	default:
		cant_happen();
	}

#ifdef VMS
	if ((openfd = vms_devopen(name)) >= 0)
		return openfd;
# define strcmp  strcasecmp	/* VMS filenames are not case sensitive; */
# define strncmp strncasecmp	/*  strncmp() is used by STREQN() below. */
#endif /*VMS*/

	if (STREQ(name, "-"))
		openfd = fileno(stdin);
	else if (STREQN(name, "/dev/", 5) && stat(name, &buf) == -1) {
		cp = name + 5;
		
		/* XXX - first three tests ignore mode */
		if (STREQ(cp, "stdin") && (flag & O_RDONLY) == O_RDONLY)
			openfd = fileno(stdin);
		else if (STREQ(cp, "stdout") && (flag & O_WRONLY) == O_WRONLY)
			openfd = fileno(stdout);
		else if (STREQ(cp, "stderr") && (flag & O_WRONLY) == O_WRONLY)
			openfd = fileno(stderr);
		else if (STREQN(cp, "fd/", 3)) {
			cp += 3;
			openfd = strtol(cp, &ptr, 10);
			if (openfd <= INVALID_HANDLE || ptr == cp)
				openfd = INVALID_HANDLE;
#ifdef VMS
		} else if (STREQ(cp, "null")) {
			name = "NL:";	/* "/dev/null" => "NL:" */
		} else if (STREQ(cp, "tty")) {
			name = "TT:";	/* "/dev/tty" => "TT:" */
# undef strcmp
# undef strncmp
#endif /*VMS*/
		}
	}

	if (openfd != INVALID_HANDLE)
		return openfd;
	else
		return checkopen(name, flag, 0666);
}

/* checkopen --- make sure a file is really a file */

static int
checkopen(file, flag, mode)
char *file;
int flag, mode;
{
	struct stat buf;

	if (stat(file, & buf) == 0) {
		if ((buf.st_mode & S_IFMT) == S_IFDIR)
			fatal("data file `%s' is a directory", file);
	} else if ((flag & O_CREAT) == 0)
		fatal("%s: could not stat: %s", file, strerror(errno));

	return(open(file, flag, mode));
}

#if defined(MSDOS) || defined(atarist)
#define PIPES_SIMULATED
#endif

#ifndef PIPES_SIMULATED
	/* real pipes */
static int
wait_any(interesting)
int interesting;	/* pid of interest, if any */
{
	SIGTYPE (*hstat)(), (*istat)(), (*qstat)();
	int pid;
	int status = 0;
	struct redirect *redp;
	extern int errno;

	hstat = signal(SIGHUP, SIG_IGN);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	for (;;) {
		pid = wait(&status);
		if (interesting && pid == interesting) {
			break;
		} else if (pid != -1) {
			for (redp = red_head; redp != NULL; redp = redp->next)
				if (pid == redp->pid) {
					redp->pid = -1;
					redp->status = status;
					if (redp->fp) {
						pclose(redp->fp);
						redp->fp = 0;
					}
					if (redp->iop) {
						(void) iop_close(redp->iop);
						redp->iop = 0;
					}
					break;
				}
		}
		if (pid == -1 && errno == ECHILD)
			break;
	}
	signal(SIGHUP, hstat);
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}

static IOBUF *
gawk_popen(cmd, rp)
char *cmd;
struct redirect *rp;
{
	int p[2];
	register int pid;

	(void) wait_any(0);	/* wait for outstanding processes */
	if (pipe(p) < 0)
		fatal("cannot open pipe \"%s\" (%s)", cmd, strerror(errno));
	if ((pid = fork()) == 0) {
		if (close(1) == -1)
			fatal("close of stdout in child failed (%s)",
				strerror(errno));
		if (dup(p[1]) != 1)
			fatal("dup of pipe failed (%s)", strerror(errno));
		if (close(p[0]) == -1 || close(p[1]) == -1)
			fatal("close of pipe failed (%s)", strerror(errno));
		execl("/bin/sh", "sh", "-c", cmd, 0);
		_exit(127);
	}
	if (pid == -1)
		fatal("cannot fork for \"%s\" (%s)", cmd, strerror(errno));
	rp->pid = pid;
	if (close(p[1]) == -1)
		fatal("close of pipe failed (%s)", strerror(errno));
	return (rp->iop = iop_alloc(p[0]));
}

static int
gawk_pclose(rp)
struct redirect *rp;
{
	(void) iop_close(rp->iop);
	rp->iop = NULL;

	/* process previously found, return stored status */
	if (rp->pid == -1)
		return (rp->status >> 8) & 0xFF;
	rp->status = wait_any(rp->pid);
	rp->pid = -1;
	return (rp->status >> 8) & 0xFF;
}

#else	/* PIPES_SUMULATED */
	/* use temporary file rather than pipe */

#ifdef VMS
static IOBUF *
gawk_popen(cmd, rp)
char *cmd;
struct redirect *rp;
{
	FILE *current;

	if ((current = popen(cmd, "r")) == NULL)
		return NULL;
	return (rp->iop = iop_alloc(fileno(current)));
}

static int
gawk_pclose(rp)
struct redirect *rp;
{
	int rval, aval, fd = rp->iop->fd;
	FILE *kludge = fdopen(fd, "r"); /* pclose needs FILE* w/ right fileno */

	rp->iop->fd = dup(fd);	  /* kludge to allow close() + pclose() */
	rval = iop_close(rp->iop);
	aval = pclose(kludge);
	return (rval < 0 ? rval : aval);
}
#else	/* VMS */

static
struct {
	char *command;
	char *name;
} pipes[_NFILE];

static IOBUF *
gawk_popen(cmd, rp)
char *cmd;
struct redirect *rp;
{
	extern char *strdup(const char *);
	int current;
	char *name;
	static char cmdbuf[256];

	/* get a name to use.  */
	if ((name = tempnam(".", "pip")) == NULL)
		return NULL;
	sprintf(cmdbuf,"%s > %s", cmd, name);
	system(cmdbuf);
	if ((current = open(name,O_RDONLY)) == INVALID_HANDLE)
		return NULL;
	pipes[current].name = name;
	pipes[current].command = strdup(cmd);
	return (rp->iop = iop_alloc(current));
}

static int
gawk_pclose(rp)
struct redirect *rp;
{
	int cur = rp->iop->fd;
	int rval;

	rval = iop_close(rp->iop);
	rp->iop = NULL;

	/* check for an open file  */
	if (pipes[cur].name == NULL)
		return -1;
	unlink(pipes[cur].name);
	free(pipes[cur].name);
	pipes[cur].name = NULL;
	free(pipes[cur].command);
	return rval;
}
#endif	/* VMS */

#endif	/* PIPES_SUMULATED */

NODE *
do_getline(tree)
NODE *tree;
{
	struct redirect *rp = NULL;
	IOBUF *iop;
	int cnt = EOF;
	char *s = NULL;

	while (cnt == EOF) {
		if (tree->rnode == NULL) {	 /* no redirection */
			iop = nextfile(0);
			if (iop == NULL)		/* end of input */
				return tmp_number((AWKNUM) 0.0);
		} else {
			int redir_error = 0;

			rp = redirect(tree->rnode, &redir_error);
			if (rp == NULL && redir_error)	/* failed redirect */
				return tmp_number((AWKNUM) -1.0);
			iop = rp->iop;
		}
		cnt = get_a_record(&s, iop, *RS);
		if (cnt == EOF) {
			if (rp) {
				(void) iop_close(iop);
				rp->iop = NULL;
				return tmp_number((AWKNUM) 0.0);
			} else
				continue;	/* try another file */
		}
		if (!rp) {
			NR += 1;
			FNR += 1;
		}
		if (tree->lnode == NULL)	/* no optional var. */
			set_record(s, cnt, 1);
		else {			/* assignment to variable */
			Func_ptr after_assign = NULL;
			NODE **lhs;

			lhs = get_lhs(tree->lnode, &after_assign);
			unref(*lhs);
			*lhs = make_string(s, strlen(s));
			/* we may have to regenerate $0 here! */
			if (after_assign)
				(*after_assign)();
		}
	}
	return tmp_number((AWKNUM) 1.0);
}

int
pathopen (file)
char *file;
{
	int fd = do_pathopen(file);

#ifdef DEFAULT_FILETYPE
	if (!strict && fd <= INVALID_HANDLE) {
		char *file_awk;
		int save = errno;
#ifdef VMS
		int vms_save = vaxc$errno;
#endif

		/* append ".awk" and try again */
		emalloc(file_awk, char *, strlen(file) +
			sizeof(DEFAULT_FILETYPE) + 1, "pathopen");
		strcat(strcpy(file_awk, file), DEFAULT_FILETYPE);
		fd = do_pathopen(file_awk);
		free(file_awk);
		if (fd <= INVALID_HANDLE) {
			errno = save;
#ifdef VMS
			vaxc$errno = vms_save;
#endif
		}
	}
#endif	/*DEFAULT_FILETYPE*/

	return fd;
}

static int
do_pathopen (file)
char *file;
{
	static char *savepath = DEFPATH;	/* defined in config.h */
	static int first = 1;
	char *awkpath, *cp;
	char trypath[BUFSIZ];
	int fd;

	if (STREQ(file, "-"))
		return (0);

	if (strict)
		return (checkopen (file, 0, 0));

	if (first) {
		first = 0;
		if ((awkpath = getenv ("AWKPATH")) != NULL && *awkpath)
			savepath = awkpath;	/* used for restarting */
	}
	awkpath = savepath;

	/* some kind of path name, no search */
#ifdef VMS	/* (strchr not equal implies either or both not NULL) */
	if (strchr(file, ':') != strchr(file, ']')
	 || strchr(file, '>') != strchr(file, '/'))
#else /*!VMS*/
#ifdef MSDOS
	if (strchr(file, '/') != strchr(file, '\\')
	 || strchr(file, ':') != NULL)
#else
	if (strchr(file, '/') != NULL)
#endif	/*MSDOS*/
#endif	/*VMS*/
		return (devopen (file, "r"));

	do {
		trypath[0] = '\0';
		/* this should take into account limits on size of trypath */
		for (cp = trypath; *awkpath && *awkpath != ENVSEP; )
			*cp++ = *awkpath++;

		if (cp != trypath) {	/* nun-null element in path */
			/* add directory punctuation only if needed */
#ifdef VMS
			if (strchr(":]>/", *(cp-1)) == NULL)
#else
#ifdef MSDOS
			if (strchr(":\\/", *(cp-1)) == NULL)
#else
			if (*(cp-1) != '/')
#endif
#endif
				*cp++ = '/';
			/* append filename */
			strcpy (cp, file);
		} else
			strcpy (trypath, file);
		if ((fd = devopen (trypath, "r")) >= 0)
			return (fd);

		/* no luck, keep going */
		if(*awkpath == ENVSEP && awkpath[1] != '\0')
			awkpath++;	/* skip colon */
	} while (*awkpath);
	/*
	 * You might have one of the awk
	 * paths defined, WITHOUT the current working directory in it.
	 * Therefore try to open the file in the current directory.
	 */
	return (devopen(file, "r"));
}
