/*
 * Title:	dlog.c
 * Author:	T.E.Dickey
 * Created:	14 Mar 1989
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		27 Dec 1996, move ANSI_VARARGS ifdef to configure-script.
 *		01 Dec 1993, moved most 'refresh()' calls under 'dlog_char()'
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		28 Aug 1992, implemented inline history in 'dlog_string()'. 
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

MODULE_ID("$Id: dlog.c,v 12.20 2004/03/07 23:25:18 tom Exp $")

#define	NOW		time((time_t *)0)

/* state of command-file (input) */
static FILE *cmd_fp;
static DYN *cmd_bfr;		/* most recently-read buffer */
static char *cmd_ptr;		/* points into cmd_bfr */

/* state of log-file (output) */
static FILE *log_fp;
static DYN *log_name;
static time_t mark_time;
static DYN *pending;		/* buffers parts of raw-commands */

static void
show_time(char *msg)
{
    time_t now = NOW;

    dlog_comment("process %d %s at %s", getpid(), msg, ctime(&now));
}

/*
 * Find the last character (or escaped-character) in the given string.  Assumes
 * this is formatted by 'encode_logch()'.
 */
static char *
find_ending(char *s)
{
    char *mark = 0;

    while (s != 0 && *s != EOS) {
	mark = s;
	(void) decode_logch(&s, (int *) 0);
    }
    return mark;
}

/*
 * Trims the last "character" in the given edit-string, assumed to be one of
 * "\n", "\U", "\D", quit or the end-character.
 */
static void
trim_ending(DYN * p)
{
    char *s = find_ending(dyn_string(p));
    int len;

    if (s != 0) {
	len = strlen(s);
	while (len-- > 0)
	    (void) dyn_trim1(p);
    }
}

/*
 * Convert the newline-char on the end of the edit-string to an escape-sequence
 */
static void
convert_newline(DYN ** p)
{
    char *s = find_ending(dyn_string(*p));

    if (s != 0 && *s == '\n') {
	trim_ending(*p);
	*p = dyn_append(*p, "\\n");
    }
}

/*
 * Force a newline-char on the end of the given string, used to finish an edit-
 * command in 'wrawgets()'.
 */
static void
supply_newline(DYN ** p)
{
    char *s = find_ending(dyn_string(*p));
    if (s != 0 && strcmp(s, "\\n"))
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

static void
flush_pending(int new_line)
{
    if (log_fp) {
	char *s = dyn_string(pending);
	if (s != 0 && *s != EOS) {
	    FPRINTF(log_fp, "%s%s", s, new_line ? "\n" : "");
	    dyn_init(&pending, 1);
	}
    }
}

/*
 * See if we have a command-file open, and if so, whether we have exhausted
 * the current buffer.  If so, read the next buffer from the command-file.
 * return TRUE if we have command-file text available.
 */
static int
read_script(void)
{
    char *s;
    int join = FALSE;
    char temp[BUFSIZ];

    if (cmd_ptr == 0 || !*cmd_ptr) {
	dyn_init(&cmd_bfr, BUFSIZ);
	cmd_ptr = dyn_string(cmd_bfr);
    }
    while (!*cmd_ptr || join) {
	if (cmd_fp == 0)
	    break;
	else if (fgets(temp, sizeof(temp), cmd_fp)) {
	    join = TRUE;	/* ...unless we find newline */
	    for (s = temp; *s; s++)
		if (!isprint(UCH(*s))) {	/* tab or newline */
		    join = (*s == '\t');
		    *s = EOS;
		    break;
		}

	    cmd_bfr = dyn_append(cmd_bfr, temp);
	    cmd_ptr = dyn_string(cmd_bfr);
	} else {
	    FCLOSE(cmd_fp);
	    cmd_fp = 0;		/* forced-close */
	    break;
	}
    }
    return *cmd_ptr;		/* have text to process */
}

/*
 * Read a single-letter command from either the command-file (if open), or
 * from the keyboard.
 */
static int
read_char(int *count_)
{
    int num = 0;
    int j;

    refresh();
    if (read_script())
	num = decode_logch(&cmd_ptr, count_);

#define	MAXTRIES	5
    /*
     * If we get an EOF (or NULL) on input, wait and retry.  On apollo, at
     * least, sometimes the terminal emulator dies and simply gives me
     * nulls.
     */
    else {
	for (j = 0; j < MAXTRIES; j++) {
	    if ((num = cmdch(count_)) > 0)
		break;
	    if (j == 0)
		beep();
	    sleep(2);
	}
	if (num <= 0)
	    failed("read_char");
	/*NOTREACHED */
    }
    return num;
}

static int
record_char(int c, int *count_, int begin)
{
    if (log_fp) {
	char *s;
	if (begin) {
	    dlog_flush();
	    mark_time = NOW;
	}
	pending = dyn_alloc(pending, dyn_length(pending) + 20);
	s = dyn_string(pending) + dyn_length(pending);
	encode_logch(s, count_, c);
	pending->cur_length += strlen(s);
    }
    return c;
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * Specify a command-script from which to read.  Note that command scripts
 * work only within a single process, though logging is performed on multiple
 * processes.
 */
void
dlog_read(char *name)
{
    if (!(cmd_fp = fopen(name, "r")))
	failed(name);
    dyn_init(&cmd_bfr, BUFSIZ);
    cmd_ptr = dyn_string(cmd_bfr);
}

/*
 * Open/append to log-file
 */
char *
dlog_open(char *name, int argc, char **argv)
{
    int j;

    if (name != 0 && *name != EOS) {
	char temp[MAXPATHLEN];
	abspath(name = strcpy(temp, name));
	if (!(log_fp = fopen(name, "a+")))
	    failed(name);

	log_name = dyn_copy(log_name, name);

	show_time("begun");
	for (j = 0; j < argc; j++)
	    dlog_comment("argv[%d] = '%s'\n", j, argv[j]);
	return dyn_string(log_name);
    }
    return ((char *) 0);
}

void
dlog_reopen(void)
{
    if (dyn_string(log_name)) {
	if ((log_fp = fopen(dyn_string(log_name), "a+")) != NULL)
	    dlog_comment("process %d resuming\n", getpid());
    }
}

/*
 * Close the log-file (i.e., while spawning a subprocess which will append
 * to the log).
 */
void
dlog_close(void)
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
void
dlog_exit(int code)
{
    if (log_fp) {
	show_time("ended");
	dlog_close();
    }
    (void) exit(code);
}

/*
 * Read a single-character command, logging it appropriately.
 */
int
dlog_char(RING * gbl, int *count_, int begin)
{
    int c;
    dedsize(gbl);
    c = record_char(read_char(count_), count_, begin);
    dedsize((RING *) 0);
    return c;
}

/*
 * If we're given a nonnull prompt-string, clear the work-area and display
 * the prompt.
 */
void
dlog_prompt(RING * gbl, char *prompt, int row)
{
    int y, x;

    if (prompt != 0) {
	if (row >= 0) {
	    markC(gbl, TRUE);
	    move(row, 0);
	    clrtoeol();
	} else
	    to_work(gbl, TRUE);
	addstr(prompt);
	getyx(stdscr, y, x);
	clrtobot();
	move(y, x);
    }
}

/******************************************************************************/
#define	IGNORE	beep(), s = to_hist

/*
 * We need these variables cached to be able to reconstruct the prompt prefix
 * when resizing the window.
 */
static RING *gets_g_data;
static char *gets_prompt;
static int gets_row;

#ifdef	SIGWINCH
void
dlog_resize(void)
{
    dlog_prompt(gets_g_data, gets_prompt, gets_row);
    getyx(stdscr, y_rawgets, x_rawgets);
}
#endif

/*
 * Obtain a string from the user and log it if logging is active.
 *
 * Note: all line-buffer text in DED is written after one or more single-
 * character commands, so we can test easily for the case of a null-buffer (it
 * simply has no more characters in the current buffer).
 */
char *
dlog_string(RING * gbl,
	    char *prompt,	/* nonnull iff we're using work-area */
	    int row,		/* positive to specify row */
	    DYN ** result,
	    DYN ** inflag,
	    HIST ** history,
	    int fast_q,
	    int wrap_len)
{
    static DYN *original, *before, *after, *script_bfr, *edited;
    static char *i_pref[] =
    {"^", " "}, *n_pref[] =
    {"^ ", ": "};

    int done;
    int wrap = wrap_len <= 0;
    int len = wrap ? -wrap_len : wrap_len;
    int nnn = 0;		/* history-index */
    int script_inx, use_script;
    int y, x;
    char *buffer, *to_hist;
    char **prefix = inflag ? i_pref : n_pref;
    char *script_ptr;

    int c;
    char *s;

    /* make sure we have enough space to write result; don't delete it! */
    if (!len)
	len = BUFSIZ;
    *result = dyn_alloc(*result, (size_t) len + 1);
    buffer = dyn_string(*result);

    gets_g_data = gbl;
    dlog_prompt(gbl, gets_prompt = prompt, gets_row = row);

    /* Inline-editing records the editing actions in history, not the
     * resulting buffer.  If we are also reading from a command-file,
     * the inline-edit is interleaved with the newline/arrow codes that
     * end a buffer-edit.
     */
    dyn_init(&script_bfr, 1);
    if (inflag) {
	original = dyn_copy(original, buffer);

	if ((s = dyn_string(*inflag)) != NULL) {
	    script_bfr = dyn_copy(script_bfr, s);
	    convert_newline(&script_bfr);
	}
    }

    getyx(stdscr, y, x);

    for (;;) {

	/*
	 * Interleave inline-editing replay and command-file:
	 */
	if (inflag) {
#ifdef	DEBUG
	    dlog_comment("hidden:%d, length:%d\n",
			 inline_hidden(), dyn_length(script_bfr));
	    dlog_comment("INLINE:%s\n", dyn_string(script_bfr));
#endif
	    script_inx = dyn_length(script_bfr);
	    if (cmd_ptr)
		script_bfr = dyn_append(script_bfr, cmd_ptr);
	} else if (cmd_ptr) {
	    script_bfr = dyn_copy(script_bfr, cmd_ptr);
	    script_inx = 0;
	} else {
	    dyn_init(&script_bfr, 1);
	    script_inx = 0;
	}

	/*
	 * Now, 'script_bfr' is empty only if we are neither replaying
	 * inline text, nor reading from a command-file:
	 */
	if ((use_script = (dyn_length(script_bfr) != 0)) != 0) {
	    script_ptr = dyn_string(script_bfr);
#ifdef	DEBUG
	    dlog_comment("SCRIPT:%s\n", dyn_string(script_bfr));
#endif
	}

	/*
	 * Perform interactive (or scripted) edit:
	 */
	before = dyn_copy(before, buffer);
	after = dyn_copy(after, buffer);

	move(y, x);
	gets_active = TRUE;
	dedsize(gbl);
	c = wrawgets(stdscr,
		     buffer,
		     prefix,
		     len,
		     len + 1,
		     (inflag != 0) ? 0 : (int) strlen(buffer),
		     (fast_q == EOS),
		     wrap,
		     fast_q,
		     use_script ? &script_ptr : (char **) 0,
		     history || inflag || log_fp);
	dedsize((RING *) 0);
	gets_active = FALSE;

	/* account for chars we read from command-file */
	if (*cmd_ptr) {
	    cmd_ptr += (script_ptr - dyn_string(script_bfr))
		- script_inx;
#ifdef	DEBUG
	    dlog_comment("s::CMD:%d:%d:%s\n",
			 cmd_ptr - dyn_string(cmd_bfr),
			 dyn_length(cmd_bfr),
			 cmd_ptr);
#endif
	}

	/*
	 * Record the inline-editing keystrokes (new response only,
	 * ignoring the characters from the script).
	 */
	if (log_fp)
	    pending = dyn_append(pending, script_inx + rawgets_log());

	/*
	 * Copy the inline-editing keystrokes (except the terminating
	 * newline/arrow) to 'script_bfr' so we can manipulate it in
	 * the history-record.
	 */
	if (inflag) {
	    script_bfr = dyn_copy(script_bfr, rawgets_log());
	    trim_ending(script_bfr);
#ifdef	DEBUG
	    dlog_comment("EDITED:%s\n", dyn_string(script_bfr));
#endif
	    to_hist = dyn_string(script_bfr);
	} else
	    to_hist = buffer;

	/* If any change was made, reset history-index */
	if (strcmp(dyn_string(before), dyn_string(after))) {
	    nnn = 0;
	    after = dyn_copy(after, buffer);
	    edited = dyn_copy(edited, to_hist);
	}

	if (c == KEY_UP) {
	    if (!nnn)
		edited = dyn_copy(edited, to_hist);

	    if (!history)
		IGNORE;

	    else if ((s = get_history(*history, nnn)) != NULL) {
		if (strcmp(s, to_hist)) ;	/* cannot skip */
		else if ((s = get_history(*history, nnn + 1)) != NULL)
		    nnn++;
		else
		    IGNORE;	/* last and only item */
		nnn++;
	    } else
		IGNORE;

	} else if (c == KEY_DOWN) {
	    if (!history)
		IGNORE;

	    else if ((s = get_history(*history, nnn - 2)) != NULL) {
		nnn--;
		if (nnn == 1 && !strcmp(s, dyn_string(edited)))
		    nnn = 0;
	    } else
		IGNORE;

	} else if (c == '\n') {
	    done = TRUE;
	    break;
	} else {		/* assume quit */
	    done = -TRUE;
	    break;
	}

	/*
	 * Reset the script+result for the next editing pass.
	 */
	if (inflag) {
	    if (s != to_hist)
		script_bfr = dyn_copy(script_bfr, s);
	    *result = dyn_copy(*result, dyn_string(original));
	} else {
	    *result = dyn_copy(*result, s);
	}
	buffer = dyn_string(*result);
    }

    if (inflag) {
	supply_newline(&script_bfr);
	*inflag = dyn_copy(*inflag, to_hist = dyn_string(script_bfr));
    }

    PENDING(string, TRUE);

#ifdef	DEBUG
    dlog_comment("%s:%s%c",
		 (done == TRUE) ? "done" : "quit",
		 to_hist,
		 inflag ? EOS : '\n');
#endif
    if (done == TRUE) {
	if (!inflag)
	    put_history(history, to_hist);
	return buffer;
    }
    return 0;			/* if quit, return null */
}

/*
 * Log elapsed time since the beginning of a command
 */
void
dlog_elapsed(void)
{
    if (log_fp) {
	dlog_comment("elapsed time = %ld seconds\n", NOW - mark_time);
    }
}
/*
 * Flush the pending command.  We buffer commands so that multi-character
 * stuff (such as sort) is written on one line.
 */
void
dlog_flush(void)
{
    if (log_fp) {
	PENDING(flush, TRUE);
	(void) fflush(log_fp);
    }
}

/*
 * Annotate the given command with the name of the current entry
 */
void
dlog_name(char *name)
{
    dlog_comment("\"%s\"\n", name);
}

/*
 * Write a comment to the log-file (with trailing newline in 'fmt').
 */
void
dlog_comment(char *fmt,...)
{
    va_list args;
    static DYN *msg, *tmp;
    char buffer[BUFSIZ], Fmt[BUFSIZ];

    if (!log_fp)
	return;

    PENDING(comment, FALSE);
    FPRINTF(log_fp, "\t# ");
    va_start(args, fmt);

    dyn_init(&msg, 1);

    while (*fmt) {
	int c = *fmt++;
	if (c == '%' && *fmt == '%') {
	    msg = dyn_append_c(msg, *fmt++);
	} else if (c == '%') {
	    char *dst = Fmt;
	    int is_long = FALSE;

	    *dst++ = c;
	    do {
		if ((c = *fmt++) == '*') {
		    FORMAT(dst, "%d", va_arg(args, int));
		    dst += strlen(dst);
		} else {
		    *dst++ = c;
		    if (c == 'l') {
			is_long = TRUE;
			continue;
		    }
		}
	    } while ((c == 'l') || !isalpha(c));
	    *dst = EOS;

	    switch (c) {
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
		FORMAT(buffer, Fmt, va_arg(args, double));
		break;
	    case 's':
		dst = va_arg(args, char *);
		dyn_init(&tmp, 1);

		if (!dst)
		    dst = "<null>";
		while ((c = *dst++) != EOS) {
		    c = toascii(c);
		    if (c == '\n' && *fmt == EOS) ;	/* fix for ctime */
		    else if (!isprint(c)) {
			tmp = dyn_append_c(tmp, '^');
			if (c == '\177')
			    c = '?';
			else
			    c |= '@';
		    }
		    tmp = dyn_append_c(tmp, c);
		}
		FORMAT(buffer, Fmt, dyn_string(tmp));
		break;
	    default:		/* c, d, i, o, p, u, x, X */
		if (is_long)
		    FORMAT(buffer, Fmt, va_arg(args, long));
		else
		    FORMAT(buffer, Fmt, va_arg(args, int));
		break;
	    }
	    msg = dyn_append(msg, buffer);
	} else
	    msg = dyn_append_c(msg, c);
    }
    va_end(args);
    FPRINTF(log_fp, "%s", dyn_string(msg));
}
