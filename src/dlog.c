#ifndef	lint
static	char	what[] = "$Id: dlog.c,v 5.0 1989/08/11 14:22:26 ste_cm Rel $";
#endif	lint

/*
 * Title:	dlog.c
 * Author:	T.E.Dickey
 * Created:	14 Mar 1989
 * $Log: dlog.c,v $
 * Revision 5.0  1989/08/11 14:22:26  ste_cm
 * BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *
 *		Revision 4.0  89/08/11  14:22:26  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.2  89/08/11  14:22:26  dickey
 *		lint
 *		
 *		Revision 3.1  89/08/11  10:42:32  dickey
 *		wrapped some code around the call on 'cmdch()' to try to
 *		recover from I/O errors.
 *		
 *		Revision 3.0  89/03/24  10:43:36  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/24  10:43:36  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.6  89/03/24  10:43:36  dickey
 *		fixed bugs in command-script found in regression tests
 *		
 *		Revision 1.5  89/03/24  08:39:26  dickey
 *		added 'dlog_read()' and local code to support
 *		command-script.  also, some lint.
 *		
 *		Revision 1.4  89/03/15  11:34:19  dickey
 *		sccs2rcs keywords
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
#define	CONVERT(base,p,n)	n = (base * n) + (*p++ - '0')

/* state of command-file (input) */
static	FILE	*cmd_fp;
static	char	cmd_bfr[BUFSIZ],	/* most recently-read buffer */
		*cmd_ptr;		/* points into cmd_bfr */

/* state of log-file (output) */
static	FILE	*log_fp;
static	char	log_name[BUFSIZ];
static	time_t	mark_time;
static	char	pending[BUFSIZ];	/* buffers parts of raw-commands */

static
show_time(msg)
char	*msg;
{
	auto	time_t	now = NOW;

	dlog_comment("process %d %s at %s", getpid(), msg, ctime(&now));
}

/*
 * Flush pending text from raw commands so that it is logged without a
 * newline embedded.  We suppress newline after flush if we call this from
 * the string-read, to make the entire command appear on a single line.
 *
 * Keep 's' argument in PENDING for debugging aid.
 */
#define	PENDING(s,flag)	flush_pending(flag)

static
flush_pending(newline)
{
	if (*pending) {
		FPRINTF(log_fp, "%s%s", pending, newline ? "\n" : "");
		*pending = EOS;
	}
}

/*
 * See if we have a command-file open, and if so, whether we have exhausted
 * the current buffer.  If so, read the next buffer from the command-file.
 * return TRUE if we have command-file text available.
 */
static
read_script()
{
	register char	*s;

	if (cmd_fp != 0) {
		while (!*cmd_bfr || (cmd_ptr != 0 && !*cmd_ptr)) {
			if (fgets(cmd_bfr, sizeof(cmd_bfr), cmd_fp)) {
				cmd_ptr = cmd_bfr;
				for (s = cmd_bfr; *s; s++)
					if (!isprint(*s)) {
						*s = EOS;
						break;
					}
			} else {
				FCLOSE(cmd_fp);
				cmd_fp = 0;
				return (FALSE);	/* forced-close */
			}
		}
		return (TRUE);	/* have text to process */
	}
	return (FALSE);		/* no command-script to read */
}

/*
 * Read a single-letter command from either the command-file (if open), or
 * from the keyboard.
 */
static
read_char(count_)
int	*count_;
{
	auto	int	num;
	register int	j;

	if (read_script()) {
		if (count_) {
			num = 0;
			while (isdigit(*cmd_ptr)) {
				CONVERT(10,cmd_ptr,num);
			}
			*count_ = num;
		}
		if (*cmd_ptr == '\\') {
			switch (*(++cmd_ptr)) {
			case '\\':	num = '\\';		break;
			case 'b':	num = '\b';		break;
			case 'f':	num = '\f';		break;
			case 'n':	num = '\n';		break;
			case 'r':	num = '\r';		break;
			case 't':	num = '\t';		break;
			case 'v':	num = '\v';		break;
			case 's':	num = ' ';		break;
			case 'E':	num = '\033';		break;
			case 'U':	num = ARO_UP;		break;
			case 'D':	num = ARO_DOWN;		break;
			case 'L':	num = ARO_LEFT;		break;
			case 'R':	num = ARO_RIGHT;	break;
			default:
				if (isdigit(*cmd_ptr)) {
					num = 0;
					for (j = 0; j < 3; j++) {
						if (isdigit(*cmd_ptr))
							CONVERT(8,cmd_ptr,num);
						else
							break;	/* error? */
					}
					cmd_ptr--; /* point to last digit */
				} else {
					return (*(--cmd_ptr));	/* recover */
				}
			}
			cmd_ptr++;	/* point past decoded char */
			return (num);
		} else
			return (*cmd_ptr++);
	}

#define	MAXTRIES	5
	/*
	 * If we get an EOF (or NULL) on input, wait and retry.  On apollo, at
	 * least, sometimes the terminal emulator dies and simply gives me
	 * nulls.
	 */
	for (j = 0; j < MAXTRIES; j++) {
		if ((num = cmdch(count_)) > 0)
			return (num);
		if (j == 0)	beep();
		sleep(2);
	}
	failed("read_char");
	/*NOTREACHED*/
}

/*
 * Read a line-buffer from the command-file (if open), or from the keyboard
 * if not.  Note that all line-buffer text in DED is written after one or
 * more single-character commands, so we can test easily for the case of
 * a null-buffer (it simply has no more characters in the current buffer).
 */
static
read_line(s,len,wrap)
char	*s;
{
	if (cmd_fp != 0) {
		while (*cmd_ptr) {
			*s = *cmd_ptr++;
			if (len-- > 0)
				s++;
		}
		*s = EOS;
		return;
	}
	rawgets(s, len, wrap);
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * Specify a command-script from which to read.  Note that command scripts
 * work only within a single process, though logging is performed on multiple
 * processes.
 */
dlog_read(name)
char	*name;
{
	if (!(cmd_fp = fopen(name, "r")))
		failed(name);
	*(cmd_ptr = cmd_bfr) = EOS;
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

	c = read_char(count_);
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
	read_line(s, len, wrap);
	if (log_fp) {
		PENDING(string,FALSE);
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
	if (log_fp) {
		PENDING(flush,TRUE);
		(void)fflush(log_fp);
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
#define	va_dcl		char	*va_alist;
#undef	va_arg
#define	va_arg(p,c)	(c)0
#endif	lint

/*VARARGS*/
dlog_comment(va_alist)
va_dcl
{
	auto	va_list	args;
	auto	char	*fmt;

	if (log_fp) {
		PENDING(comment,FALSE);
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
