#ifndef	lint
static	char	what[] = "$Id: dlog.c,v 11.24 1992/08/26 11:58:30 dickey Exp $";
#endif

/*
 * Title:	dlog.c
 * Author:	T.E.Dickey
 * Created:	14 Mar 1989
 * Modified:
 *		06 Aug 1992, added \F, \B, \W, \? escape decoding.
 *		18 Oct 1991, converted to ANSI
 *		09 Sep 1991, lint
 *		06 Mar 1990, 'cmdch()' can now return explicit zero-count
 *		11 Aug 1989, wrapped some code around the call on 'cmdch()' to
 *			     try to recover from I/O errors.
 *		24 Mar 1989, fixed bugs in command-script found in regression
 *			     tests.  Added 'dlog_read()' and local code to
 *			     support command-script.  also, some lint.
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

#define	NOW		time((time_t *)0)

/* state of command-file (input) */
static	FILE	*cmd_fp;
static	DYN	*cmd_bfr;		/* most recently-read buffer */
static	char	*cmd_ptr;		/* points into cmd_bfr */

/* state of log-file (output) */
static	FILE	*log_fp;
static	char	log_name[MAXPATHLEN];
static	time_t	mark_time;
static	DYN	*pending;		/* buffers parts of raw-commands */

private	void	show_time _ONE(char *,msg)
{
	auto	time_t	now = NOW;

	dlog_comment("process %d %s at %s", getpid(), msg, ctime(&now));
}

#ifdef	DEBUG
private	void	show_text _ONE(char *,text)
{
	if (log_fp) {
		char	temp_t[BUFSIZ],
			*text2s	= temp_t;

		while (*text) {
			encode_logch(text2s, (int *)0, *text++);
			text2s += strlen(text2s);
		}
		*text2s = EOS;
		FPRINTF(log_fp, "%s\n", temp_t);
	}
}
#endif	/* DEBUG */

/*
 * Force a newline on the end of the given string, used to finish an edit-
 * command in 'wrawgets()'.
 */
private	void	supply_newline _ONE(DYN **,p)
{
	register int	len;

	if (len = dyn_length(*p))
		if (dyn_string(*p)[len-1] == '\n')
			return;
	*p = dyn_append_c(*p, '\n');
}

/*
 * Flush pending text from raw commands so that it is logged without a
 * newline embedded.  We suppress newline after flush if we call this from
 * the string-read, to make the entire command appear on a single line.
 *
 * Keep 's' argument in PENDING for debugging aid.
 */
#define	PENDING(s,flag)	flush_pending(flag)

private	void	flush_pending _ONE(int,newline)
{
	if (log_fp) {
		register char	*s = dyn_string(pending);
		if (s != 0 && *s != EOS) {
			FPRINTF(log_fp, "%s%s", s, newline ? "\n" : "");
			dyn_init(&pending, 1);
		}
	}
}

/*
 * See if we have a command-file open, and if so, whether we have exhausted
 * the current buffer.  If so, read the next buffer from the command-file.
 * return TRUE if we have command-file text available.
 */
private	int	read_script(_AR0)
{
	if (cmd_fp != 0) {
		register char	*s;
		int	join	= FALSE;
		char	temp[BUFSIZ];

		if (cmd_ptr == 0 || !*cmd_ptr) {
			dyn_init(&cmd_bfr, BUFSIZ);
			cmd_ptr = dyn_string(cmd_bfr);
		}
		while (	!*cmd_ptr || join) {
			if (fgets(temp, sizeof(temp), cmd_fp)) {
				for (s = temp, join = FALSE; *s; s++)
					if (!isprint(*s)) {
						join = (*s == '\t');
						*s = EOS;
						break;
					}

				cmd_bfr = dyn_append(cmd_bfr, temp);
				cmd_ptr = dyn_string(cmd_bfr);
			} else {
				FCLOSE(cmd_fp);
				cmd_fp = 0;	/* forced-close */
				break;
			}
		}
		return *cmd_ptr;	/* have text to process */
	}
	return (FALSE);			/* no command-script to read */
}

/*
 * Read a single-letter command from either the command-file (if open), or
 * from the keyboard.
 */
private	int	read_char _ONE(int *,count_)
{
	auto	int	num;
	register int	j;

	if (read_script())
		return decode_logch(&cmd_ptr, count_);

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

private	int	record_char(
	_ARX(int,	c)
	_ARX(int *,	count_)
	_AR1(int,	begin)
		)
	_DCL(int,	c)
	_DCL(int *,	count_)
	_DCL(int,	begin)
{
	if (log_fp) {
		register char	*s;
		if (begin) {
			dlog_flush();
			mark_time = NOW;
		}
		pending = dyn_alloc(pending, dyn_length(pending)+20);
		s = dyn_string(pending) + dyn_length(pending);
		encode_logch(s, count_, c);
		pending->cur_length += strlen(s);
	}
	return c;
}

private	void	record_string _ONE(char *,s)
{
	register int	c;

	while (c = *s++)
		(void)record_char(c & 0xff, (int *)0, FALSE);
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * Specify a command-script from which to read.  Note that command scripts
 * work only within a single process, though logging is performed on multiple
 * processes.
 */
public	void	dlog_read _ONE(char *,name)
{
	if (!(cmd_fp = fopen(name, "r")))
		failed(name);
	dyn_init(&cmd_bfr, BUFSIZ);
	cmd_ptr = dyn_string(cmd_bfr);
}

/*
 * Open/append to log-file
 */
public	char *	dlog_open(
	_ARX(char *,	name)
	_ARX(int,	argc)
	_AR1(char **,	argv)
		)
	_DCL(char *,	name)
	_DCL(int,	argc)
	_DCL(char **,	argv)
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

public	void	dlog_reopen(_AR0)
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
public	void	dlog_close(_AR0)
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
public	void	dlog_exit _ONE(int,code)
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
public	int	dlog_char(
	_ARX(int *,	count_)
	_AR1(int,	begin)
		)
	_DCL(int *,	count_)
	_DCL(int,	begin)
{
	return record_char(read_char(count_), count_, begin);
}

#define	IGNORE	{ beep(); if (inline) goto ResetResult; else continue; }

/*
 * Obtain a string from the user and log it if logging is active.
 *
 * Note: all line-buffer text in DED is written after one or more single-
 * character commands, so we can test easily for the case of a null-buffer (it
 * simply has no more characters in the current buffer).
 *
 * Note: we don't log the up/down arrows used for history retrieval for
 * simplification.  Ultimately, this could be done.
 */
public	char *	dlog_string(
	_ARX(DYN **,	result)
	_ARX(DYN **,	inline)
	_ARX(HIST **,	history)
	_ARX(int,	fast_q)
	_AR1(int,	wrap_len)
		)
	_DCL(DYN **,	result)
	_DCL(DYN **,	inline)
	_DCL(HIST **,	history)
	_DCL(int,	fast_q)
	_DCL(int,	wrap_len)
{
	static	DYN	*original, *before, *after, *trace, *edited;
	static	char	*i_pref[] = { "^",  " "  },
			*n_pref[] = { "^ ", ": " };

	int	done;
	int	wrap	= wrap_len <= 0;
	int	len	= wrap ? -wrap_len : wrap_len;
	int	nnn	= 0;	/* history-index */
	int	y,x;
	char	*buffer, *to_hist, *prior;
	char	**prefix= inline ? i_pref : n_pref;

	register int	c;
	register char	*s;

	/* make sure we have enough space to write result; don't delete it! */
	if (!len)
		len = BUFSIZ;
	*result = dyn_alloc(*result, (size_t)len+1);
	buffer  = dyn_string(*result);


	/* Inline-editing records the editing actions in history, not the
	 * resulting buffer.
	 */
	if (inline) {
		original = dyn_copy(original, buffer);
		if (s = dyn_string(*inline))
			trace = dyn_copy(trace, s);
		else
			dyn_init(&trace,1);
#ifdef	DEBUG
		dlog_comment("INLINE:");
		show_text(dyn_string(trace));
#endif
	}

	getyx(stdscr,y,x);

	for (;;) {
		int	first_col,	/* column at which to begin edit */
			first_ins;	/* set true to initially insert */

		/*
		 * Replay the portion of the inline editing from history.
		 */
		if (inline) {
			if ((s = dyn_string(trace))
			 && (*s != EOS)) {
				*inline = dyn_copy(*inline, s);
				supply_newline(&trace);
				prior = dyn_string(trace);
				(void)wrawgets(stdscr,
					buffer,
					prefix,
					len,
					len,
					0,
					(fast_q == EOS),
					wrap,
					fast_q,
					&prior,
					TRUE);
#ifdef	DEBUG
				dlog_comment("PLAY  :");
				show_text(rawgets_log());
#endif
				first_col = strlen(buffer);
				first_ins = TRUE;
			} else {
				dyn_init(inline,1);
				first_col = 0;
				first_ins = FALSE;
			}
		} else {
			first_col = strlen(buffer);
			first_ins = (fast_q == EOS);
		}

		/*
		 * Perform interactive (or scripted) edit:
		 */
		before = dyn_copy(before, buffer);
		after  = dyn_copy(after,  buffer);

		move(y,x);
		c = wrawgets(stdscr,
			buffer,
			prefix,
			len,
			len + 1,
			first_col,
			first_ins,
			wrap,
			fast_q,
			cmd_fp ? &cmd_ptr : (char **)0,
			history || inline || log_fp);

		/*
		 * Record the inline-editing keystrokes
		 */
		if (inline) {
#ifdef	DEBUG
			dlog_comment("BEFORE:");
			show_text(dyn_string(trace));
			dlog_comment("EDITED:");
			show_text(rawgets_log());
#endif
			(void)dyn_trim1(trace);
			trace = dyn_append(trace, rawgets_log());
			(void)dyn_trim1(trace);
#ifdef	DEBUG
			dlog_comment("AFTER :");
			show_text(dyn_string(trace));
#endif
		}

		to_hist = inline ? dyn_string(trace) : buffer;

		/* If any change was made, reset history-index */
		if (strcmp(dyn_string(before), dyn_string(after))) {
			nnn = 0;
			after  = dyn_copy(after,  buffer);
			edited = dyn_copy(edited, to_hist);
		}

		if (log_fp)
			record_string(rawgets_log());

		if (c == ARO_UP) {
			if (!history) IGNORE

			if (!nnn)
				edited = dyn_copy(edited, to_hist);

			if (s = get_history(*history, nnn)) {
				if (strcmp(s, to_hist))
					;	/* cannot skip */
				else if (s = get_history(*history, nnn+1))
					nnn++;
				else IGNORE	/* last and only item */
				nnn++;
			} else IGNORE

		} else if (c == ARO_DOWN) {
			if (!history) IGNORE

			if (s = get_history(*history, nnn-2)) {
				nnn--;
				if (nnn == 1 && !strcmp(s, dyn_string(edited)))
					nnn = 0;
			} else if (nnn == 1) {
				nnn = 0;
				s = dyn_string(edited);
			} else IGNORE

		} else if (c == '\n') {
			done = TRUE;
			break;
		} else {/* assume quit */
			done = -TRUE;
			break;
		}

		if (inline)
			trace = dyn_copy(trace, s);
ResetResult:
		*result = dyn_copy(*result, inline ? dyn_string(original) : s);
		buffer = dyn_string(*result);
	}

	if (inline) {
		supply_newline(&trace);
		*inline = dyn_copy(*inline, to_hist);
	}

	PENDING(string,TRUE);

#ifdef	DEBUG
	dlog_comment("done:%d:", done);
	show_text(to_hist);
#endif
	if (done == TRUE) {
		if (!inline)
			put_history(history, to_hist);
		return buffer;
	}
	return 0;	/* if quit, return null */
}

/*
 * Log elapsed time since the beginning of a command
 */
public	void	dlog_elapsed(_AR0)
{
	if (log_fp) {
		dlog_comment("elapsed time = %ld seconds\n", NOW - mark_time);
	}
}

/*
 * Flush the pending command.  We buffer commands so that multi-character
 * stuff (such as sort) is written on one line.
 */
public	void	dlog_flush(_AR0)
{
	if (log_fp) {
		PENDING(flush,TRUE);
		(void)fflush(log_fp);
	}
}

/*
 * Annotate the given command with the name of the current entry
 */
public	void	dlog_name _ONE(char *,name)
{
	dlog_comment("\"%s\"\n", name);
}

/*
 * Write a comment to the log-file (with trailing newline in 'fmt').
 */
#ifdef	lint
#undef	va_dcl
#define	va_dcl		char	*va_alist;
#undef	va_start
#define	va_start(args)	args = va_alist
#undef	va_arg
#define	va_arg(p,c)	(c)0
#endif

/*VARARGS*/
public	void	dlog_comment(va_alist)
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
#else	/* VFPRINTF */
		(void)vfprintf(log_fp, fmt, args);
#endif	/* !VFPRINTF/VFPRINTF */
		va_end(args);
	}
}
