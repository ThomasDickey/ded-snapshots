#ifndef	lint
static	char	sccs_id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/dlog.c,v 1.4 1989/03/15 11:34:19 dickey Exp $";
#endif	lint

/*
 * Title:	dlog.c
 * Author:	T.E.Dickey
 * Created:	14 Mar 1989
 * $Log: dlog.c,v $
 * Revision 1.4  1989/03/15 11:34:19  dickey
 * sccs2rcs keywords
 *
 *		15 Mar 1989, added 'dlog_exit()', mods to make this work with
 *			     subprocesses.
 *
 * Function:	Writes a log-file for 'ded' in consistent format so that the
 *		session can easily be analyzed.
 *
 *		Special characters are translated to printing form.
 *		Whitespace is used for formatting only (except where it appears
 *		in a command-string).
 *
 *		Comments are written with a tab followed by '#'.
 */

#include	"ded.h"
#include	<time.h>
#include	<varargs.h>
extern	time_t	time();

#define	NOW	time((time_t *)0)


static	FILE	*log_fp;
static	char	log_name[BUFSIZ];
static	time_t	mark_time;
static	char	pending[BUFSIZ];

static
show_time(msg)
char	*msg;
{
	auto	time_t	now = NOW;

	dlog_comment("process %d %s at %s", getpid(), msg, ctime(&now));
}

/*
 * Open/append to log-file
 */
char *
dlog_open(name, argc, argv)
char	*name;
char	*argv[];
{
	register int	j;

	if (name != 0 && *name != EOS) {
		if (!(log_fp = fopen(name, "a+")))
			failed(name);

		abspath(strcpy(log_name, name));
		show_time("begun");
		for (j = 0; j < argc; j++)
			dlog_comment("argv[%d] = '%s'\n", j, argv[j]);
		return (log_name);
	}
	return ((char *)0);
}

dlog_reopen()
{
	if (*log_name) {
		if (log_fp = fopen(log_name, "a+"))
			dlog_comment("process %d resuming\n", getpid());
	}
}

/*
 * Close the log-file (i.e., while spawning a subprocess which will append
 * to the log).
 */
dlog_close()
{
	if (log_fp) {
		dlog_flush();
		FCLOSE(log_fp);
		log_fp = 0;
	}
}

/*
 * Exit from the current process, marking the final time on the log-file
 */
dlog_exit(code)
{
	if (log_fp) {
		show_time("ended");
		dlog_close();
	}
	(void)exit(code);
}

/*
 * Read a single-character command, logging it appropriately.
 */
dlog_char(count_,begin)
int	*count_;
{
	register int	c;
	register char	*s;

	c = cmdch(count_);
	if (log_fp) {
		if (begin) {
			dlog_flush();
			mark_time = NOW;
		}
		s = pending + strlen(pending);
		if (count_ && *count_) {
			FORMAT(s, "%d", *count_);
			s += strlen(s);
		}
		if (c == '\\')
			FORMAT(s, "\\\\");
		else if (isascii(c) && isgraph(c))
			FORMAT(s, "%c", c);
		else switch (c) {
			case '\b':	FORMAT(s, "\\b");	break;
			case '\f':	FORMAT(s, "\\f");	break;
			case '\n':	FORMAT(s, "\\n");	break;
			case '\r':	FORMAT(s, "\\r");	break;
			case '\t':	FORMAT(s, "\\t");	break;
			case '\v':	FORMAT(s, "\\v");	break;
			case ' ':	FORMAT(s, "\\s");	break;
			case '\033':	FORMAT(s, "\\E");	break;
			case ARO_UP:	FORMAT(s, "\\U");	break;
			case ARO_DOWN:	FORMAT(s, "\\D");	break;
			case ARO_LEFT:	FORMAT(s, "\\L");	break;
			case ARO_RIGHT:	FORMAT(s, "\\R");	break;
			default:	FORMAT(s, "\\%03o", c);
		}
	}
	return (c);
}

/*
 * Obtain a string from the user and log it if logging is active.
 */
dlog_string(s,len,wrap)
char	*s;
{
	rawgets(s, len, wrap);
	if (log_fp) {
		if (*pending) {
			FPRINTF(log_fp, "%s", pending);
			*pending = EOS;
		}
		FPRINTF(log_fp, "%s\n", s);
	}
}

/*
 * Log elapsed time since the beginning of a command
 */
dlog_elapsed()
{
	if (log_fp) {
		dlog_comment("elapsed time = %ld seconds\n", NOW - mark_time);
	}
}

/*
 * Flush the pending command.  We buffer commands so that multi-character
 * stuff (such as sort) is written on one line.
 */
dlog_flush()
{
	if (log_fp)
		if (*pending) {
			FPRINTF(log_fp, "%s\n", pending);
			(void)fflush(log_fp);
			*pending = EOS;
		}
}

/*
 * Annotate the given command with the name of the current entry
 */
dlog_name(name)
char	*name;
{
	dlog_comment("\"%s\"\n", name);
}

/*
 * Write a comment to the log-file (with trailing newline in 'fmt').
 */
#ifdef	lint
#undef	va_dcl
#define	va_dcl	char	*va_alist;
#endif	lint

/*VARARGS*/
dlog_comment(va_alist)
va_dcl
{
	auto	va_list	args;
	auto	char	*fmt;

	if (log_fp) {
		if (*pending) {
			FPRINTF(log_fp, "%s", pending);
			*pending = EOS;
		}
		FPRINTF(log_fp, "\t# ");
		va_start(args);
		fmt = va_arg(args, char *);
#if	defined(gould) || defined(GOULD_PN)
		{
#define	MAXARG	10
			register int	n = nargs(),
					j = 1;
			auto	 long	v[MAXARG];

			while (j < n)
				v[j++] = va_arg(args, long);
			FPRINTF(log_fp, fmt, v[1], v[2], v[3], v[4]);
		}
#else	VFPRINTF
		(void)vfprintf(log_fp, fmt, args);
#endif	VFPRINTF
		va_end(args);
	}
}
