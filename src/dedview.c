#ifndef	lint
static	char	Id[] = "$Id: dedview.c,v 10.6 1992/04/06 16:38:50 dickey Exp $";
#endif

/*
 * Title:	dedview.c (viewport procedures)
 * Author:	T.E.Dickey
 * Created:	03 Apr 1992, from 'ded.c'
 * Modified:
 *
 * Function:	manages a viewport for ded (see: 'X' command)
 */

#include	"ded.h"

#define	MAXVIEW	2		/* number of viewports */
#define	MINLIST	2		/* minimum length of file-list + header */
#define	MINWORK	3		/* minimum size of work-area */

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

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/************************************************************************
 *	public	utility procedures					*
 ************************************************************************/

/*
 * Set 'Ylast' as a function of the current viewport and our position in it.
 * Also, set 'Ynext' to the row number of the first line after the viewport.
 */
public	void	viewset _ONE(RING *,gbl)
{
	register int	j	= curview + 1;

	Ynext	= (j >= maxview) ? mark_W : viewlist[j].Yhead;
	Ylast	= Ynext + Ybase - (Yhead + MINLIST);
	if (Ylast >= gbl->numfiles)	Ylast = gbl->numfiles - 1;
}

/*
 * Translate an index into the file-list to a row-number in the screen for the
 * current viewport.
 */
public	int	file2row _ONE(int,n)
{
	return ((n - Ybase) + Yhead + 1);
}

/*
 * Move to the indicated row; return FALSE if it does not correspond to a
 * currently-displayed line.
 */
public	int	move2row(
	_ARX(int,	n)
	_AR1(int,	col)
		)
	_DCL(int,	n)
	_DCL(int,	col)
{
	if (n >= Ybase && n <= Ylast) {
		move(file2row(n), col);
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Clear the work-area, and move the cursor there.
 */
public	void	clear_work(_AR0)
{
	move(mark_W + 1, 0);
	clrtobot();
	move(mark_W + 1, 0);
	refresh();
}

/*
 * Clear the work area, move the cursor there after setting marker
 */
public	void	to_work(
	_ARX(RING *,	gbl)
	_AR1(int,	clear_it)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	clear_it)
{
	markC(gbl,TRUE);
	if (clear_it)
		clear_work();
}

/*
 * Scroll, as needed, to put current-file in the window
 */
public	int	to_file _ONE(RING *,gbl)
{
	register int	code;
	viewset(gbl);		/* ensure that Ylast is up to date */
	code	= ((gbl->curfile < Ybase)
		|| (gbl->curfile > Ylast));
	while (gbl->curfile > Ylast)	forward(gbl, 1);
	while (gbl->curfile < Ybase)	backward(gbl, 1);
	return(code);
}

public	void	scroll_to_file(
	_ARX(RING *,	gbl)
	_AR1(int,	inx)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inx)
{
	if (gbl->curfile != inx) {
		gbl->curfile = inx;
		if (to_file(gbl))
			showFILES(gbl,FALSE,TRUE);
		else
			showC(gbl);
	}
}

/*
 * Move the workspace marker.  If we are in split-screen mode, also adjust the
 * length of the current viewport.  Finally, re-display the screen.
 */
public	void	markset(
	_ARX(RING *,	gbl)
	_ARX(int,	num)
	_AR1(int,	reset_work)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	num)
	_DCL(int,	reset_work)
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

	(void)to_file(gbl);
	showFILES(gbl,FALSE,reset_work);
}

/*
 * Move the cursor up/down the specified number of lines, scrolling
 * to a new screen if necessary.
 */
public	void	upLINE(
	_ARX(RING *,	gbl)
	_AR1(int,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	n)
{
	gbl->curfile -= n;
	if (gbl->curfile < 0)		gbl->curfile = 0;
	if (gbl->curfile < Ybase) {
		while (gbl->curfile < Ybase)	backward(gbl, 1);
		showFILES(gbl,FALSE,FALSE);
	} else
		showC(gbl);
}

public	void	downLINE(
	_ARX(RING *,	gbl)
	_AR1(int,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	n)
{
	gbl->curfile += n;
	if (gbl->curfile >= gbl->numfiles)	gbl->curfile = gbl->numfiles-1;
	if (gbl->curfile > Ylast) {
		while (gbl->curfile > Ylast)	forward(gbl, 1);
		showFILES(gbl,FALSE,FALSE);
	} else
		showC(gbl);
}

public	int	showDOWN _ONE(RING *,gbl)
{
	showLINE(gbl, gbl->curfile);
	dlog_name(cNAME);
	if (gbl->curfile < gbl->numfiles - 1)
		downLINE(gbl, 1);
	else {
		showC(gbl);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Recompute viewport line-limits for forward/backward scrolling
 */
public	void	forward(
	_ARX(RING *,	gbl)
	_AR1(int,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	n)
{
	while (n-- > 0) {
		if (Ylast < (gbl->numfiles - 1)) {
			Ybase = Ylast + 1;
			viewset(gbl);
		} else
			break;
	}
}

public	void	backward(
	_ARX(RING *,	gbl)
	_AR1(int,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	n)
{
	while (n-- > 0) {
		if (Ybase > 0) {
			Ybase -= (Ynext - Yhead - 1);
			if (Ybase < 0)	Ybase = 0;
			viewset(gbl);
		} else
			break;
	}
}

/*
 * Show, in the viewport-header, where the cursor points (which file, which
 * path) as well as the current setting of the 'C' command.  If any files are
 * tagged, show the heading highlighted.
 */
public	void	showWHAT _ONE(RING *,gbl)
{
	static	char	datechr[] = "acm";

	move(Yhead,0);
	if (gbl->tag_count)	standout();
	PRINTW("%2d of %2d [%ctime] ",
		gbl->curfile + 1,
		gbl->numfiles,
		datechr[gbl->dateopt]);
	if (gbl->tag_opt)
		PRINTW("(%d files, %d %s) ",
			gbl->tag_count,
			(gbl->tag_opt > 1) ? gbl->tag_bytes : gbl->tag_blocks,
			(gbl->tag_opt > 1) ? "bytes"   : "blocks");
	showpath(gbl->new_wd, 999, -1, 0);
	if (gbl->tag_count)	standend();
	clrtoeol();
}

/*
 * Display the given line.  If it is tagged, highlight the name.
 */
public	void	showLINE(
	_ARX(RING *,	gbl)
	_AR1(int,	j)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	j)
{
	int	col, len;
	char	bfr[BUFSIZ];

	if (move2row(j,0)) {
		ded2s(gbl, j, bfr, sizeof(bfr));
		if (gbl->Xbase < strlen(bfr)) {
			PRINTW("%.*s", COLS-1, &bfr[gbl->Xbase]);
			if (gFLAG(j)) {
				col = gbl->cmdcol[CCOL_NAME] - gbl->Xbase;
				len = (COLS-1) - col;
				if (len > 0) {
					int	adj = gbl->cmdcol[CCOL_NAME];
					if (col < 0) {
						adj -= col;
						col  = 0;
						len  = COLS-1;
					}
					(void)move2row(j, col);
					standout();
					PRINTW("%.*s", len, &bfr[adj]);
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
public	void	showVIEW _ONE(RING *,gbl)
{
	register int j;

	viewset(gbl);		/* set 'Ylast' as function of mark_W */

	for (j = Ybase; j <= Ylast; j++)
		showLINE(gbl, j);
	for (j = file2row(Ylast+1); j < Ynext; j++) {
		move(j,0);
		clrtoeol();
	}
}

/*
 * Display the marker which precedes the workspace
 */
public	void	showMARK _ONE(int,col)
{
	register int j, k;
	char	scale[20];

	move(mark_W,0);
	for (j = 0; j < COLS - 1; j += 10) {
		k = ((col + j) / 10) + 1;
		FORMAT(scale, "----+---%d", k > 9 ? k : -k);
		PRINTW("%.*s", COLS - j - 1, scale);
	}
}

/*
 * Display all files in the current screen (all viewports), and then show the
 * remaining stuff on the screen (position in each viewport and workspace
 * marker).
 */
public	void	showFILES(
	_ARX(RING *,	gbl)
	_ARX(int,	reset_cols)
	_AR1(int,	reset_work)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	reset_cols)
	_DCL(int,	reset_work)
{
	register int j;

	if (reset_cols)
		for (j = 0; j < CCOL_MAX; j++)
			gbl->cmdcol[j] = 0;

	for (j = 0; j < maxview; j++) {
		showVIEW(gbl);
		if (maxview > 1) {
			if (j) showWHAT(gbl);
			nextVIEW(gbl,TRUE);
		}
	}
	showMARK(gbl->Xbase);
	if (reset_work) {
		move(mark_W+1,0);
		clrtobot();
	}
	showC(gbl);
}

/*
 * Open a new viewport by splitting the current one after the current file.
 */
public	void	openVIEW _ONE(RING *,gbl)
{
	if (maxview < MAXVIEW) {
		saveVIEW(gbl);
		if (maxview) {
			int	adj	= (Yhead = file2row(gbl->curfile) + 1)
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
		saveVIEW(gbl);		/* current viewport */
		markset(gbl, mark_W,TRUE);	/* adjust marker, re-display */
	} else
		dedmsg(gbl, "too many viewports");
}

/*
 * Store parameters corresponding to the current viewport
 */
public	void	saveVIEW _ONE(RING *,gbl)
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
public	void	quitVIEW _ONE(RING *,gbl)
{
	register int j;

	if (maxview > 1) {
		maxview--;
		for (j = curview; j < maxview; j++)
			viewlist[j] = viewlist[j+1];
		viewlist[0].Yhead = 0;
		curview--;
		nextVIEW(gbl,FALSE);	/* load current-viewport */
		showFILES(gbl,FALSE,TRUE);
	} else
		dedmsg(gbl, "no more viewports to quit");
}

/*
 * Toggle between split- and full-view
 */
public	void	splitVIEW _ONE(RING *,gbl)
{
	if (maxview > 1)	quitVIEW(gbl);
	else			openVIEW(gbl);
}

/*
 * Switch to the next viewport (do not re-display, this is handled elsewhere)
 */
public	void	nextVIEW(
	_ARX(RING *,	gbl)
	_AR1(int,	save)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	save)
{
	register VIEW *p;

	if (save)
		saveVIEW(gbl);
	if (++curview >= maxview)
		curview = 0;
	p = &viewlist[curview];
	Yhead	= p->Yhead;
	Ybase	= p->Ybase;
	gbl->curfile = findFILE(gbl, p->cname);
	(void)to_file(gbl);
}

/*
 * Set the cursor to the current file, noting this in the viewport header.
 */
public	void	showC _ONE(RING *,gbl)
{
	register int	x = gbl->cmdcol[CCOL_CMD] - gbl->Xbase;

	if (x < 0)		x = 0;
	if (x > COLS-1)		x = COLS-1;

	showWHAT(gbl);
	markC(gbl,FALSE);
	(void)move2row(gbl->curfile, x);
	refresh();
}

/*
 * Move cursor to the "other" view.
 */
public	void	tab2VIEW _ONE(RING *,gbl)
{
	if (maxview > 1) {
		nextVIEW(gbl,TRUE);
		showC(gbl);
	} else
		dedmsg(gbl, "no other viewport");
}

/*
 * Flag the current-file in the display (i.e., when leaving the current
 * line for the work-area).
 */
public	void	markC(
	_ARX(RING *,	gbl)
	_AR1(int,	on)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	on)
{
	int	col = gbl->cmdcol[CCOL_CMD] - gbl->Xbase;

	if (col >= 0) {
		(void)move2row(gbl->curfile, col);
		addch(on ? '*' : ' ');
	}
}

public	void	restat_l _ONE(RING *,gbl)
{
	restat(gbl,TRUE);
}

public	void	restat_W _ONE(RING *,gbl)
{
	register int j;
	for (j = Ybase; j <= Ylast; j++) {
		statLINE(gbl, j);
		showLINE(gbl, j);
	}
	showC(gbl);
}

public	int	baseVIEW _ONE(RING *,gbl)
{
	return Ybase;
}

public	int	lastVIEW _ONE(RING *,gbl)
{
	return Ylast;
}
