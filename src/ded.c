#ifndef	lint
static	char	sccs_id[] = "@(#)ded.c	1.55 88/09/02 06:28:26";
#endif	lint

/*
 * Title:	ded.c (directory-editor)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
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
extern	char	*strchr();
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
		howami[BUFSIZ];		/* my help-file */

static	char	sortc[] = ".cdgilnprstuwGTUyvzZ";/* valid sort-keys */
					/* (may correspond with cmds) */

/************************************************************************
 *	local procedures						*
 ************************************************************************/
static
sortset(ord,opt)
{
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
realstat(inx)
{
register j = xSTAT(inx).st_mode;

#ifdef	S_IFLNK
	if (isLINK(j)) {
	struct	stat	sb;
		j = (stat(xNAME(inx), &sb) >= 0) ? sb.st_mode : 0;
	}
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
	}
	move(LINES-1,0);
	refresh();
	beep();
	(void)cmdch((int *)0);	/* pause beside error message */
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
		PRINTF("-------- \n?? %-79s\n-------- \n", msg);
#ifdef	apollo
	if (msg) {
		(void)kill(getpid(), SIGKILL);
		for (;;);	/* when terminated, will be able to use 'tb' */
	}
#endif	apollo
	exit(1);
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
int	k = file2row(j),
	col, len;
char	bfr[BUFSIZ];

	if (j >= Ybase && j <= Ylast) {
		move(k,0);
		ded2s(j, bfr, sizeof(bfr));
		if (Xbase < strlen(bfr)) {
			PRINTW("%.*s", COLS-1, &bfr[Xbase]);
			if (xFLAG(j)) {
				col = cmdcol[3] - Xbase;
				len = (COLS-1) - col;
				if (len > 0) {
					move(k, col);
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
				blip('*');
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
	move(file2row(curfile), x);
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
		move(file2row(curfile), col);
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
		markset(mark_W);
		showFILES();
		return;
	}
#endif	apollo
	getyx(stdscr,y,x);
	move(mark_W+1,0);
	clrtobot();
	move(y,x);
	savewin();
	unsavewin(TRUE,row);
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
	for (j = Ybase; j <= Ylast; j++)
		statLINE(j);
	showFILES();
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
	if (execute(arg0, arg1) < 0)
		warn(arg0);
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
	char	*editor = (readonly ? ENV(BROWSE) : ENV(EDITOR));
	switch (realstat(curfile)) {
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
		forkfile(whoami, cNAME, TRUE);
		ft_read(new_wd);
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
	FPRINTF(stderr, "usage: ded [-IGS] [-[s|r][%s]] [filespecs]\n", sortc);
}

main(argc, argv)
char	*argv[];
{
extern	int	optind;
extern	char	*optarg;

#include	"version.h"

register int j, k;
int	c,
	count,
	lastc	= '?',
	quit	= FALSE;
char	tpath[BUFSIZ],
	dpath[BUFSIZ];

	PRINTF("%s\r\n", version+4);	/* show me when entering process */
	(void)fflush(stdout);
	(void)sortset('s', 'n');
	ft_read(getwd(old_wd));

	/* find which copy I am executing from, for future use */
	if (which(whoami, sizeof(whoami), argv[0], old_wd) <= 0)
		failed("which-path");
	FORMAT(howami, "%s.hlp", whoami);

	while ((c = getopt(argc, argv, "GIPSUZr:s:zd")) != EOF) switch (c) {
	case 'G':	G_opt = !G_opt;	break;
	case 'I':	I_opt = !I_opt;	break;
	case 'P':	P_opt = !P_opt;	break;
	case 'S':	S_opt = !S_opt;	break;
	case 'U':	U_opt = !U_opt;	break;
#ifdef	Z_RCS_SCCS
	case 'Z':	Z_opt = 1;	break;
	case 'z':	Z_opt = -1;	break;
#endif	Z_RCS_SCCS
	case 's':
	case 'r':	if (!sortset(c,*optarg))	usage();
			break;
	case 'd':	debug = TRUE;	break;
	default:	usage();
			exit(1);
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

	while (!quit) { switch (c = cmdch(&count)) {
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
			for (j = 0; j < numfiles; j++) {
				if (xLTXT(j))
					statLINE(j);
					count++;
			}
			if (count)
				showFILES();
			break;
#endif	S_IFLNK
	case 'G':	G_opt = !G_opt; showFILES(); break;
	case 'I':	I_opt = !I_opt; showFILES(); break;
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

	case 'R':	/* re-stat display-list */
			to_work();
			tag_count = 0;
			(void)strcpy(tpath,cNAME);
			if (dedscan(top_argc, top_argv)) {
				curfile = 0;	/* numfiles may be less now */
				dedsort();
				curfile = 0;
				for (j = 0; j < numfiles; j++)
					if (!strcmp(xNAME(j), tpath)) {
						curfile = j;
						break;
					}
				(void)to_file();
				showFILES();
			} else
				quit = !new_args(strcpy(tpath, new_wd), 'F', 1);
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
	case 's':	j = cmdch((int *)0);
			if (tagsort = (j == '+'))
				j = cmdch((int *)0);
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
	case '>':	editlink();	break;
#endif	S_IFLNK

	case '"':	switch (dedline(TRUE)) {
			case 'p':	editprot();	break;
			case 'u':	edit_uid();	break;
			case 'g':	edit_gid();	break;
			case '=':	editname();	break;
#ifdef	S_IFLNK
			case '>':	editlink();	break;
#endif	S_IFLNK
			default:	dedmsg("no inline command saved");
			}
			(void)dedline(FALSE);
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
	case 'T':	if (realstat(curfile) >= 0)
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

				savewin();
				getyx(stdscr, y, x);
				if (++y >= LINES)	y = LINES-1;
				move(y, x-x);
				clrtobot();
				move(y, 0);
				refresh();
				ft_write();
				forkfile(whoami, tpath, FALSE);
				unsavewin(TRUE,0);
				ft_read(new_wd);
			    }
			} while (!new_tree(tpath, 'E', 1));
			break;

	case 'E':	/* enter new directory on ring */
			if (realstat(curfile) == 1) {
				markC(TRUE);
				if (!new_args(strcat(
						strcat(
							strcpy(tpath, new_wd),
							"/"),
						cNAME),
					c, 1)) {
					showC();
				}
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
	exit(0);
	/*NOTREACHED*/
}
