/*
 * Title:	ded.c (directory-editor)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		24 Jun 2010, look first for help-file in data-directory.
 *		07 Sep 2004, add -D option for date-editing.
 *		07 Mar 2004, remove K&R support, indent'd
 *		03 Jul 2003, move dedsigs() after initscr() to avoid conflict
 *			     with ncurses' sigwinch() handler.
 *		21 Dec 2002, use setlocale(), needed with ncursesw
 *		16 Apr 2002, make 'T' a 4-way toggle, showing seconds if T=3.
 *		30 Jan 2001, avoid aborting on initscr failure (use -d option,
 *			     or $DED_DEBUG variable to get this behavior).
 *		19 Oct 2000, add '-p' option to print selected pathnames.
 *		08 Apr 2000, remove unneeded call for ncurses' trace().
 *		16 Aug 1999, use ttyname() for BeOS port.
 *		10 Aug 1999, change -b to a toggle, allow curses to decide if
 *			     box characters are available.
 *		29 May 1998, compile with g++
 *		15 Feb 1998, remove special code for apollo sr10.
 *			     working on signed/unsigned compiler warnings.
 *			     add home/end/ppage/npage cases.
 *			     change tag/untag to repaint faster.
 *		09 Jan 1996, mods for scrolling-regions
 *		16 Dec 1995, added '-i' option.
 *		05 Nov 1995, mods to prevent tilde-expansion in cNAME
 *		30 Aug 1995, added "-e" option.
 *		16 Jul 1994, allow DED_TREE to be file or directory.
 *		23 May 1994, port to Solaris.
 *		26 Apr 1994, provided sys5-like defaults for EDITOR, etc.
 *		23 Nov 1993, new blip-code.
 *		19 Nov 1993, added xterm-mouse support.
 *		01 Nov 1993, port to HP/UX.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		12 Aug 1992, added '-command.
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		30 Mar 1992, corrected highlighting of long, shifted lines.
 *		28 Feb 1992, convert shell-command to dynamic-string.
 *		21 Nov 1991, make 'tag_opt' public to use in dedring.
 *		15 Oct 1991, converted to ANSI. Allow replay of 'c' commands.
 *		16 Aug 1991, added interpretation of "2T"
 *		22 Jul 1991, quote filename before using it in 'forkfile()'
 *		10 Jul 1991, added parm to 'markset()' to tell if we must clear
 *			     workspace
 *		16 Jul 1991, modified logic of 'edithead()' to account for the
 *			     case in which the current entry contains '/'.
 *		12 Jul 1991, added CTL/G command to show tagged files
 *			     (+blocks/bytes).  Cleanup some message code.
 *		11 Jul 1991, modified interface to to_work().  Broke-out
 *			     'showMARK()' for use in 'dedtype()'.  Modified
 *			     interface to 'showFILES()' so that scrolling (in a
 *			     given file-list) and inline/toggle operations do
 *			     not cause the workspace to be cleared.
 *		02 Jul 1991, made S_opt, P_opt 3-way toggles.
 *		28 Jun 1991, lint (apollo sr10.3)
 *		20 Jun 1991, don't need "-" special argument to make pipe-args.
 *		31 May 1991, modified interface to 'showpath()'
 *		15 May 1991, mods to accommodate apollo sr10.3
 *		22 Apr 1991, re-open standard input if we were reading from a
 *			     pipe.
 *		18 Apr 1991, mods to 'dedwait()' and 'dedread()' to debug and
 *			     guard against user-pattern not finding files.
 *		16 Apr 1991, suppress empty-strings from argument list.
 *		04 Apr 1991, guard against 'getwd()' failure.
 *		27 Aug 1990, mods to make error-reporting routines work properly
 *			     if they are called before screen is initialized,
 *			     etc., to support mods to "ftree.c" for better error
 *			     recovery.
 *		23 May 1990, Modified so that CTL(E) and CTL(V) on a directory
 *			     will cause a prompt for read-pattern a la CTL(R).
 *			     Allow CTL(E) command from directory-tree as well
 *		18 May 1990, modified 'edithead()' so it does the "right" thing
 *			     when going to an Apollo DSEE revision.
 *		07 May 1990, make "-t" option inherit into subprocesses
 *		17 Apr 1990, simplified/corrected code for 'edithead()'
 *		06 Mar 1990, test for sort-keys which assume RCS/SCCS scanning
 *			     is in effect and perform scanning if it has not
 *			     been done.
 *		05 Mar 1990, forgot to init reply-buffer in 'user_says()'
 *		02 Mar 1990, set 'no_worry' in 'dedring()', not 'new_args()'.
 *			     Modified quit-behavior so that if user has gone
 *			     into any directory other than the original one, he
 *			     will be prompted on quit.  Added "-n" option so
 *			     this doesn't happen in a subprocess.  Added
 *			     special case for typing contents of directory-file.
 *		07 Feb 1990, modified '#' command (deduniq-proc) to 3 modes of
 *			     operation.
 *		01 Feb 1990, use 'showpath()' to handle long pathname-display
 *		30 Jan 1990, added 'T' (date+time) toggle and command-option to
 *			     match.  Altered 't' command so "2t" types binary-
 *			     file.  Modified shell-command stuff so 0/2 repeat-
 *			     count on ':' or '.' can reset/set the clear-screen
 *			     flag of '!'/'%' commands
 *		01 Dec 1989, broke out 'sortset()' module
 *		12 Oct 1989, converted 'I', 'G' commands to three-state toggles
 *		06 Oct 1989, modified 'showFILES()' so that on certain calls we
 *			     reset the 'cmdcol[]' array.
 *		04 Oct 1989, added -a, -O options.  Added &, O toggles.  Added
 *			     o, O sorts.
 *		25 Aug 1989, added new procedure 'scroll_to_file()' so 'E'-
 *			     command on link can go to link-target.  Use
 *			     'wrepaint()' rather than savewin/unsavewin.  Added
 *			     arg to 'realstat()' for 'E'-enhancement.
 *		22 Aug 1989, if user tries to apply 'E' command to symbolic-
 *			     link-to-file, edit instead the directory containing
 *			     the target file.
 *		11 Aug 1989, added/used procedure 'move2row()'
 *		06 Jun 1989, made blip-call for 'Z' toggle show results like
 *			     '#'.
 *		31 May 1989, revised/updated 'usage()'
 *		26 May 1989, Corrected last mod so failed-rescan keeps original
 *			     name to find in resulting list.  Added CTL/R
 *			     command to control read-selection
 *		03 Apr 1989, use of 'showFILES()' in 'restat_W()' did not work
 *			     (?).  Recoded using 'showLINE()' and 'showC()'.
 *		24 Mar 1989, added "-c" (command-script) option, and changed
 *			     version to RCS format using sscanf hack.
 *		15 Mar 1989, if log-option is set, pass this to subprocess ded.
 *		14 Mar 1989, added '-l' option for logging.
 *		13 Mar 1989, added '<' command (short-form of '>')
 *		23 Jan 1989, added 'N' sort
 *		20 Jan 1989, added '-t' option.
 *		18 Jan 1989, added '#' command.
 *		12 Sep 1988, show blip during '@'.  Added 'c' command.
 *		02 Sep 1988, added '>' command.
 *		01 Sep 1988, trim repeats in 'argv[]'.
 *		17 Aug 1988, added repeat (sleep) count to 'W', 'l'.
 *		12 Aug 1988, apollo sys5 environment permits symbolic links.
 *			     Added 'd' (directory-order) sort.
 *		04 Aug 1988, added debug-option.
 *		03 Aug 1988, added signal catch/ignore processing.
 *		02 Aug 1988, show column-scale on workspace marker, permit a
 *			     repeat-count for left/right scroll.
 *		01 Aug 1988, moved Xbase,Ybase to ring-structure.  Broke out
 *			     'dedline.c' module to work on 'X' command here.
 *			     Implemented a crude split-window command 'X'.
 *		27 Jul 1988, use 'execute()' to parse args, etc., in forkfile.
 *			     modified 'padedit()' to support X-windows.
 *		11 Jul 1988, added '+' mode to sort.
 *		16 Jun 1988, added '@' toggle.
 *		07 Jun 1988, added CTL(K) screen-dump
 *		06 Jun 1988, if 'R' finds nothing, do 'F' to recover before 'q'.
 *		01 Jun 1988, added 'Y' toggle, y-sort.
 *		25 May 1988, fix 'edittext()' for left/right scroll position.
 *			     also, +/- update via 'showDOWN()'.
 *		23 May 1988, use 'setmtime()', corrected masking in 'replay()'.
 *		13 May 1988, provide walkback on Apollo after 'failed()'.
 *		11 May 1988, use 'rename()' if it is available.
 *		03 May 1988, added 'P', 's', 't' subcommands to editprot.
 *		02 May 1988, forgot to allow arrow keys in insert-mode of
 *			     'edittext()'.
 *		28 Apr 1988, added 'new_args()' to integrate with ftree module.
 *			     Modified 'edittext()' to correspond with rawgets.
 *			     Added "=" renaming command.
 *		21 Apr 1988, use 'savewin()' module for retouch.  Apollo SR9.7
 *			     changes 'fixtime()'.  Added call on 'resizewin'.
 *		25 Mar 1988, corrected 'retouch()' so that last line is cleared,
 *			     and so we don't have to change terminal-mode.
 *			     Added ':' command, and began interface to 'ftree'.
 *		24 Mar 1988, to provide interface to 'ftree.c' module.
 *
 * Function:	Interactively display/modify unix directory structures
 *		and files.
 */

#define	MAIN
#include	"ded.h"

#include <locale.h>

MODULE_ID("$Id: ded.c,v 12.79 2010/07/04 23:02:35 tom Exp $")

#define	EDITOR	DEFAULT_EDITOR
#define	BROWSE	DEFAULT_BROWSE
#define	PAGER	DEFAULT_PAGER

#define	COMPLEMENT(opt) (opt) = !(opt)

int debug = FALSE;		/* generic debug-flag */
int no_worry = -1;		/* don't prompt on quit */
int in_screen;			/* TRUE if we have init'ed */

/*
 * Other, private main-module state:
 */
static char whoami[MAXPATHLEN],	/* my execution-path */
 *log_opt,			/* log-file option */
 *tree_opt,			/* my file-tree database */
  howami[MAXPATHLEN];		/* my help-file */

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Returns 0, 1 or val for a multi-way toggle.  Normally 3 states.
 */
static int
one_or_both(int opt,
	    int val)
{
    if (val == 0)
	opt = 0;
    else if (val == 1)
	opt = !opt;
    else
	opt = val;
    return (opt);
}

#ifdef	S_IFLNK
static int
edithead(RING * gbl,
	 char *dst,		/* receives destination-directory */
	 char *leaf)		/* receives destination-leaf */
{
    Stat_t sb;
    char *s;

    if (cLTXT != 0) {
	dlog_comment("try to edit link-head \"%s\"\n", cLTXT);
	(void) pathcat2(dst, gbl->new_wd, cLTXT);
	(void) strcpy(dst, pathhead(dst, &sb));
	dlog_comment("... becomes pathname  \"%s\"\n", dst);
	(void) strcpy(leaf, cLTXT);
	if ((s = strrchr(cLTXT, '/')) != NULL) {
#ifdef	apollo
	    /* special hack for DSEE library */
	    if (s[1] == '[') {
		leaf[s - cLTXT] = EOS;
		if (s = strrchr(leaf, '/')) {
		    s++;
		    while (*leaf++ = *s++) ;
		}
	    } else
#endif /* apollo */
	    if (s[1] != EOS)
		(void) strcpy(leaf, ++s);
	}
	return (TRUE);
    }
    return (FALSE);		/* head would duplicate current directory */
}
#endif /* S_IFLNK */

/************************************************************************
 *	public	utility procedures					*
 ************************************************************************/

/*
 * Exit from window mode
 */
void
to_exit(int last)
{
    if (in_screen) {
	if (last) {
	    clearmsg();
	    refresh();
	}
	cookterm();
	endwin();
	restore_terminal();	/* some curses are buggy */
    }
}

/*
 * Determine if the given entry is a file, directory or none of these.
 */
int
realstat(RING * gbl, unsigned inx, Stat_t * sb)
{
    mode_t j = gSTAT(inx).st_mode;

#ifdef	S_IFLNK
    if (isLINK(j)) {
	j = (stat(gNAME(inx), sb) >= 0) ? sb->st_mode : 0;
    } else
	sb->st_mode = 0;
#endif
    if (isFILE(j))
	return (0);
    if (isDIR(j))
	return (1);
    return (-1);
}

/*
 * Fatal-error exit from this process
 */
void
failed(const char *msg)
{
    if (debug) {
	FPRINTF(stderr, "failed?");
	(void) cmdch((int *) 0);
    }
    to_exit(msg != 0);
    if (msg)
	FPRINTF(stderr, "-------- \n?? %-79s\n-------- \n", msg);
    if (msg) {
	(void) fflush(stderr);
	if (getenv("DED_DEBUG") != 0 || debug)
	    abort();
    }

    dlog_exit(FAIL);
}

/*
 * Prompt user for yes/no response
 */
int
user_says(RING * gbl, int ok)
{
    char *s;
    static DYN *reply;

    if (!ok) {
	dyn_init(&reply, 8);
	if ((s = dlog_string(gbl, "Are you sure (y/n)? ", -1, &reply,
			     (DYN **) 0, NO_HISTORY, EOS, -8)) != NULL)
	    ok = (*s == 'y' || *s == 'Y');
	showC(gbl);
    }
    return (ok);
}

/*
 * Given the name of a file in the current list, find its index in 'flist[]'.
 * This is used to reposition after sorting, etc, and uses the feature that
 * strings in 'txtalloc()' are uniquely determined by their address.
 */
unsigned
findFILE(RING * gbl, char *name)
{
    unsigned j;
    for_each_file(gbl, j)
	if (name == gNAME(j))
	return j;
    return (0);			/* give up, set to beginning of list */
}

#ifdef	Z_RCS_SCCS
static int
needSCCS(RING * gbl, int c)
{
    return (!gbl->Z_opt && (strchr("vyZz", c) != 0));
}

void
showSCCS(RING * gbl)
{
    unsigned j;

    if (!gbl->Z_opt) {		/* catch up */
	set_dedblip(gbl);
	gbl->Z_opt = -1;
	for_each_file(gbl, j) {
	    FLIST *f = &(gbl->flist[j]);
	    if (!f->z_time) {
		statSCCS(gbl, gNAME(j), f);
		put_dedblip((f->z_time != 0) ? '#' : '.');
	    }
	}
    }
}
#endif /* Z_RCS_SCCS */

/*
 * Repaint the screen
 */
void
retouch(RING * gbl, int row)
{
    int y, x;
#if	defined(apollo) || defined(SIGWINCH)
    if (resizewin()) {
	dlog_comment("resizewin(%d,%d)\n", LINES, COLS);
	markset(gbl, (unsigned) mark_W);
	showFILES(gbl, FALSE);
	return;
    }
#endif /* apollo */
    getyx(stdscr, y, x);
    move(mark_W + 1, 0);
    clrtobot();
    move(y, x);
    wrepaint(stdscr, row);
}

/*
 * Process the given function in a repeat-loop which is interruptable.
 */
void
resleep(RING * gbl,
	int count,
	void (*func) (RING * g))
{
    int interrupted = 1, last = count;

    while (count-- > 1) {
	move(LINES - 1, 0);
	PRINTW("...waiting (%d of %d) ...", last - count, last);
	clrtoeol();
	(*func) (gbl);
	refresh();
	sleep(3);
	if ((interrupted = dedsigs(TRUE)) != 0)
	    break;
    }
    clearmsg();
    if (interrupted)
	(*func) (gbl);
    else
	showC(gbl);
}

/*
 * Use the 'dedring()' module to switch to a different file-list
 */
static RING *
new_args(RING * gbl,
	 char *path,
	 int cmd,
	 int count,
	 int flags,
	 int set_pattern,
	 char *pattern)
{
    RING *ok;

    if (flags & 1)
	markC(gbl, TRUE);
    clear_work();
    if ((ok = dedring(gbl, path, cmd, count, set_pattern, pattern)) != 0) {
	redoVIEW(gbl = ok, FALSE);
	(void) to_file(gbl);
	count_tags(gbl);
	showFILES(gbl, TRUE);
    }
    (void) chdir(gbl->new_wd);
    dlog_comment("chdir %s\n", gbl->new_wd);
    if (!ok && (flags & 2))
	showC(gbl);
    return ok;
}

/*
 * Set list to an old set of arguments
 */
static RING *
old_args(RING * gbl,
	 int cmd,		/* 'F' or 'B' */
	 int count)
{
    RING *tmp;

    tmp = new_args(gbl, gbl->new_wd, cmd, count, 0, FALSE, (char *) 0);
    if (tmp != 0)
	gbl = tmp;
    else
	showC(gbl);		/* try to recover */
    return gbl;
}

/*
 * Open a (new) argument-list, setting the scan-pattern
 */
static RING *
pattern_args(RING * gbl, char *path)
{
    char *pattern = 0;
    RING *tmp;

    while (dedread(gbl, &pattern, FALSE)) {
	if ((tmp = new_args(gbl, path, 'E', 1, 3, TRUE, pattern)) != NULL)
	    return (tmp);
    }
    return (0);
}

	/* re-scan argument list */
static RING *
rescan(RING * gbl, int fwd)
{
    char *cur_name = gbl->numfiles ? cNAME : 0;

    set_dedblip(gbl);
    init_tags(gbl);
    if (dedscan(gbl)) {
	gbl->curfile = (unsigned) (cur_name
				   ? findFILE(gbl, cur_name)
				   : 0);
	(void) to_file(gbl);
	showFILES(gbl, TRUE);
	return (gbl);
    } else if (fwd) {
	return old_args(gbl, 'F', 1);
    }
    return (0);
}

/*
 * Convert a name to a form which shell commands can use.  For most
 * names, this is simply a copy of the original name.  However, on
 * Apollo, we may have names with '$' and other special characters.
 */
char *
fixname(RING * gbl, unsigned j)
{
    static char nbfr[MAXPATHLEN];
    (void) ded2string(gbl, nbfr, sizeof(nbfr), gNAME(j), TRUE);
    return (nbfr);
}

/*
 * Adjust mtime-field so that chmod, chown do not alter it.
 * This fixes Apollo/NFS kludges!
 */
void
fixtime(RING * gbl, unsigned j)
{
    if (setmtime(gNAME(j), gSTAT(j).st_mtime, gSTAT(j).st_atime) < 0)
	warn(gbl, "utime");
}

/*
 * Spawn a subprocess, wait for completion.
 */
static void
forkfile(RING * gbl, const char *arg0, const char *arg1, int option)
{
    char quoted[MAXPATHLEN];

    *quoted = EOS;
    catarg(quoted, arg1);

    cookterm();
    dlog_comment("execute %s %s\n", arg0, arg1);
    dedsigs(FALSE);
    (void) signal(SIGINT, SIG_IGN);	/* Linux need this */
    if (execute(arg0, quoted) < 0)
	warn(gbl, arg0);
    dedsigs(TRUE);
    dlog_elapsed();
    rawterm();

    switch (option) {
    case TRUE + 1:
	dedwait(gbl, TRUE);
	restat(gbl, FALSE);
	break;
    case TRUE:
	retouch(gbl, 0);
	restat(gbl, FALSE);
    }
}

/*
 * Enter an editor (separate process) for the current-file/directory.
 */
static RING *
run_editor(RING * gbl, int readonly, int extended)
{
    Stat_t sb;
    const char *editor = (readonly ? ENV(BROWSE) : ENV(EDITOR));
    char tpath[MAXPATHLEN];

    dlog_name(cNAME);
    (void) pathcat2(tpath, gbl->new_wd, cNAME);
    switch (realstat(gbl, gbl->curfile, &sb)) {
    case 0:			/* edit-file */
	to_work(gbl, TRUE);
	if (extended) {
	    if (padedit(tpath, readonly, editor) < 0)
		beep();
	    restat(gbl, FALSE);
	} else
	    forkfile(gbl, editor, tpath, TRUE);
	break;
    case 1:			/* edit-directory */
	if (extended) {
	    RING *tmp;
	    if ((tmp = pattern_args(gbl, tpath)) != NULL)
		gbl = tmp;
	} else {
	    to_work(gbl, TRUE);
	    ft_write();
	    dlog_close();
	    forkfile(gbl, whoami, cNAME, TRUE);
	    dlog_reopen();
	    ft_read(gbl->new_wd, tree_opt);
	    if (no_worry < 0)	/* start worrying! */
		no_worry = FALSE;
	}
	break;
    default:
	dedmsg(gbl, "cannot edit this item");
    }
    return gbl;
}

/*
 * Edit a (new) directory w/o spawning a process.
 */
static RING *
edit_directory(RING * gbl)
{
    RING *tmp = gbl;
    char tpath[MAXPATHLEN];
    char dpath[MAXPATHLEN];
    Stat_t sb;

    if (realstat(gbl, gbl->curfile, &sb) == 1) {
	if (!(tmp = new_args(gbl,
			     pathcat2(tpath, gbl->new_wd, cNAME),
			     'E', 1, 3, FALSE, (char *) 0)))
	    return gbl;

#ifdef	S_IFLNK			/* try to edit head-directory of symbolic-link */
    } else if (edithead(gbl, tpath, dpath)) {
	if (strcmp(tpath, gbl->new_wd)
	    && !(tmp = new_args(gbl, tpath, 'E', 1, 3, FALSE, (char *) 0)))
	    return gbl;

	scroll_to_file(tmp, findFILE(tmp, txtalloc(dpath)));
#endif /* S_IFLNK */

    } else
	dedmsg(gbl, "not a directory");

    return tmp;
}

/*
 * Invoke a new file-list from the directory-tree display, cleaning up if
 * it fails.
 */
static RING *
new_tree(RING * gbl, char *path, int cmd)
{
    RING *tmp;

    if (iscntrl(cmd))
	tmp = pattern_args(gbl, path);
    else
	tmp = new_args(gbl, path, 'E', 1, 0, FALSE, (char *) 0);

    return tmp;
}

/*
 * Invoke a new copy of 'ded'
 */
static void
new_process(RING * gbl, char *path)
{
    int y, x;

    getyx(stdscr, y, x);
    if (++y >= LINES)
	y = LINES - 1;
    move(y, x - x);
    clrtobot();
    move(y, 0);
    ft_write();
    dlog_close();
    forkfile(gbl, whoami, path, FALSE);
    dlog_reopen();
    /*wrepaint(stdscr,0); */
    ft_read(gbl->new_wd, tree_opt);
}

static void
trace_pipe(char *arg)
{
    if (debug) {
	if (debug > 1) {
	    FPRINTF(stderr, "%s\n", arg);
	    (void) fflush(stderr);
	} else
	    put_dedblip('#');
    }
}

static int
x_scroll(void)
{
    return ((1 + (COLS / 4) / 10) * 10);
}

/************************************************************************
 *	main program							*
 ************************************************************************/

void
usage(void)
{
    char tmp[BUFSIZ];
    static const char *tbl[] =
    {
	"Usage: ded [options] [filespecs]",
	"(filespecs may be read from pipe)",
	"",
	"Options which alter initial display fields:",
	"  -A       show \".\" and \"..\" names",
	"  -G       show group-name instead of user-name",
	"  -I       show inode field",
	"  -P       show protection in octal",
#if defined(HAVE_NEWTERM)
	"  -p       print selected filenames",
#endif
	"  -S       show file-size in blocks",
	"  -T       show long date+time",
#ifdef	apollo
	"  -U       show AEGIS-style names",
#endif
#ifdef	Z_RCS_SCCS
	"  -Z       read RCS/SCCS data, show date",
	"  -z       read RCS/SCCS data, don't show date",
#endif
	"",
	"Options controlling initial sort:",
	"  -s KEY   set forward sort",
	"  -r KEY   set reverse sort",
	"",
	"Options controlling environment:",
	"  -b       use box characters",
	"  -c FILE  read DED commands from FILE",
	"  -e       edit 'e' in new process",
	"  -d       (debug)",
#if defined(HAVE_HAS_COLORS)
	"  -i       invert default colors",
#endif
	"  -l FILE  write DED commands to log-FILE",
	"  -n       disable \"are you sure\" on quit",
	"  -t DIR   read \".ftree\"-file from directory DIR",
	(char *) 0
    };
    const char **p;

    setbuf(stderr, tmp);
    for (p = tbl; *p; p++)
	FPRINTF(stderr, "%s\n", *p);
    FPRINTF(stderr, "\nSort KEY-options are: \"%s\"\n", sortc);

    dlog_exit(FAIL);
}

static int
inline_nesting(RING * gbl, int c)
{
    if (c == 'c') {
	ReplayTopC(c);
	c = dlog_char(gbl, (int *) 0, FALSE);
    } else {
	switch (c) {
#ifdef	S_IFLNK
	case 'l':
#endif /* S_IFLNK */
	case 'd':
	case 'f':
	case 'L':
	    ReplayTopC('c');
	    break;
	default:
	    ReplayTopC(EOS);
	}
    }
    return (c);
}

static void
inline_command(RING * gbl, int c)
{
    switch (c) {
    case 'p':
	editprot(gbl);
	break;
    case 'u':
	edit_uid(gbl);
	break;
    case 'g':
	edit_gid(gbl);
	break;
    case '=':
	editname(gbl);
	break;
#ifdef	S_IFLNK
    case '<':
    case '>':
	editlink(gbl, c);
	break;
    case 'l':
#endif /* S_IFLNK */
    case 'd':
    case 'f':
    case 'L':
	dedmake(gbl, c);
	break;

    case 'T':
	if (edit_dates)
	    editdate(gbl, gbl->curfile, FALSE);
	else
	    dedmsg(gbl, "Date-editing is disabled");
	break;

    default:{
	    char temp[80];
	    FORMAT(temp, "no inline command (%c)", isprint(c) ? c : '?');
	    dedmsg(gbl, temp);
	}
    }
    (void) edit_inline(FALSE);
}

/*ARGSUSED*/
_MAIN
{
#include	"version.h"

    int optBox = FALSE;
    int optInprocess = TRUE;
    int j;
    unsigned k;
    Stat_t sb;
    int c;
    int count;
    int lastc = '?';
    char tree_bfr[MAXPATHLEN];
    char tpath[MAXPATHLEN];
    char dpath[MAXPATHLEN];

    RING *gbl = ring_alloc();
#if defined(HAVE_NEWTERM)
    int do_select = FALSE;
#endif

#ifdef LOCALE
    setlocale(LC_ALL, "");
#endif
    (void) sortset(gbl, 's', 'n');
    (void) sscanf(version, "%*s %s %s", tpath, dpath);
    FPRINTF(stderr, "DED Directory Editor (%s %s)\r\n", tpath, dpath);
    /* show when entering process */
    (void) fflush(stdout);

    /* if curses supports line-drawing characters, try to use them */
#ifdef ACS_PLUS
    optBox = TRUE;
#endif
    while ((c = getopt(argc, argv, "abDeGIiOPSTUZc:l:r:s:zdt:np")) != EOF)
	switch (c) {
	case 'a':
	    COMPLEMENT(gbl->A_opt);
	    break;
	case 'b':
	    optBox = !optBox;
	    break;
	case 'D':
	    edit_dates = TRUE;
	    gbl->T_opt = TRUE;
	    break;
	case 'e':
	    optInprocess = FALSE;
	    break;
	case 'G':
	    COMPLEMENT(gbl->G_opt);
	    break;
	case 'I':
	    COMPLEMENT(gbl->I_opt);
	    break;
#if defined(HAVE_HAS_COLORS)
	case 'i':
	    invert_colors = TRUE;
	    break;
#else
	case 'i':
	    break;		/* ignored */
#endif
	case 'P':
	    COMPLEMENT(gbl->P_opt);
	    break;
	case 'S':
	    COMPLEMENT(gbl->S_opt);
	    break;
	case 'T':
	    if (!edit_dates)
		COMPLEMENT(gbl->T_opt);
	    break;
#ifdef	apollo
	case 'U':
	    COMPLEMENT(gbl->U_opt);
	    break;
#endif
#ifdef	Z_RCS_SCCS
	case 'Z':
	    gbl->Z_opt = 1;
	    break;
	case 'z':
	    gbl->Z_opt = -1;
	    break;
#endif
	case 'c':
	    dlog_read(optarg);
	    break;
	case 'l':
	    log_opt = dlog_open(optarg, argc, argv);
	    break;
	case 's':
	case 'r':
	    if (!sortset(gbl, c, *optarg)
		|| strlen(optarg) > 1)
		usage();
	    break;
	case 'd':
	    debug++;
	    break;
	case 't':
	    tree_opt = optarg;
	    break;
	case 'n':
	    no_worry = TRUE;
	    break;
#if defined(HAVE_NEWTERM)
	case 'p':
	    do_select = TRUE;
	    break;
#endif
	default:
	    usage();
	    /*NOTREACHED */
	}

#ifdef	Z_RCS_SCCS
    /* if we're going to sort by checkin-date, ensure we read the dates */
    if (needSCCS(gbl, gbl->sortopt))
	gbl->Z_opt = -1;
#endif /* Z_RCS_SCCS */

#define	DED_TREE	".ftree"
    if (tree_opt != 0) {
	abspath(tree_opt = strcpy(tree_bfr, tree_opt));
    } else {
	if ((tree_opt = getenv("DED_TREE")) == 0)
	    tree_opt = strcpy(tree_bfr, gethome());
    }
    if (stat_dir(tree_opt, &sb) >= 0)
	tree_opt = pathcat(tree_opt, tree_opt, DED_TREE);

    if (!getwd(old_wd))
	(void) strcpy(old_wd, ".");
    ft_read(old_wd, tree_opt);

    /* find which copy I am executing from, for future use */
    if (which(whoami, sizeof(whoami), argv[0], old_wd) <= 0)
	failed("which-path");

    sprintf(howami, "%s/ded.hlp", DATA_DIR);
    if (!stat_file(howami, &sb)) {
	/* my help-file lives where the binary does */
	strcpy(howami, whoami);
	strcpy(ftype(howami), ".hlp");
    }

    /* pass options to lower-level processes of ded */
    (void) strcat(strcat(whoami, " -t"), tree_opt);
    if (log_opt)
	(void) strcat(strcat(whoami, " -l"), log_opt);
    (void) strcat(whoami, " -n");

    if (!isatty(fileno(stdin))) {
	if (optind < argc)
	    FPRINTF(stderr, "? ignored arguments, using pipe\n");
	argc = fp2argv(stdin, &argv, trace_pipe);
	optind = 0;
	for (j = 0; j < argc; j++) {
	    char *s = argv[j];
	    int len = (int) strlen(s) - 1;
	    if (len >= 0 && s[len] == '\n')
		s[len] = EOS;	/* trim trailing newline */
	}
    }

    /* protect against pipes */
    if (!isatty(fileno(stdin))) {
# if defined(HAVE_TTYNAME)
	char *tty = ttyname(fileno(stderr));
# else
	char *tty = "/dev/tty";
# endif
	if ((freopen(tty, "r", stdin)) == 0
	    || !isatty(fileno(stdin)))
	    failed("reopen stdin");
    }

    save_terminal();

    if (getenv("TERM") == 0) {
	FPRINTF(stderr, "$TERM is not set\n");
	return (FAIL);
    }
#if defined(HAVE_NEWTERM)
    if (!newterm(getenv("TERM"), do_select ? stderr : stdout, stdin)) {
	FPRINTF(stderr, "newterm failed to initialize screen\n");
	return (FAIL);
    }
#else
    if (!initscr()) {
	FPRINTF(stderr, "initscr failed to initialize screen\n");
	return (FAIL);
    }
#endif
    (void) dedsigs(TRUE);
    (void) dedsize((RING *) 0);

#if defined(HAVE_WSCRL)
    (void) scrollok(stdscr, TRUE);
#endif
#if defined(HAVE_HAS_COLORS)
    init_dedcolor();
#endif
#if defined(HAVE_TYPEAHEAD)
    typeahead(-1);		/* disable it */
#endif
    boxchars(optBox);

    in_screen = TRUE;
    if (LINES > BUFSIZ || COLS > BUFSIZ) {
	char bfr[80];
	FORMAT(bfr, "screen too big: %d by %d\n", LINES, COLS);
	failed(bfr);
    }
    rawterm();

    argc -= optind;
    argv += optind;
    first_scan = TRUE;
    ring_args(gbl, argc, argv);
    first_scan = FALSE;

    mark_W = (LINES / 2);
    openVIEW(gbl);

    while (gbl != 0) {
	switch (c = dlog_char(gbl, &count, 1)) {
	    /* scrolling */
	case KEY_UP:
	case '\b':
	case 'k':
	    upLINE(gbl, (unsigned) count);
	    break;

	case KEY_DOWN:
	case '\n':
	case 'j':
	    downLINE(gbl, (unsigned) count);
	    break;

	case KEY_HOME:
	    upLINE(gbl, gbl->curfile);
	    break;

	case KEY_END:
	    downLINE(gbl, gbl->numfiles - gbl->curfile);
	    break;

	case KEY_NPAGE:
	case 'f':
	    scrollVIEW(gbl, count);
	    break;

	case KEY_PPAGE:
	case 'b':
	    scrollVIEW(gbl, -count);
	    break;

#ifndef	NO_XTERM_MOUSE
	case KEY_MOUSE:
	    if (xt_mouse.released) {
		if (xt_mouse.button == 1) {
		    gbl = row2VIEW(gbl, xt_mouse.row);
		    if (xt_mouse.dbl_clik) {
			j = (realstat(gbl, gbl->curfile, &sb)) ? 'E' : CTL('v');
			(void) ungetch(j);
		    }
		} else {
		    beep();
		}
	    }
	    break;
#endif
	case KEY_LEFT:
	    if (gbl->Xbase > 0) {
		if ((gbl->Xbase -= x_scroll() * count) < 0)
		    gbl->Xbase = 0;
		showFILES(gbl, FALSE);
	    } else
		dedmsg(gbl, "already at left margin");
	    break;

	case KEY_RIGHT:
	    if ((j = (gbl->Xbase + (x_scroll() * count))) < 990) {
		gbl->Xbase = j;
		showFILES(gbl, FALSE);
	    } else
		beep();
	    break;

	    /* cursor-movement in-screen */
	case 'H':
	    gbl->curfile = baseVIEW();
	    showC(gbl);
	    break;
	case 'M':
	    gbl->curfile = ((baseVIEW() + (unsigned) lastVIEW()) / 2);
	    showC(gbl);
	    break;
	case 'L':
	    gbl->curfile = (unsigned) lastVIEW();
	    showC(gbl);
	    break;
	case '^':
	    top2VIEW(gbl);
	    break;

	    /* display-toggles */
#ifdef	S_IFLNK
	case '@':
	    COMPLEMENT(gbl->AT_opt);
	    count = 0;
	    set_dedblip(gbl);
	    for_each_file(gbl, k) {
		if (gLTXT(k)) {
		    put_dedblip('@');
		    statLINE(gbl, k);
		    count++;
		} else
		    put_dedblip('.');
	    }
	    if (count)
		showFILES(gbl, TRUE);
	    else
		showC(gbl);
	    break;
#endif /* S_IFLNK */

	    /* note that '.' and '..' may be the only files! */
	case '&':
	    COMPLEMENT(gbl->A_opt);	/* sorry about inconsistency */
	    gbl = rescan(gbl, TRUE);
	    break;
	case 'G':
	    gbl->G_opt = one_or_both(j = gbl->G_opt, count);
	    showFILES(gbl, (gbl->G_opt < 2) != (j < 2));
	    break;
	case 'I':
	    gbl->I_opt = one_or_both(j = gbl->I_opt, count);
	    showFILES(gbl, gbl->I_opt != j);
	    break;
	case 'P':
	    gbl->P_opt = one_or_both(j = gbl->P_opt, count);
	    showFILES(gbl, gbl->P_opt != j);
	    break;
	case 'S':
	    gbl->S_opt = one_or_both(j = gbl->S_opt, count);
	    showFILES(gbl, gbl->S_opt != j);
	    break;
	case 'T':
	    if (edit_dates) {
		inline_command(gbl, ReplayInit(inline_nesting(gbl, c)));
	    } else {
		gbl->T_opt = one_or_both(j = gbl->T_opt, count);
		showFILES(gbl, gbl->T_opt != j);
	    }
	    break;
#ifdef	apollo
	case 'U':
	    COMPLEMENT(gbl->U_opt);
	    showFILES(gbl, FALSE);
	    break;
#endif

#ifdef	Z_RCS_SCCS
	case 'V':		/* toggle sccs-version display */
	    showSCCS(gbl);
	    gbl->V_opt = !gbl->V_opt;
	    showFILES(gbl, TRUE);
	    break;

	case 'O':		/* show owner of file lock */
	    showSCCS(gbl);
	    gbl->O_opt = !gbl->O_opt;
	    showFILES(gbl, TRUE);
	    break;

	case 'Z':		/* toggle sccs-date display */
	    showSCCS(gbl);
	    gbl->Z_opt = -gbl->Z_opt;
	    showFILES(gbl, TRUE);
	    break;

	case 'z':		/* cancel sccs-display */
	    if (gbl->Z_opt) {
		gbl->Z_opt = 0;
		showFILES(gbl, TRUE);
	    }
	    break;
#endif /* Z_RCS_SCCS */

	case 'q':		/* quit this process */
	    if (lastc == 't')
		retouch(gbl, mark_W + 1);
	    else if (user_says(gbl, no_worry))
		gbl = 0;
	    break;

	    /* move work-area marker */
	case 'A':
	    count = -count;
	case 'a':
	    markset(gbl, (unsigned) (mark_W + count));
	    break;

	case CTL('R'):		/* modify read-expression */
	    while (dedread(gbl, &gbl->toscan, gbl->numfiles == 0)) {
		RING *tmp;
		if ((tmp = rescan(gbl, FALSE)) != NULL) {
		    gbl = tmp;
		    break;
		}
	    }
	    break;

	case 'R':		/* re-stat display-list */
	    gbl = rescan(gbl, TRUE);
	    break;

	case 'W':		/* re-stat window */
	    resleep(gbl, count, restat_W);
	    break;

	case 'w':		/* refresh window */
	    retouch(gbl, 0);
	    break;

	case 'l':		/* re-stat line */
	    resleep(gbl, count, restat_l);
	    break;

	case ' ':		/* clear workspace */
#if	!defined(CURSES_LIKE_BSD)
	    if (lastc == c)
		clearok(stdscr, TRUE);
#endif
	    retouch(gbl, mark_W + 1);
	    break;

	case 'r':
	case 's':
	    j = dlog_char(gbl, (int *) 0, 0);
	    if ((gbl->tagsort = (j == '+')) != 0)
		j = dlog_char(gbl, (int *) 0, 0);
	    if (!(j = sortget(gbl, j))) ;
	    else if (sortset(gbl, c, j)) {
#ifdef	Z_RCS_SCCS
		if (needSCCS(gbl, j))
		    showSCCS(gbl);
#endif /* Z_RCS_SCCS */
		dedsort(gbl);
		(void) to_file(gbl);
		showFILES(gbl, FALSE);
	    } else
		dedmsg(gbl, "unknown sort-key");
	    break;

	case 'C':
	    gbl->dateopt += 1;
	    if (gbl->dateopt > 2)
		gbl->dateopt = 0;
	    showFILES(gbl, FALSE);
	    break;

	case '#':		/* tag files with duplicated fields */
	    deduniq(gbl, count);
	    count_tags(gbl);
	    showFILES(gbl, FALSE);
	    break;

	    /* tag/untag specific files */
	case '+':
	    tag_entry(gbl, gbl->curfile, (unsigned) count);
	    downLINE(gbl, (unsigned) count);
	    showFILES(gbl, FALSE);
	    break;

	case '-':
	    untag_entry(gbl, gbl->curfile, (unsigned) count);
	    downLINE(gbl, (unsigned) count);
	    showFILES(gbl, FALSE);
	    break;

	case '_':
	    for_each_file(gbl, k)
		untag_entry(gbl, k, 1);
	    init_tags(gbl);
	    showFILES(gbl, FALSE);
	    break;

	case CTL('G'):
	    gbl->tag_opt = one_or_both(j = gbl->tag_opt, count);
	    if (gbl->tag_opt != j) {
		showWHAT(gbl);
		showC(gbl);
	    }
	    break;

	    /* edit specific fields */
	case '\'':
	    ReplayFind(
			  inline_nesting(gbl,
					 dlog_char(gbl, (int *) 0, FALSE)));
	    ReplayTrim();	/* chop off endc */
	    /* FALLTHRU */
	case '"':
	    inline_command(gbl, edit_inline(TRUE));
	    break;

	case 'p':
	case 'u':
	case 'g':
	case '=':
	case '<':
	case '>':
	case 'c':
	    inline_command(gbl, ReplayInit(inline_nesting(gbl, c)));
	    break;

	case 'm':
	    to_work(gbl, TRUE);
	    forkfile(gbl, ENV(PAGER), cNAME, TRUE + 1);
	    break;

	    /* page thru files in work area */
	case 'h':
	    dedtype(gbl, howami, -1, FALSE, FALSE, FALSE);
	    c = 't';		/* force work-clear if 'q' */
	    break;
	case 't':
	    if ((j = realstat(gbl, gbl->curfile, &sb)) >= 0)
		dedtype(gbl, cNAME, (int) gbl->curfile, (count != 1), (count
								       > 2), j);
	    break;

	case '%':		/* execute shell command with screen refresh */
	case '!':		/* execute shell command w/o screen refresh */
	    count = (c == '!') ? 0 : 2;		/* force refresh-sense */
	case '.':		/* re-execute last shell command */
	case ':':		/* edit last shell command */
	    deddoit(gbl, c, count);
	    break;

	case '*':		/* display last shell command(s) */
	    show_history(gbl, count);
	    break;

	case '/':
	case '?':
	case 'n':
	case 'N':		/* execute a search command */
	    dedfind(gbl, c);
	    break;

	case 'D':		/* toggle directory/filelist mode */
	    (void) strcpy(dpath, strcpy(tpath, gbl->new_wd));
	    for (;;) {
		RING *tmp;
		tree_visible = TRUE;
		gbl = ft_view(gbl, tpath, &c);
		tree_visible = FALSE;
		if (c == 'e' && !optInprocess)
		    new_process(gbl, tpath);
		else if ((tmp = new_tree(gbl, tpath, c)) != NULL) {
		    gbl = tmp;
		    break;
		}
	    }
	    break;

	case CTL('e'):		/* pad-edit */
	case CTL('v'):		/* pad-view */
	case 'e':
	case 'v':		/* enter new process with current file */
	    if (!optInprocess
		|| !realstat(gbl, gbl->curfile, &sb)
		|| iscntrl(c)) {
		gbl = run_editor(gbl, (c & 037) != CTL('e'), (int) iscntrl(c));
		break;
	    }
	    /*FALLTHRU */

	case 'E':		/* enter new directory on ring */
	    gbl = edit_directory(gbl);
	    break;

	case 'F':		/* move forward in directory-ring */
	case 'B':		/* move backward in directory-ring */
	    gbl = old_args(gbl, c, count);
	    break;

	case CTL('K'):		/* dump the current screen */
	    deddump(gbl);
	    break;

	case 'Y':		/* FIXME: reserve for vertical split */
	    beep();
	    break;

	case 'X':		/* split/join screen (1 or 2 viewports) */
	    gbl = splitVIEW(gbl);
	    break;

	case '\t':		/* tab to next viewport */
	    gbl = tab2VIEW(gbl);
	    break;

	default:
	    beep();
	};
	lastc = c;
    }

    to_exit(TRUE);
#if defined(HAVE_NEWTERM)
    if (do_select)
	ring_tags();
#endif
    ft_write();
    dlog_exit(SUCCESS);
    /*NOTREACHED */
    return (SUCCESS);
}
