#ifndef	lint
static	char	sccs_id[] = "@(#)dlog.c	1.1 89/03/14 13:00:02";
#endif	lint

/*
 * Title:	dlog.c
 * Author:	T.E.Dickey
 * Created:	14 Mar 1989
 * Modified:
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
static	time_t	mark_time;
static	char	pending[BUFSIZ];

dlog_open(name, argc, argv)
char	*name;
char	*argv[];
{
	auto	time_t	now = NOW;
	register int	j;

	if (!(log_fp = fopen(name, "a+")))
		failed(name);

	dlog_comment("session begun at %s", ctime(&now));
	for (j = 0; j < argc; j++)
		dlog_comment("argv[%d] = '%s'\n", j, argv[j]);
	*pending = EOS;
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
			fprintf(log_fp, "%s", pending);
			*pending = EOS;
		}
		fprintf(log_fp, "%s\n", s);
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
			fprintf(log_fp, "%s\n", pending);
			fflush(log_fp);
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
/*VARARGS*/
dlog_comment(va_alist)
va_dcl
{
	auto	va_list	args;
	auto	char	*fmt;

	if (log_fp) {
		if (*pending) {
			fprintf(log_fp, "%s", pending);
			*pending = EOS;
		}
		(void)fprintf(log_fp, "\t# ");
		va_start(args);
		fmt = va_arg(args, char *);
		(void)vfprintf(log_fp, fmt, args);
		va_end(args);
	}
}
