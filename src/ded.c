#ifndef	lint
static	char	Id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/ded.c,v 12.2 1993/09/28 12:21:19 dickey Exp $";
#endif

/*
 * Title:	ded.c (directory-editor)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
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
#include	<signal.h>
#include	<errno.h>

#ifndef	EDITOR
#define	EDITOR	"/usr/ucb/vi"
#endif

#ifndef	BROWSE
#define	BROWSE	"/usr/ucb/view"
#endif

#ifndef	PAGER
#define	PAGER	"/usr/ucb/more"
#endif

#define	COMPLEMENT(opt) (opt) = !(opt)

public	int	debug	= FALSE;	/* generic debug-flag */
public	int	no_worry = -1;		/* don't prompt on quit */
public	int	in_screen;		/* TRUE if we have init'ed */

/*
 * Other, private main-module state:
 */
static	char	whoami[BUFSIZ],		/* my execution-path */
		*log_opt,		/* log-file option */
		*tree_opt,		/* my file-tree database */
		howami[BUFSIZ];		/* my help-file */

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Returns 0, 1 or 2 for a three-way toggle
 */
private	int	one_or_both(
	_ARX(int,	opt)
	_AR1(int,	val)
		)
	_DCL(int,	opt)
	_DCL(int,	val)
{
	if (val == 0)
		opt = 0;
	else if (val == 1)
		opt = !opt;
	else
		opt = 2;
	return (opt);
}

#ifdef	S_IFLNK
private	int	edithead(
	_ARX(RING *,	gbl)
	_ARX(char *,	dst)	/* receives destination-directory */
	_AR1(char *,	leaf)	/* receives destination-leaf */
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	dst)
	_DCL(char *,	leaf)
{
	auto	STAT	sb;
	auto	char	*s;

	if (cLTXT != 0) {
		dlog_comment("try to edit link-head \"%s\"\n", cLTXT);
		(void)pathcat(dst, gbl->new_wd, cLTXT);
		(void)strcpy(dst, pathhead(dst, &sb));
		dlog_comment("... becomes pathname  \"%s\"\n", dst);
		(void)strcpy(leaf, cLTXT);
		if ((s = strrchr(cLTXT, '/')) != NULL) {
#ifdef	apollo
			/* special hack for DSEE library */
			if (s[1] == '[') {
				leaf[s-cLTXT] = EOS;
				if (s = strrchr(leaf, '/')) {
					s++;
					while (*leaf++ = *s++);
				}
			} else
#endif	/* apollo */
			if (s[1] != EOS)
				(void)strcpy(leaf, ++s);
		}
		return (TRUE);
	}
	return (FALSE);		/* head would duplicate current directory */
}
#endif	/* S_IFLNK */

/************************************************************************
 *	public	utility procedures					*
 ************************************************************************/

/*
 * Exit from window mode
 */
public	void	to_exit _ONE(int,last)
{
	if (in_screen) {
		if (last) {
			clearmsg();
			refresh();
		}
		resetty();
#ifndef	SYSTEM5			/* patch: apollo 'endwin()' ? */
		endwin();
#endif
	}
}

/*
 * Determine if the given entry is a file, directory or none of these.
 */
public	int	realstat(
	_ARX(RING *,	gbl)
	_ARX(int,	inx)
	_AR1(STAT *,	sb)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inx)
	_DCL(STAT *,	sb)
{
	register j = gSTAT(inx).st_mode;

#ifdef	S_IFLNK
	if (isLINK(j)) {
		j = (stat(gNAME(inx), sb) >= 0) ? sb->st_mode : 0;
	} else
		sb->st_mode = 0;
#endif
	if (isFILE(j))	return(0);
	if (isDIR(j))	return(1);
	return (-1);
}

/*
 * Fatal-error exit from this process
 */
public	void	failed _ONE(char *,msg)
{
	if (debug) {
		FPRINTF(stderr, "failed?");
		(void) cmdch((int *)0);
	}
	to_exit(msg != 0);
	if (msg)
		FPRINTF(stderr, "-------- \n?? %-79s\n-------- \n", msg);
	if (msg) {
		(void)fflush(stderr);
		abort();
	}

	dlog_exit(FAIL);
}

/*
 * Prompt user for yes/no response
 */
public	int	user_says(
	_ARX(RING *,	gbl)
	_AR1(int,	ok)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	ok)
{
	register char	*s;
	int	y,x;
	static	DYN	*reply;

	if (!ok) {
		to_work(gbl,TRUE);
		PRINTW("Are you sure (y/n)? ");
		getyx(stdscr,y,x);
		clrtobot();
		move(y,x);
		refresh();
		dyn_init(&reply, 8);
		if ((s = dlog_string(&reply,(DYN **)0,NO_HISTORY,EOS,-8)) != NULL)
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
public	int	findFILE(
	_ARX(RING *,	gbl)
	_AR1(char *,	name)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
{
	register int j;
	for (j = 0; j < gbl->numfiles; j++)
		if (name == gNAME(j))
			return (j);
	return (0);			/* give up, set to beginning of list */
}

#ifdef	Z_RCS_SCCS
private	int	needSCCS(
	_ARX(RING *,	gbl)
	_AR1(int,	c)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	c)
{
	return (!gbl->Z_opt && (strchr("vyZz",c) != 0));
}

public	void	showSCCS _ONE(RING *,gbl)
{
	register int j;

	if (!gbl->Z_opt) {		/* catch up */
		to_work(gbl,TRUE);
		gbl->Z_opt = -1;
		for (j = 0; j < gbl->numfiles; j++) {
			register FLIST *f = &(gbl->flist[j]);
			if (!f->z_time) {
				statSCCS(gbl, gNAME(j), f);
				blip((f->z_time != 0) ? '*' : '.');
			}
		}
	}
}
#endif	/* Z_RCS_SCCS */

/*
 * Repaint the screen
 */
public	void	retouch(
	_ARX(RING *,	gbl)
	_AR1(int,	row)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	row)
{
int	y,x;
#ifdef	apollo
	if (resizewin()) {
		dlog_comment("resizewin(%d,%d)\n", LINES, COLS);
		markset(gbl, mark_W,FALSE);
		showFILES(gbl,FALSE,TRUE);
		return;
	}
#endif	/* apollo */
	getyx(stdscr,y,x);
	move(mark_W+1,0);
	clrtobot();
	move(y,x);
	wrepaint(stdscr,row);
}

/*
 * Process the given function in a repeat-loop which is interruptable.
 */
public	void	resleep(
	_ARX(RING *,	gbl)
	_ARX(int,	count)
	_FN1(void,	func,	(_AR1(RING*,g)))
		)
	_DCL(RING *,	gbl)
	_DCL(int,	count)
	_DCL(void,	(*func)())
{
	register int	interrupted = 1,
			last	= count;

	while (count-- > 1) {
		move(LINES-1,0);
		PRINTW("...waiting (%d of %d) ...", last-count, last);
		clrtoeol();
		(*func)(gbl);
		sleep(3);
		if ((interrupted = dedsigs(TRUE)) != 0)
			break;
	}
	clearmsg();
	if (interrupted)
		(*func)(gbl);
	else
		showC(gbl);
}

/*
 * Use the 'dedring()' module to switch to a different file-list
 */
private	RING *	new_args(
	_ARX(RING *,	gbl)
	_ARX(char *,	path)
	_ARX(int,	cmd)
	_ARX(int,	count)
	_ARX(int,	flags)
	_ARX(int,	set_pattern)
	_AR1(char *,	pattern)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(int,	cmd)
	_DCL(int,	count)
	_DCL(int,	flags)
	_DCL(int,	set_pattern)
	_DCL(char *,	pattern)
{
	RING *	ok;

	if (flags & 1)
		markC(gbl,TRUE);
	clear_work();
	if ((ok = dedring(gbl, path, cmd, count, set_pattern, pattern)) != 0) {
		redoVIEW(gbl = ok);
		(void)to_file(gbl);
		count_tags(gbl);
		showFILES(gbl,TRUE,TRUE);
	}
	(void)chdir(gbl->new_wd);
	dlog_comment("chdir %s\n", gbl->new_wd);
	if (!ok && (flags & 2))
		showC(gbl);
	return ok;
}

/*
 * Set list to an old set of arguments
 */
private	RING *	old_args(
	_ARX(RING *,	gbl)
	_ARX(int,	cmd)	/* 'F' or 'B' */
	_AR1(int,	count)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	cmd)
	_DCL(int,	count)
{
	auto	RING *	new;

	new = new_args(gbl, gbl->new_wd, cmd, count, 0, FALSE, (char *)0);
	if (new != 0)
		gbl = new;
	else
		showC (gbl);	/* try to recover */
	return gbl;
}

/*
 * Open a (new) argument-list, setting the scan-pattern
 */
private	RING *	pattern_args(
	_ARX(RING *,	gbl)
	_AR1(char *,	path)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
{
	char	*pattern = 0;
	RING	*new;

	while (dedread(gbl, &pattern, FALSE)) {
		if ((new = new_args(gbl, path, 'E', 1, 3, TRUE, pattern)) != NULL)
			return (new);
	}
	return (0);
}

	/* re-scan argument list */
private	RING *	rescan(
	_ARX(RING *,	gbl)
	_AR1(int,	fwd)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	fwd)
{
	char	*cur_name = gbl->numfiles ? cNAME : 0;

	to_work(gbl,TRUE);
	init_tags(gbl);
	if (dedscan(gbl)) {
		gbl->curfile = cur_name ? findFILE(gbl, cur_name) : 0;
		(void)to_file(gbl);
		showFILES(gbl,TRUE,TRUE);
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
public	char *	fixname(
	_ARX(RING *,	gbl)
	_AR1(int,	j)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	j)
{
	static	char	nbfr[BUFSIZ];
	(void)ded2string(gbl, nbfr, sizeof(nbfr), gNAME(j), TRUE);
	return (nbfr);
}

/*
 * Adjust mtime-field so that chmod, chown do not alter it.
 * This fixes Apollo/NFS kludges!
 */
public	void	fixtime(
	_ARX(RING *,	gbl)
	_AR1(int,	j)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	j)
{
	if (setmtime(gNAME(j), gSTAT(j).st_mtime) < 0)	warn(gbl, "utime");
}

/*
 * Spawn a subprocess, wait for completion.
 */
private	void	forkfile(
	_ARX(RING *,	gbl)
	_ARX(char *,	arg0)
	_ARX(char *,	arg1)
	_AR1(int,	option)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	arg0)
	_DCL(char *,	arg1)
	_DCL(int,	option)
{
	char	quoted[MAXPATHLEN];

	*quoted = EOS;
	catarg(quoted, arg1);

	resetty();
	dlog_comment("execute %s %s\n", arg0, arg1);
	if (execute(arg0, quoted) < 0)
		warn(gbl, arg0);
	dlog_elapsed();
	rawterm();

	switch (option) {
	case TRUE+1:
		dedwait(gbl, TRUE);
		/* fall-thru */
	case TRUE:
		retouch(gbl, 0);
		restat(gbl,FALSE);
	}
}

/*
 * Enter an editor (separate process) for the current-file/directory.
 */
private	RING *	editfile(
	_ARX(RING *,	gbl)
	_ARX(int,	readonly)
	_AR1(int,	extended)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	readonly)
	_DCL(int,	extended)
{
	STAT	sb;
	char	*editor = (readonly ? ENV(BROWSE) : ENV(EDITOR));
	char	tpath[BUFSIZ];

	dlog_name(cNAME);
	switch (realstat(gbl, gbl->curfile, &sb)) {
	case 0:	/* edit-file */
		to_work(gbl,TRUE);
		if (extended) {
			if (padedit(cNAME, readonly, editor) < 0)
				beep();
			restat(gbl,FALSE);
		} else
			forkfile(gbl, editor, cNAME, TRUE);
		break;
	case 1:	/* edit-directory */
		if (extended) {
			RING *new;
			if ((new = pattern_args(gbl, pathcat(tpath, gbl->new_wd, cNAME))) != NULL)
				gbl = new;
		} else {
			to_work(gbl,TRUE);
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
private	RING *	edit_directory _ONE(RING *,gbl)
{
	RING	*new = gbl;
	char	tpath[MAXPATHLEN];
	char	dpath[MAXPATHLEN];
	STAT	sb;

	if (realstat(gbl, gbl->curfile, &sb) == 1) {
		if (!(new = new_args(gbl,
			pathcat(tpath, gbl->new_wd, cNAME),
			'E', 1, 3, FALSE, (char *)0)))
			return gbl;

#ifdef	S_IFLNK		/* try to edit head-directory of symbolic-link */
	} else if (edithead(gbl, tpath, dpath)) {
		if (strcmp(tpath, gbl->new_wd)
		&&  !(new = new_args(gbl, tpath, 'E', 1, 3, FALSE, (char *)0)))
			return gbl;

		scroll_to_file(new, findFILE(new, txtalloc(dpath)));
#endif	/* S_IFLNK */

	} else
		dedmsg(gbl, "not a directory");

	return new;
}

/*
 * Invoke a new file-list from the directory-tree display, cleaning up if
 * it fails.
 */
private	RING *	new_tree(
	_ARX(RING *,	gbl)
	_ARX(char *,	path)
	_AR1(int,	cmd)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(int,	cmd)
{
	RING *	new;

	if (iscntrl(cmd))
		new = pattern_args(gbl, path);
	else
		new = new_args(gbl, path, 'E', 1, 0, FALSE, (char *)0);

	return new;
}

/*
 * Invoke a new copy of 'ded'
 */
private	void	new_process(
	_ARX(RING *,	gbl)
	_AR1(char *,	path)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
{
	int	y,x;

	getyx(stdscr, y, x);
	if (++y >= LINES)	y = LINES-1;
	move(y, x-x);
	clrtobot();
	move(y, 0);
	refresh();
	ft_write();
	dlog_close();
	forkfile(gbl, whoami, path, FALSE);
	dlog_reopen();
	wrepaint(stdscr,0);
	ft_read(gbl->new_wd, tree_opt);
}

private	void	trace_pipe _ONE(char *,arg)
{
	if (debug) {
		if (debug > 1) {
			FPRINTF(stderr, "%s\n", arg);
			(void)fflush(stderr);
		} else
			blip('#');
	}
}

private	int	x_scroll (_AR0)
{
	return ((1 + (COLS/4)/10) * 10);
}

/************************************************************************
 *	main program							*
 ************************************************************************/

void	usage(_AR0)
{
	auto	char	tmp[BUFSIZ];
	static	char	*tbl[] = {
			"Usage: ded [options] [filespecs]",
			"(filespecs may be read from pipe)",
			"",
			"Options which alter initial display fields:",
			"  -A       show \".\" and \"..\" names",
			"  -G       show group-name instead of user-name",
			"  -I       show inode field",
#ifdef	apollo_sr10
			"  -O       show apollo object-types",
#endif
			"  -P       show protection in octal",
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
			"  -c FILE  read DED commands from FILE",
			"  -d       (debug)",
			"  -l FILE  write DED commands to log-FILE",
			"  -n       disable \"are you sure\" on quit",
			"  -t DIR   read \".ftree\"-file from directory DIR",
			(char *)0
			};
	register char	**p;

	setbuf(stderr,tmp);
	for (p = tbl; *p; p++)
		FPRINTF(stderr, "%s\n", *p);
	FPRINTF(stderr, "\nSort KEY-options are: \"%s\"\n", sortc);

	dlog_exit(FAIL);
}

private	int	inline_nesting _ONE(int,c)
{
	if (c == 'c') {
		ReplayTopC(c);
		c = dlog_char((int *)0,FALSE);
	} else {
		switch (c) {
#ifdef	S_IFLNK
		case 'l':
#endif	/* S_IFLNK */
		case 'd':
		case 'f':
		case 'L':	ReplayTopC('c');	break;
		default:	ReplayTopC(EOS);
		}
	}
	return (c);
}

private	void	inline_command(
	_ARX(RING *,	gbl)
	_AR1(int,	c)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	c)
{
	switch (c) {
	case 'p':	editprot(gbl);		break;
	case 'u':	edit_uid(gbl);		break;
	case 'g':	edit_gid(gbl);		break;
	case '=':	editname(gbl);		break;
#ifdef	S_IFLNK
	case '<':
	case '>':	editlink(gbl, c);	break;
	case 'l':
#endif	/* S_IFLNK */
	case 'd':
	case 'f':
	case 'L':	dedmake(gbl, c);	break;

	default:	{
		char	temp[80];
		FORMAT(temp, "no inline command (%c)", c ? c : '?');
		dedmsg(gbl, temp);
		}
	}
	(void)edit_inline(FALSE);
}

/*ARGSUSED*/
_MAIN
{
#include	"version.h"

	register int	j;
	auto	STAT	sb;
	auto	int	c,
			count,
			lastc	= '?';
	auto	char	tree_bfr[BUFSIZ],
			tpath[BUFSIZ],
			dpath[BUFSIZ];

	RING	*gbl = ring_alloc();

	(void)sortset(gbl, 's', 'n');
	(void)sscanf(version, "%*s %s %s", tpath, dpath);
	FPRINTF(stderr, "DED Directory Editor (%s %s)\r\n", tpath, dpath);
	/* show when entering process */
	(void)fflush(stdout);

	while ((c = getopt(argc, argv, "aGIOPSTUZc:l:r:s:zdt:n")) != EOF)
	switch (c) {
	case 'a':	COMPLEMENT(gbl->A_opt);	break;
	case 'G':	COMPLEMENT(gbl->G_opt);	break;
	case 'I':	COMPLEMENT(gbl->I_opt);	break;
#ifdef	apollo_sr10
	case 'O':	COMPLEMENT(gbl->O_opt);	break;
#endif
	case 'P':	COMPLEMENT(gbl->P_opt);	break;
	case 'S':	COMPLEMENT(gbl->S_opt);	break;
	case 'T':	COMPLEMENT(gbl->T_opt);	break;
#ifdef	apollo
	case 'U':	COMPLEMENT(gbl->U_opt);	break;
#endif
#ifdef	Z_RCS_SCCS
	case 'Z':	gbl->Z_opt = 1;		break;
	case 'z':	gbl->Z_opt = -1;	break;
#endif
	case 'c':	dlog_read(optarg);			break;
	case 'l':	log_opt = dlog_open(optarg,argc,argv);	break;
	case 's':
	case 'r':	if (!sortset(gbl, c,*optarg))	usage();
#ifdef	Z_RCS_SCCS
			if (needSCCS(gbl,c))
				gbl->Z_opt = -1;
#endif	/* Z_RCS_SCCS */
			break;
	case 'd':	debug++;		break;
	case 't':	tree_opt = optarg;	break;
	case 'n':	no_worry = TRUE;	break;
	default:	usage();
			/*NOTREACHED*/
	}

	if (!tree_opt)	tree_opt = gethome();
	else		abspath(tree_opt = strcpy(tree_bfr, tree_opt));
	if (!getwd(old_wd))
		(void)strcpy(old_wd, ".");
	ft_read(old_wd, tree_opt);

	/* find which copy I am executing from, for future use */
	if (which(whoami, sizeof(whoami), argv[0], old_wd) <= 0)
		failed("which-path");

	/* my help-file lives where the binary does */
	FORMAT(howami, "%s.hlp", whoami);

	/* pass options to lower-level processes of ded */
	(void)strcat(strcat(whoami, " -t"), tree_opt);
	if (log_opt)
		(void)strcat(strcat(whoami, " -l"), log_opt);
	(void)strcat(whoami, " -n");

	if (!isatty(fileno(stdin))) {
		if (optind < argc)
			FPRINTF(stderr, "? ignored arguments, using pipe\n");
		argc = fp2argv(stdin, &argv, trace_pipe);
		optind = 0;
		for (j = 0; j < argc; j++) {
			register char *s = argv[j];
			register int  len = strlen(s) - 1;
			if (len >= 0 && s[len] == '\n')
				s[len] = EOS;	/* trim trailing newline */
		}
	}

	/* protect against pipes */
	if (!isatty(fileno(stdin))) {
		FILE	*p = freopen("/dev/tty", "r", stdin);
		if (!p)	failed("reopen stdin");
		*stdin = *p;
	}

	(void)dedsigs(TRUE);
	if (!initscr())			failed("initscr");
	in_screen = TRUE;
	if (LINES > BUFSIZ || COLS > BUFSIZ) {
	char	bfr[80];
		FORMAT(bfr, "screen too big: %d by %d\n", LINES, COLS);
		failed(bfr);
	}
	rawterm();

	argc -= optind;
	argv += optind;
	ring_args(gbl, argc, argv);

	mark_W = (LINES/2);
	openVIEW(gbl);

	while (gbl != 0) { switch (c = dlog_char(&count,1)) {
			/* scrolling */
	case ARO_UP:
	case '\b':
	case 'k':	upLINE(gbl, count);
			break;

	case ARO_DOWN:
	case '\n':
	case 'j':	downLINE(gbl, count);
			break;

	case 'f':	scrollVIEW(gbl, count);
			break;

	case 'b':	scrollVIEW(gbl, -count);
			break;

	case ARO_LEFT:	if (gbl->Xbase > 0) {
				if ((gbl->Xbase -= x_scroll() * count) < 0)
					gbl->Xbase = 0;
				showFILES(gbl,FALSE,FALSE);
			} else
				dedmsg(gbl, "already at left margin");
			break;

	case ARO_RIGHT:	if ((j = (gbl->Xbase + (x_scroll() * count))) < 990) {
				gbl->Xbase = j;
				showFILES(gbl,FALSE,FALSE);
			} else
				beep();
			break;

			/* cursor-movement in-screen */
	case 'H':	gbl->curfile = baseVIEW(gbl);
			showC(gbl);
			break;
	case 'M':	gbl->curfile = (baseVIEW(gbl)+lastVIEW(gbl))/2;
			showC(gbl);
			break;
	case 'L':	gbl->curfile = lastVIEW(gbl);
			showC(gbl);
			break;
	case '^':	top2VIEW(gbl);
			break;

			/* display-toggles */
#ifdef	S_IFLNK
	case '@':	COMPLEMENT(gbl->AT_opt);
			count = 0;
			to_work(gbl,TRUE);
			PRINTW(" ");	/* cheat the optimizer */
			refresh();
			for (j = 0; j < gbl->numfiles; j++) {
				if (gLTXT(j)) {
					blip('@');
					statLINE(gbl, j);
					count++;
				} else
					blip('.');
			}
			if (count)
				showFILES(gbl,TRUE,TRUE);
			else
				showC(gbl);
			break;
#endif	/* S_IFLNK */

			/* note that '.' and '..' may be the only files! */
	case '&':	COMPLEMENT(gbl->A_opt); /* sorry about inconsistency */
			gbl = rescan(gbl, TRUE);
			break;
	case 'G':	gbl->G_opt = one_or_both(j = gbl->G_opt,count);
			showFILES(gbl,(gbl->G_opt != 2) != (j != 2), FALSE);
			break;
	case 'I':	gbl->I_opt = one_or_both(j = gbl->I_opt,count);
			showFILES(gbl,gbl->I_opt != j, FALSE);
			break;
#ifdef	apollo_sr10
	case 'O':	COMPLEMENT(gbl->O_opt);
			showFILES(gbl, TRUE, FALSE);
			break;
#endif
	case 'P':	gbl->P_opt = one_or_both(j = gbl->P_opt,count);
			showFILES(gbl,gbl->P_opt != j, FALSE);
			break;
	case 'S':	gbl->S_opt = one_or_both(j = gbl->S_opt,count);
			showFILES(gbl,gbl->S_opt != j, FALSE);
			break;
	case 'T':	gbl->T_opt = one_or_both(j = gbl->T_opt,count);
			showFILES(gbl,gbl->T_opt != j, FALSE);
			break;
#ifdef	apollo
	case 'U':	COMPLEMENT(gbl->U_opt);
			showFILES(gbl, FALSE, FALSE);
			break;
#endif

#ifdef	Z_RCS_SCCS
	case 'V':	/* toggle sccs-version display */
			j = !gbl->Z_opt;
			showSCCS(gbl);
			gbl->V_opt = !gbl->V_opt;
			showFILES(gbl,TRUE,j);
			break;

	case 'Y':	/* show owner of file lock */
			j = !gbl->Z_opt;
			showSCCS(gbl);
			gbl->Y_opt = !gbl->Y_opt;
			showFILES(gbl,TRUE,j);
			break;

	case 'Z':	/* toggle sccs-date display */
			j = !gbl->Z_opt;
			showSCCS(gbl);
			gbl->Z_opt = -gbl->Z_opt;
			showFILES(gbl,TRUE,j);
			break;

	case 'z':	/* cancel sccs-display */
			if (gbl->Z_opt) {
				gbl->Z_opt = 0;
				showFILES(gbl,TRUE,FALSE);
			}
			break;
#endif	/* Z_RCS_SCCS */

	case 'q':	/* quit this process */
			if (lastc == 't')
				retouch(gbl, mark_W+1);
			else if (user_says(gbl, no_worry))
				gbl = 0;
			break;

			/* move work-area marker */
	case 'A':	count = -count;
	case 'a':
			markset(gbl, mark_W + count,TRUE);
			break;

	case CTL('R'):	/* modify read-expression */
			while (dedread(gbl, &gbl->toscan, gbl->numfiles == 0)) {
				RING *new;
				if ((new = rescan(gbl, FALSE)) != NULL) {
					gbl = new;
					break;
				}
			}
			break;

	case 'R':	/* re-stat display-list */
			gbl = rescan(gbl, TRUE);
			break;

	case 'W':	/* re-stat window */
			resleep(gbl, count,restat_W);
			break;

	case 'w':	/* refresh window */
			retouch(gbl, 0);
			break;

	case 'l':	/* re-stat line */
			resleep(gbl, count,restat_l);
			break;

	case ' ':	/* clear workspace */
			retouch(gbl, mark_W+1);
			break;

	case 'r':
	case 's':	j = dlog_char((int *)0,0);
			if ((gbl->tagsort = (j == '+')) != 0)
				j = dlog_char((int *)0,0);
			if (!(j = sortget(gbl, j)))
				;
			else if (sortset(gbl, c,j)) {
#ifdef	Z_RCS_SCCS
				if (needSCCS(gbl,j))
					showSCCS(gbl);
#endif	/* Z_RCS_SCCS */
				dedsort(gbl);
				(void)to_file(gbl);
				showFILES(gbl,FALSE,FALSE);
			} else
				dedmsg(gbl, "unknown sort-key");
			break;

	case 'C':	gbl->dateopt += 1;
			if (gbl->dateopt > 2)	gbl->dateopt = 0;
			showFILES(gbl,FALSE,FALSE);
			break;

	case '#':	/* tag files with duplicated fields */
			deduniq(gbl, count);
			count_tags(gbl);
			showFILES(gbl,FALSE,TRUE);
			break;

			/* tag/untag specific files */
	case '+':	while (count-- > 0) {
				tag_entry(gbl,gbl->curfile);
				if (!showDOWN(gbl))
					break;
			}
			break;

	case '-':	while (count-- > 0) {
				untag_entry(gbl,gbl->curfile);
				if (!showDOWN(gbl))
					break;
			}
			break;

	case '_':	for (j = 0; j < gbl->numfiles; j++)
				untag_entry(gbl,j);
			init_tags(gbl);
			showFILES(gbl,FALSE,FALSE);
			break;

	case CTL('G'):	gbl->tag_opt = one_or_both(j = gbl->tag_opt,count);
			if (gbl->tag_opt != j) {
				showWHAT(gbl);
				showC(gbl);
			}
			break;

			/* edit specific fields */
	case '\'':	ReplayFind(inline_nesting(dlog_char((int *)0,FALSE)));
			ReplayTrim();	/* chop off endc */
	case '"':	inline_command(gbl, edit_inline(TRUE));
			break;

	case 'p':
	case 'u':
	case 'g':
	case '=':
	case '<':
	case '>':
	case 'c':	inline_command(gbl, ReplayInit(inline_nesting(c)));
			break;

	case CTL('e'):	/* pad-edit */
	case CTL('v'):	/* pad-view */
	case 'e':
	case 'v':	/* enter new process with current file */
			gbl = editfile(gbl, (c & 037) != CTL('e'), (int)iscntrl(c));
			break;

	case 'm':	to_work(gbl,TRUE);
			forkfile(gbl, ENV(PAGER), cNAME, TRUE+1);
			break;

			/* page thru files in work area */
	case 'h':	dedtype(gbl, howami,-1,FALSE,FALSE,FALSE);
			c = 't';	/* force work-clear if 'q' */
			break;
	case 't':	if ((j = realstat(gbl, gbl->curfile, &sb)) >= 0)
				dedtype(gbl, cNAME,gbl->curfile,(count != 1),(count>2),j);
			break;

	case '%':	/* execute shell command with screen refresh */
	case '!':	/* execute shell command w/o screen refresh */
			count = (c == '!') ? 0 : 2; /* force refresh-sense */
	case '.':	/* re-execute last shell command */
	case ':':	/* edit last shell command */
			deddoit(gbl, c,count);
			break;

	case '*':	/* display last shell command */
			dedshow(gbl, "Command=", dyn_string(gbl->cmd_sh));
			showC(gbl);
			break;

	case '/':
	case '?':
	case 'n':
	case 'N':	/* execute a search command */
			dedfind(gbl, c);
			break;

	case 'D':	/* toggle directory/filelist mode */
			(void)strcpy(dpath, strcpy(tpath, gbl->new_wd) );
			for (;;) {
				RING *new;
				gbl = ft_view(gbl, tpath, &c);
				if (c == 'e')
					new_process(gbl, tpath);
				else if ((new = new_tree(gbl, tpath, c)) != NULL) {
					gbl = new;
					break;
				}
			}
			break;

	case 'E':	/* enter new directory on ring */
			gbl = edit_directory(gbl);
			break;

	case 'F':	/* move forward in directory-ring */
	case 'B':	/* move backward in directory-ring */
			gbl = old_args(gbl, c, count);
			break;

	case CTL('K'):	/* dump the current screen */
			deddump(gbl);
			break;

	case 'X':	/* split/join screen (1 or 2 viewports) */
			gbl = splitVIEW(gbl);
			break;

	case '\t':	/* tab to next viewport */
			gbl = tab2VIEW(gbl);
			break;

	default:	beep();
	}; lastc = c; }

	to_exit(TRUE);
	ft_write();
	dlog_exit(SUCCESS);
	/*NOTREACHED*/
}
