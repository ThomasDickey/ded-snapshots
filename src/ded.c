#ifndef	lint
static	char	what[] = "$Id: ded.c,v 4.3 1989/10/04 16:46:48 dickey Exp $";
#endif	lint

/*
 * Title:	ded.c (directory-editor)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * $Log: ded.c,v $
 * Revision 4.3  1989/10/04 16:46:48  dickey
 * added -a, -O options
 * added &, O toggles
 * added o, O sorts
 *
 *		Revision 4.2  89/08/25  08:52:50  dickey
 *		added new procedures 'scroll_to_stat()' and 'scroll_to_file()'
 *		so 'E'-command on link can go to link-target.
 *		
 *		Revision 4.1  89/08/25  08:22:15  dickey
 *		use 'wrepaint()' rather than savewin/unsavewin.  added
 *		arg to 'realstat()' for 'E'-enhancement.
 *		
 *		Revision 4.0  89/08/22  16:31:22  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.2  89/08/22  16:31:22  dickey
 *		if user tries to apply 'E' command to symbolic-link-to-file,
 *		edit instead the directory containing the target file.
 *		
 *		Revision 3.1  89/08/11  14:22:57  dickey
 *		added/used procedure 'move2row()'
 *		
 *		Revision 3.0  89/06/06  08:38:13  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.4  89/06/06  08:38:13  dickey
 *		made blip-call for 'Z' toggle show results like '#'.
 *		
 *		Revision 2.3  89/05/31  08:21:11  dickey
 *		revised/updated 'usage()'
 *		
 *		Revision 2.2  89/05/26  14:05:00  dickey
 *		corrected last mod so failed-rescan keeps original
 *		name to find in resulting list
 *		
 *		Revision 2.1  89/05/26  13:38:22  dickey
 *		added CTL/R command to control read-selection
 *		
 *		Revision 2.0  89/04/03  09:41:04  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.65  89/04/03  09:41:04  dickey
 *		use of 'showFILES()' in 'restat_W()' did not work (?).  Recoded
 *		using 'showLINE()' and 'showC()'.
 *		
 *		Revision 1.64  89/03/24  08:36:36  dickey
 *		added "-c" (command-script) option, and changed version to RCS
 *		format using sscanf hack.
 *		
 *		Revision 1.63  89/03/15  09:12:02  dickey
 *		sccs2rcs keywords
 *		
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
extern	char	*dlog_open();
extern	char	*pathcat();
extern	char	*txtalloc();
extern	char	**vecalloc();

#ifndef	EDITOR
#define	EDITOR	"/usr/ucb/vi"
#endif	EDITOR

#ifndef	BROWSE
#define	BROWSE	"/usr/ucb/view"
#endif	BROWSE

#ifndef	PAGER
#define	PAGER	"/usr/ucb/more"
#endif	PAGER

#define	MAXVIEW	2		/* number of viewports */
#define	MINLIST	2		/* minimum length of file-list + header */
#define	MINWORK	3		/* minimum size of work-area */

int	debug	= FALSE;	/* generic debug-flag */

/*
 * Per-viewport main-module state:
 */
typedef	struct	{
		int	Yhead;		/* beginning of viewport (row)	*/
		int	Ybase;		/* beginning of viewport (file)	*/
		char	*cname;		/* arg for 'findFILE()		*/
		char	*path;		/* ...so dedring can identify	*/
	} VIEW;

static	VIEW	viewlist[MAXVIEW];

static	int	Yhead = 0,		/* first line of viewport	*/
		Ylast,			/* last visible file on screen	*/
		Ynext;			/* marks end of viewport	*/
static	int	curview,		/* 0..MAXVIEW			*/
		maxview;		/* current number of viewports	*/

static	int	Xscroll;		/* amount by which to left/right */
static	int	tag_count;		/* number of tagged files */

/*
 * Other, private main-module state:
 */
static	int	in_screen;		/* TRUE if we have successfully init'ed */
static	char	whoami[BUFSIZ],		/* my execution-path */
		*log_opt,		/* log-file option */
		*tree_opt,		/* my file-tree database */
		howami[BUFSIZ];		/* my help-file */

static	char	sortc[] = ".cdgGilnNoOprstTuUwyvzZ";/* valid sort-keys */
					/* (may correspond with cmds) */

/************************************************************************
 *	local procedures						*
 ************************************************************************/
static
sortset(ord,opt)
{
#ifndef	apollo_sr10
	if (strchr("oO", opt) != 0)
		opt = '?';
#endif
#ifndef	Z_RCS_SCCS
	if (strchr("vyzZ", opt) != 0)
		opt = '?';
#endif	Z_RCS_SCCS
	if (strchr(sortc, opt) != 0) {
		dateopt = opt == 'c'  ? 1 : (opt == 'r' ? 0 : 2);
		sortopt = opt;
		sortord = (ord == 'r');
		return(TRUE);
	}
	return(FALSE);
}

/*
 * Set 'Ylast' as a function of the current viewport and our position in it.
 * Also, set 'Ynext' to the row number of the first line after the viewport.
 */
static
viewset()
{
	register int	j	= curview + 1;

	Ynext	= (j >= maxview) ? mark_W : viewlist[j].Yhead;
	Ylast	= Ynext + Ybase - (Yhead + MINLIST);
	if (Ylast >= numfiles)	Ylast = numfiles-1;
}

#ifdef	S_IFLNK
static
edithead(dst, src)
char	*dst,*src;
{
	extern	char		*pathhead();
	auto	struct	stat	sb;

	if (src != 0) {
		dlog_comment("try to edit link-head \"%s\"\n", src);
		(void)strcpy(dst, pathhead(strcpy(dst, src), &sb));
		abspath(dst);
		if (strcmp(dst, new_wd))
			return (TRUE);
	}
	return (FALSE);		/* head would duplicate current directory */
}

/* after we edit the head-directory, try to find the link-target. */
static
scroll_to_stat(sb)
struct	stat	*sb;
{
	register int	j;
	for (j = 0; j < numfiles; j++) {
		if (xSTAT(j).st_dev == sb->st_dev
		&&  xSTAT(j).st_ino == sb->st_ino) {
			scroll_to_file(j);
			return;
		}
	}
}
#endif	S_IFLNK

/************************************************************************
 *	public	utility procedures					*
 ************************************************************************/

/*
 * Translate an index into the file-list to a row-number in the screen for the
 * current viewport.
 */
int
file2row(n)
{
	return ((n - Ybase) + Yhead + 1);
}

/*
 * Move to the indicated row; return FALSE if it does not correspond to a
 * currently-displayed line.
 */
move2row(n,col)
{
	if (n >= Ybase && n <= Ylast) {
		move(file2row(n), col);
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Exit from window mode
 */
to_exit(last)
{
	if (in_screen) {
		if (last) {
			move(LINES-1,0);
			clrtoeol();
			refresh();
		}
		resetty();
#ifndef	SYSTEM5			/* patch: apollo 'endwin()' ? */
		endwin();
#endif
	}
}

/*
 * Clear the work-area, and move the cursor there.
 */
clear_work()
{
	move(mark_W + 1, 0);
	clrtobot();
	move(mark_W + 1, 0);
	refresh();
}

/*
 * Clear the work area, move the cursor there after setting marker
 */
to_work()
{
	markC(TRUE);
	clear_work();
}

/*
 * Scroll, as needed, to put current-file in the window
 */
to_file()
{
	register int	code;
	viewset();		/* ensure that Ylast is up to date */
	code	= ((curfile < Ybase)
		|| (curfile > Ylast));
	while (curfile > Ylast)	forward(1);
	while (curfile < Ybase)	backward(1);
	return(code);
}

scroll_to_file(inx)
{
	if (curfile != inx) {
		curfile = inx;
		if (to_file())
			showFILES();
		else
			showC();
	}
}

/*
 * Move the workspace marker.  If we are in split-screen mode, also adjust the
 * length of the current viewport.  Finally, re-display the screen.
 */
markset(num)
{
	int	lo = (Yhead + MINLIST * (maxview - curview)),
		hi = (LINES - MINWORK);

	if (num < lo)	num = lo;

	if (curview < (maxview-1)) {	/* not the last viewport */
		int	next_W = viewlist[curview+1].Yhead;
		if (num > hi) {		/* multiple-adjust */
			mark_W = hi;
			next_W += (num - hi);
			if (curview < (maxview-2))
				hi = viewlist[curview+2].Yhead;
			hi -= MINLIST;
			if (next_W > hi)
				next_W = hi;
		} else if (Yhead + MINLIST <= (hi = next_W + (num - mark_W))) {
			next_W = hi;
			mark_W = num;
		}
		viewlist[curview+1].Yhead = next_W;
	} else {			/* in the last viewport */
		if (num > hi)	num = hi;
		mark_W = num;
	}

	(void)to_file();
	showFILES();
}

/*
 * Determine if the given entry is a file, directory or none of these.
 */
realstat(inx, sb)
struct	stat	*sb;
{
register j = xSTAT(inx).st_mode;

#ifdef	S_IFLNK
	if (isLINK(j)) {
		j = (stat(xNAME(inx), sb) >= 0) ? sb->st_mode : 0;
	} else
		sb->st_mode = 0;
#endif	S_IFLNK
	if (isFILE(j))	return(0);
	if (isDIR(j))	return(1);
	return (-1);
}

/*
 * Print an error/warning message
 */
dedmsg(msg)
char	*msg;
{
	move(LINES-1,0);
	PRINTW("** %s", msg);
	clrtoeol();
	showC();
	dlog_comment("(dedmsg) %s\n", msg);
}

warn(msg)
char	*msg;
{
extern	int	errno;
extern	char	*sys_errlist[];
	move(LINES-1,0);
	PRINTW("** %s: %s", msg, sys_errlist[errno]);
	clrtoeol();
	showC();
	dlog_comment("(warn) %s: %s\n", msg, sys_errlist[errno]);
}

/*
 * Wait for the user to hit a key before the next screen is shown.  This is
 * used when we have put a message up and may be going back to the
 * directory tree display.
 */
waitmsg(msg)
char	*msg;
{
	if (msg) {
		move(LINES-1,0);
		PRINTW("** %s", msg);
		clrtoeol();
		dlog_comment("(waitmsg) %s\n", msg);
	}
	move(LINES-1,0);
	refresh();
	beep();
	(void)dlog_char((int *)0,-1);	/* pause beside error message */
	clrtoeol();		/* ...and clear it after pause */
}

/*
 * Fatal-error exit from this process
 */
failed(msg)
char	*msg;
{
	to_exit(msg != 0);
	if (msg)
		FPRINTF(stderr, "-------- \n?? %-79s\n-------- \n", msg);
#ifdef	apollo
	if (msg) {
		(void)kill(getpid(), SIGKILL);
		for (;;);	/* when terminated, will be able to use 'tb' */
	}
#endif	apollo
	dlog_exit(FAIL);
}

/*
 * Given the name of a file in the current list, find its index in 'flist[]'.
 * This is used to reposition after sorting, etc, and uses the feature that
 * strings in 'txtalloc()' are uniquely determined by their address.
 */
findFILE(name)
char	*name;
{
	register int j;
	for (j = 0; j < numfiles; j++)
		if (name == xNAME(j))
			return (j);
	return (0);			/* give up, set to beginning of list */
}

/*
 * Move the cursor up/down the specified number of lines, scrolling
 * to a new screen if necessary.
 */
upLINE(n)
{
	curfile -= n;
	if (curfile < 0)		curfile = 0;
	if (curfile < Ybase) {
		while (curfile < Ybase)	backward(1);
		showFILES();
	} else
		showC();
}

downLINE(n)
{
	curfile += n;
	if (curfile >= numfiles)	curfile = numfiles-1;
	if (curfile > Ylast) {
		while (curfile > Ylast)	forward(1);
		showFILES();
	} else
		showC();
}

showDOWN()
{
	showLINE(curfile);
	dlog_name(cNAME);
	if (curfile < numfiles-1)
		downLINE(1);
	else {
		showC();
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Recompute viewport line-limits for forward/backward scrolling
 */
forward(n)
{
	while (n-- > 0) {
		if (Ylast < (numfiles-1)) {
			Ybase = Ylast + 1;
			viewset();
		} else
			break;
	}
}

backward(n)
{
	while (n-- > 0) {
		if (Ybase > 0) {
			Ybase -= (Ynext - Yhead - 1);
			if (Ybase < 0)	Ybase = 0;
			viewset();
		} else
			break;
	}
}

/*
 * Show, in the viewport-header, where the cursor points (which file, which
 * path) as well as the current setting of the 'C' command.  If any files are
 * tagged, show the heading highlighted.
 */
showWHAT()
{
	static	char	datechr[] = "acm";

	move(Yhead,0);
	if (tag_count)	standout();
	PRINTW("%2d of %2d [%ctime] %", curfile+1, numfiles, datechr[dateopt]);
	PRINTW("%.*s", COLS-((stdscr->_curx)+2), new_wd);
	if (tag_count)	standend();
	clrtoeol();
}

/*
 * Display the given line.  If it is tagged, highlight the name.
 */
showLINE(j)
{
int	col, len;
char	bfr[BUFSIZ];

	if (move2row(j,0)) {
		ded2s(j, bfr, sizeof(bfr));
		if (Xbase < strlen(bfr)) {
			PRINTW("%.*s", COLS-1, &bfr[Xbase]);
			if (xFLAG(j)) {
				col = cmdcol[3] - Xbase;
				len = (COLS-1) - col;
				if (len > 0) {
					(void)move2row(j, col);
					standout();
					PRINTW("%.*s", len, &bfr[cmdcol[3]]);
					standend();
				}
			}
		}
		clrtoeol();
	}
}

/*
 * Display all files in the current viewport
 */
showVIEW()
{
	register int j;

	viewset();		/* set 'Ylast' as function of mark_W */

	for (j = Ybase; j <= Ylast; j++)
		showLINE(j);
	for (j = file2row(Ylast+1); j < Ynext; j++) {
		move(j,0);
		clrtoeol();
	}
}

/*
 * Display all files in the current screen (all viewports), and then show the
 * remaining stuff on the screen (position in each viewport and workspace
 * marker).
 */
showFILES()
{
	register int j, k;
	char	scale[20];

	for (j = 0; j < maxview; j++) {
		showVIEW();
		if (maxview > 1) {
			if (j) showWHAT();
			nextVIEW(TRUE);
		}
	}
	move(mark_W,0);
	for (j = 0; j < COLS - 1; j += 10) {
		k = ((Xbase + j) / 10) + 1;
		FORMAT(scale, "----+---%d", k > 9 ? k : -k);
		PRINTW("%.*s", COLS - j - 1, scale);
	}
	move(mark_W+1,0);
	clrtobot();
	showC();
}

/*
 * Open a new viewport by splitting the current one after the current file.
 */
static
openVIEW()
{
	if (maxview < MAXVIEW) {
		saveVIEW();
		if (maxview) {
			int	adj	= (Yhead = file2row(curfile) + 1)
					- (mark_W - MINLIST);
			if (adj > 0) {
				if (mark_W + adj < (LINES - MINWORK))
					mark_W += adj;
				else
					Yhead -= adj;
			}
		} else
			Yhead = 0;
		curview = maxview++;	/* patch: no insertion? */
		saveVIEW();		/* current viewport */
		markset(mark_W);	/* adjust marker, re-display */
	} else
		dedmsg("too many viewports");
}

/*
 * Store parameters corresponding to the current viewport
 */
static
saveVIEW()
{
	register VIEW *p = &viewlist[curview];
	p->Yhead = Yhead;
	p->Ybase = Ybase;
	p->cname = cNAME;
}

/*
 * Close the current viewport, advance to the next one, if available, and show
 * the new screen contents.
 */
static
quitVIEW()
{
	register int j;

	if (maxview > 1) {
		maxview--;
		for (j = curview; j < maxview; j++)
			viewlist[j] = viewlist[j+1];
		viewlist[0].Yhead = 0;
		curview--;
		nextVIEW(FALSE);	/* load current-viewport */
		showFILES();
	} else
		dedmsg("no more viewports to quit");
}

/*
 * Switch to the next viewport (do not re-display, this is handled elsewhere)
 */
static
nextVIEW(save)
{
	register VIEW *p;

	if (save)
		saveVIEW();
	if (++curview >= maxview)
		curview = 0;
	p = &viewlist[curview];
	Yhead	= p->Yhead;
	Ybase	= p->Ybase;
	curfile	= findFILE(p->cname);
	(void)to_file();
}

#ifdef	Z_RCS_SCCS
showSCCS()
{
register int j;
	if (!Z_opt) {		/* catch up */
		to_work();
		Z_opt = -1;
		for (j = 0; j < numfiles; j++)
			if (!flist[j].z_time) {
				statSCCS(xNAME(j), &flist[j]);
				blip((flist[j].z_time != 0) ? '*' : '.');
			}
	}
}
#endif	Z_RCS_SCCS

/*
 * Set the cursor to the current file, noting this in the viewport header.
 */
showC()
{
	register int	x = cmdcol[2] - Xbase;

	if (x < 0)		x = 0;
	if (x > COLS-1)		x = COLS-1;

	showWHAT();
	markC(FALSE);
	(void)move2row(curfile, x);
	refresh();
}

/*
 * Flag the current-file in the display (i.e., when leaving the current
 * line for the work-area).
 */
markC(on)
{
int	col = cmdcol[2] - Xbase;

	if (col >= 0) {
		(void)move2row(curfile, col);
		addch(on ? '*' : ' ');
	}
}

/*
 * Repaint the screen
 */
retouch(row)
{
int	y,x;
#ifdef	apollo
	if (resizewin()) {
		dlog_comment("resizewin(%d,%d)\n", LINES, COLS);
		markset(mark_W);
		showFILES();
		return;
	}
#endif	apollo
	getyx(stdscr,y,x);
	move(mark_W+1,0);
	clrtobot();
	move(y,x);
	wrepaint(stdscr,row);
}

rescan(fwd, backto)		/* re-scan argument list */
char	*backto;		/* name to reset to, if possible */
{
	auto	 char	tpath[MAXPATHLEN];
	register int	j;

	to_work();
	tag_count = 0;
	if (dedscan(top_argc, top_argv)) {
		curfile = 0;	/* numfiles may be less now */
		dedsort();
		curfile = 0;
		for (j = 0; j < numfiles; j++)
			if (!strcmp(xNAME(j), backto)) {
				curfile = j;
				break;
			}
		(void)to_file();
		showFILES();
		return (TRUE);
	} else if (fwd)
		return (new_args(strcpy(tpath, new_wd), 'F', 1));
	return (FALSE);
}

restat(group)		/* re-'stat()' the current line, and optionally group */
{
	if (group) {
	register int j;
		for (j = 0; j < numfiles; j++) {
			if (j != curfile) {
				if (xFLAG(j)) {
					statLINE(j);
					showLINE(j);
				}
			}
		}
	}
	statLINE(curfile);
	showLINE(curfile);
	showC();
}

restat_l()
{
	restat(TRUE);
}

restat_W()
{
	register int j;
	for (j = Ybase; j <= Ylast; j++) {
		statLINE(j);
		showLINE(j);
	}
	showC();
}

/*
 * Process the given function in a repeat-loop which is interruptable.
 */
resleep(count,func)
int	(*func)();
{
	register int	interrupted = 1,
			last	= count;

	while (count-- > 1) {
		move(LINES-1,0);
		PRINTW("...waiting (%d of %d) ...", last-count, last);
		clrtoeol();
		(*func)();
		sleep(3);
		if (interrupted = dedsigs(TRUE))
			break;
	}
	move(LINES-1,0);
	clrtoeol();		/* clear off the waiting-message */
	if (interrupted)
		(*func)();
	else
		showC();
}

/*
 * Use the 'dedring()' module to switch to a different file-list
 */
static
new_args(path, cmd, count)
char	*path;
{
register int j;
int	ok;

	clear_work();
	if (ok = dedring(path, cmd, count)) {
		(void)to_file();
		for (j = tag_count = 0; j < numfiles; j++)
			if (xFLAG(j))
				tag_count++;
		showFILES();
	}
	(void)chdir(new_wd);
	dlog_comment("chdir %s\n", new_wd);
	return (ok);
}

/*
 * Invoke a new file-list from the directory-tree display, cleaning up if
 * it fails.
 */
static
new_tree(path, cmd, count)
char	*path;
{
	if (new_args(path, cmd, count))
		return (TRUE);
	(void)strcpy(path, new_wd);
	return (FALSE);
}

/*
 * Convert a name to a form which shell commands can use.  For most
 * names, this is simply a copy of the original name.  However, on
 * Apollo, we may have names with '$' and other special characters.
 */
char *
fixname(j)
{
static	char	nbfr[BUFSIZ];
	(void)ded2string(nbfr, sizeof(nbfr), xNAME(j), TRUE);
	return (nbfr);
}

/*
 * Adjust mtime-field so that chmod, chown do not alter it.
 * This fixes Apollo kludges!
 */
fixtime(j)
{
	if (setmtime(xNAME(j), xSTAT(j).st_mtime) < 0)	warn("utime");
}

/*
 * Spawn a subprocess, wait for completion.
 */
static
forkfile(arg0, arg1, normal)
char	*arg0, *arg1;
{
	resetty();
	dlog_comment("execute %s %s\n", arg0, arg1);
	if (execute(arg0, arg1) < 0)
		warn(arg0);
	dlog_elapsed();
	rawterm();
	(void)chdir(new_wd);
	if (normal) {
		retouch(0);
		restat(FALSE);
	}
}

/*
 * Enter an editor (separate process) for the current-file/directory.
 */
static
editfile(readonly, pad)
{
	struct	stat	sb;
	char	*editor = (readonly ? ENV(BROWSE) : ENV(EDITOR));

	dlog_name(cNAME);
	switch (realstat(curfile, &sb)) {
	case 0:
		to_work();
		if (pad) {
			if (padedit(cNAME, readonly, editor) < 0)
				beep();
			restat(FALSE);
		} else
			forkfile(editor, cNAME, TRUE);
		break;
	case 1:
		to_work();
		ft_write();
		dlog_close();
		forkfile(whoami, cNAME, TRUE);
		dlog_reopen();
		ft_read(new_wd, tree_opt);
		break;
	default:
		dedmsg("cannot edit this item");
	}
}
 
/************************************************************************
 *	main program							*
 ************************************************************************/

usage()
{
	auto	char	tmp[BUFSIZ];
	static	char	*tbl[] = {
			"usage: ded [options] [filespecs]",
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
			"  -U       show AEGIS-style names",
#ifdef	Z_RCS_SCCS
			"  -Z       read RCS/SCCS data, show date",
			"  -z       read RCS/SCCS data, don't show date",
#endif	Z_RCS_SCCS
			"",
			"Options controlling initial sort:",
			"  -s KEY   set forward sort",
			"  -r KEY   set reverse sort",
			"",
			"Options controlling environment:",
			"  -c FILE  read DED commands from FILE",
			"  -l FILE  write DED commands to log-FILE",
			"  -d       (debug)",
			"  -t DIR   read \".ftree\"-file from directory DIR",
			(char *)0
			};
	register char	**p;

	setbuf(stderr,tmp);
	for (p = tbl; *p; p++)
		FPRINTF(stderr, "%s\n", *p);
	FPRINTF(stderr, "Sort KEY-options are: \"%s\"\n", sortc);

	dlog_exit(FAIL);
}

main(argc, argv)
char	*argv[];
{
	extern	int	optind;
	extern	char	*optarg;

#include	"version.h"

	register int		j, k;
	auto	struct	stat	sb;
	auto	int		c,
				count,
				lastc	= '?',
				quit	= FALSE;
	auto	char		tpath[BUFSIZ],
				dpath[BUFSIZ];

	(void)sortset('s', 'n');
	(void)sscanf(version, "%*s %s %s", tpath, dpath);
	FPRINTF(stderr, "DED Directory Editor (%s %s)\r\n", tpath, dpath);
	/* show when entering process */
	(void)fflush(stdout);

	while ((c = getopt(argc, argv, "aGIOPSUZc:l:r:s:zdt:")) != EOF)
	switch (c) {
	case 'a':	A_opt = !A_opt;	break;
	case 'G':	G_opt = !G_opt;	break;
	case 'I':	I_opt = !I_opt;	break;
#ifdef	apollo_sr10
	case 'O':	O_opt = !O_opt;	break;
#endif
	case 'P':	P_opt = !P_opt;	break;
	case 'S':	S_opt = !S_opt;	break;
	case 'U':	U_opt = !U_opt;	break;
#ifdef	Z_RCS_SCCS
	case 'Z':	Z_opt = 1;	break;
	case 'z':	Z_opt = -1;	break;
#endif	Z_RCS_SCCS
	case 'c':	dlog_read(optarg);			break;
	case 'l':	log_opt = dlog_open(optarg,argc,argv);	break;
	case 's':
	case 'r':	if (!sortset(c,*optarg))	usage();
			break;
	case 'd':	debug = TRUE;	break;
	case 't':	tree_opt = optarg;	break;
	default:	usage();
			/*NOTREACHED*/
	}

	if (!tree_opt)	tree_opt = gethome();
	ft_read(getwd(old_wd), tree_opt);

	/* find which copy I am executing from, for future use */
	if (which(whoami, sizeof(whoami), argv[0], old_wd) <= 0)
		failed("which-path");

	/* my help-file lives where the binary does */
	FORMAT(howami, "%s.hlp", whoami);

	/* pass options to lower-level processes of ded */
	if (log_opt)
		(void)strcat(strcat(whoami, " -l"), log_opt);

	(void)dedsigs(TRUE);
	if (!initscr())			failed("initscr");
	in_screen = TRUE;
	if (LINES > BUFSIZ || COLS > BUFSIZ) {
	char	bfr[80];
		FORMAT(bfr, "screen too big: %d by %d\n", LINES, COLS);
		failed(bfr);
	}
	rawterm();

	/* Copy 'argv[]' so we can reallocate it; also trim repeated items */
	top_argv = vecalloc((unsigned)(argc - optind + 2));
	top_argc = 0;
	if (optind < argc) {
		for (j = 0; j < argc - optind; j++) {
			char *s = txtalloc(argv[j+optind]);
			for (k = 0; k < j; k++)
				/* look for repeats (same pointer) */
				if (top_argv[k] == s)
					break;
			if (k == j)	/* ... then we never found a repeat */
				top_argv[top_argc++] = s;
		}
	} else
		top_argv[top_argc++] = ".";
	top_argv[top_argc] = 0;		/* always keep a null pointer on end */
	if (!dedscan(top_argc, top_argv))	failed((char *)0);

	mark_W = (LINES/2);
	Xbase = Ybase = 0;
	Xscroll = (1 + (COLS/4)/10) * 10;
	dedsort();
	curfile = 0;
	openVIEW();

	while (!quit) { switch (c = dlog_char(&count,1)) {
			/* scrolling */
	case ARO_UP:
	case '\b':
	case 'k':	upLINE(count);
			break;

	case ARO_DOWN:
	case '\r':
	case '\n':
	case 'j':	downLINE(count);
			break;

	case 'f':	forward(count);
			curfile = Ybase;
			showFILES();
			break;

	case 'b':	backward(count);
			curfile = Ybase;
			showFILES();
			break;

	case ARO_LEFT:	if (Xbase > 0) {
				if ((Xbase -= Xscroll * count) < 0)
					Xbase = 0;
				showFILES();
			} else
				dedmsg("already at left margin");
			break;

	case ARO_RIGHT:	if (Xbase + (Xscroll * count) < 990) {
				Xbase += Xscroll * count;
				showFILES();
			} else
				beep();
			break;

			/* cursor-movement in-screen */
	case 'H':	curfile = Ybase;		showC(); break;
	case 'M':	curfile = (Ybase+Ylast)/2;	showC(); break;
	case 'L':	curfile = Ylast;		showC(); break;
	case '^':	if (Ybase != curfile) {
				Ybase = curfile;
				showFILES();
			}
			break;

			/* display-toggles */
#ifdef	S_IFLNK
	case '@':	AT_opt= !AT_opt;
			count = 0;
			to_work();
			for (j = 0; j < numfiles; j++) {
				if (xLTXT(j)) {
					blip('@');
					statLINE(j);
					count++;
				} else
					blip('.');
			}
			if (count)
				showFILES();
			else
				showC();
			break;
#endif	S_IFLNK

			/* note that '.' and '..' may be the only files! */
	case '&':	A_opt = !A_opt;	/* sorry about inconsistency */
			quit = !rescan(TRUE, strcpy(tpath, cNAME));
			break;
	case 'G':	G_opt = !G_opt; showFILES(); break;
	case 'I':	I_opt = !I_opt; showFILES(); break;
#ifdef	apollo_sr10
	case 'O':	O_opt = !O_opt; showFILES(); break;
#endif
	case 'P':	P_opt = !P_opt; showFILES(); break;
	case 'S':	S_opt = !S_opt; showFILES(); break;
	case 'U':	U_opt = !U_opt; showFILES(); break;

#ifdef	Z_RCS_SCCS
	case 'V':	/* toggle sccs-version display */
			showSCCS();
			V_opt = !V_opt;
			showFILES();
			break;

	case 'Y':	/* show owner of file lock */
			showSCCS();
			Y_opt = !Y_opt;
			showFILES();
			break;

	case 'Z':	/* toggle sccs-date display */
			showSCCS();
			Z_opt = -Z_opt;
			showFILES();
			break;

	case 'z':	/* cancel sccs-display */
			if (Z_opt) {
				Z_opt = 0;
				showFILES();
			}
			break;
#endif	Z_RCS_SCCS

	case 'q':	/* quit this process */
			if (lastc == 't')
				retouch(mark_W+1);
			else
				quit = TRUE;
			break;

			/* move work-area marker */
	case 'A':	count = -count;
	case 'a':
			markset(mark_W + count);
			break;

	case CTL(R):	/* modify read-expression */
			(void)strcpy(tpath, cNAME);
			while (dedread())
				if (rescan(FALSE, tpath))
					break;
			break;

	case 'R':	/* re-stat display-list */
			quit = !rescan(TRUE, strcpy(tpath, cNAME));
			break;

	case 'W':	/* re-stat window */
			resleep(count,restat_W);
			break;

	case 'w':	/* refresh window */
			retouch(0);
			break;

	case 'l':	/* re-stat line */
			resleep(count,restat_l);
			break;

	case ' ':	/* clear workspace */
			retouch(mark_W+1);
			break;

	case 'r':
	case 's':	j = dlog_char((int *)0,0);
			if (tagsort = (j == '+'))
				j = dlog_char((int *)0,0);
			if (sortset(c,j)) {
				dedsort();
				(void)to_file();
				showFILES();
			} else
				dedmsg("unknown sort-key");
			break;

	case 'C':	if (++dateopt > 2)	dateopt = 0;
			showFILES();
			break;

	case '#':	/* tag files with duplicated fields */
			count = tag_count;
			if ((tag_count = deduniq()) || count)
				showFILES();
			else
				showC();
			break;

			/* tag/untag specific files */
	case '+':	while (count-- > 0) {
				if (!cFLAG) {
					cFLAG = TRUE;
					tag_count++;
				}
				if (!showDOWN())
					break;
			}
			break;

	case '-':	while (count-- > 0) {
				if (cFLAG) {
					cFLAG = FALSE;
					tag_count--;
				}
				if (!showDOWN())
					break;
			}
			break;

	case '_':	for (j = 0; j < numfiles; j++)
				xFLAG(j) = FALSE;
			tag_count = 0;
			showFILES();
			break;

			/* edit specific fields */
	case 'p':	editprot();	break;
	case 'u':	edit_uid();	break;
	case 'g':	edit_gid();	break;
	case '=':	editname();	break;
#ifdef	S_IFLNK
	case '<':
	case '>':	editlink(c);	break;
#endif	S_IFLNK

	case '"':	switch (c = dedline(TRUE)) {
			case 'p':	editprot();	break;
			case 'u':	edit_uid();	break;
			case 'g':	edit_gid();	break;
			case '=':	editname();	break;
#ifdef	S_IFLNK
			case '<':
			case '>':	editlink(c);	break;
#endif	S_IFLNK
			default:	dedmsg("no inline command saved");
			}
			(void)dedline(FALSE);
			break;

	case 'c':	/* create an entry */
			dedmake();
			break;

	case CTL(e):	/* pad-edit */
	case CTL(v):	/* pad-view */
	case 'e':
	case 'v':	/* enter new process with current file */
			editfile((c & 037) != CTL(e), iscntrl(c));
			break;

	case 'm':	to_work();
			forkfile(ENV(PAGER), cNAME, TRUE);
#ifndef	apollo
			dedwait();
#endif	apollo
			break;

			/* page thru files in work area */
	case 'h':	dedtype(howami,FALSE);
			c = 't';	/* force work-clear if 'q' */
			break;
	case 't':
	case 'T':	if (realstat(curfile, &sb) >= 0)
				dedtype(cNAME,(c == 'T'));
			c = 't';	/* force work-clear if 'q' */
			break;

	case '%':	/* execute shell command with screen refresh */
	case '!':	/* execute shell command w/o screen refresh */
	case '.':	/* re-execute last shell command */
	case ':':	/* edit last shell command */
			deddoit(c);
			break;

	case '*':	/* display last shell command */
			dedshow("Command=", bfr_sh);
			showC();
			break;

	case '/':
	case '?':
	case 'n':
	case 'N':	/* execute a search command */
			dedfind(c);
			break;

	case 'D':	/* toggle directory/filelist mode */
			(void)strcpy(dpath, strcpy(tpath,new_wd) );
			do {
			    while ((c = ft_view(tpath)) == 'e') {
			    int	y,x;

				getyx(stdscr, y, x);
				if (++y >= LINES)	y = LINES-1;
				move(y, x-x);
				clrtobot();
				move(y, 0);
				refresh();
				ft_write();
				dlog_close();
				forkfile(whoami, tpath, FALSE);
				dlog_reopen();
				wrepaint(stdscr,0);
				ft_read(new_wd, tree_opt);
			    }
			} while (!new_tree(tpath, 'E', 1));
			break;

	case 'E':	/* enter new directory on ring */
			if (realstat(curfile, &sb) == 1) {
				markC(TRUE);
				if (!new_args(
					pathcat(tpath, new_wd, cNAME),
					c, 1)) {
					showC();
				}
#ifdef	S_IFLNK		/* try to edit head-directory of symbolic-link */
			} else if (edithead(tpath, cLTXT)) {
				markC(TRUE);
				if (!new_args(tpath, c, 1))
					showC();
				else
					scroll_to_stat(&sb);
#endif	S_IFLNK
			} else
				dedmsg("not a directory");
			break;

	case 'F':	/* move forward in directory-ring */
	case 'B':	/* move backward in directory-ring */
			(void)new_args(strcpy(tpath, new_wd), c, count);
			showC();
			break;

	case CTL(K):	/* dump the current screen */
			deddump();
			break;

			/* patch: not implemented */
	case 'X':	/* split/join screen (1 or 2 viewports) */
			if (maxview > 1)	quitVIEW();
			else			openVIEW();
			break;

	case '\t':	/* tab to next viewport */
			if (maxview > 1) {
				nextVIEW(TRUE);
				showC();
			} else
				dedmsg("no other viewport");
			break;

	default:	beep();
	}; lastc = c; }

	to_exit(TRUE);
	ft_write();
	dlog_exit(SUCCESS);
	/*NOTREACHED*/
}
