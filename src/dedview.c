/*
 * Title:	dedview.c (viewport procedures)
 * Author:	T.E.Dickey
 * Created:	03 Apr 1992, from 'ded.c'
 * Modified:
 *		21 Jul 1998, change filelist header layout to allow for
 *			     hostname prefix to pathname.
 *		15 Feb 1998, compiler warnings.
 *		16 Feb 1996, use 'freed' parm of redoVIEW (fixes memory bug).
 *		09 Jan 1996, mods for scrolling regions
 *		05 Nov 1995, use 80th column
 *		03 Sep 1995, mods to keep base_file, curfile more stable when
 *			     switching viewports.
 *		19 Oct 1994, mods for color
 *		16 Dec 1993, added 'mrkfile' flag to cleanup 'markC()' logic.
 *		19 Nov 1993, added 'row2VIEW()' for mouse-support.
 *		17 Nov 1993, modify 'top2VIEW()' to make "^" command a toggle.
 *			     Modified up/down line code to simulate scrolling.
 *		29 Oct 1993, ifdef-ident, port to HP/UX.
 *		28 Sep 1993, gcc warnings
 *		02 Dec 1992, fix current-position in 'markC()'
 *
 * Function:	manages a viewport for ded (see: 'X' command)
 */

#include	"ded.h"

MODULE_ID("$Id: dedview.c,v 12.43 2002/12/21 20:29:18 tom Exp $")

#define	MINLIST	2		/* minimum length of file-list + header */
#define	MINWORK	3		/* minimum size of work-area */

#define	ROW2FILE(v,n)		(((n) - (v)->base_row) + (v)->base_file - 1)
#define	FILE2ROW(v,n)		(((n) - (v)->base_file) + (v)->base_row + 1)
#define	FILE_VISIBLE(v,n)	((n) >= (v)->base_file && (n) <= (v)->last_file)

#ifdef	TEST
#define	TRACE(s)	dlog_comment s;
#else
#define	TRACE(s)
#endif

/*
 * Per-viewport main-module state:
 */
typedef	struct	{
		unsigned base_row;	/* beginning of viewport (row)	*/
		unsigned base_file;	/* beginning of viewport (file)	*/
		unsigned last_row;	/* next-viewport (row)		*/
		unsigned last_file;	/* end of viewport (file)	*/
		unsigned curfile;	/* ...save for 'tab2VIEW()'	*/
		RING	*gbl;		/* ...so dedring can identify	*/
	} VIEW;

static	VIEW	viewlist[PORT_MAX],
		*vue = viewlist;

static	unsigned curview;		/* 0..PORT_MAX			*/
static	unsigned maxview;		/* current number of viewports	*/

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Copy viewport limits from the current viewport, used when splitting a
 * viewport.
 */
private	void	copy_view(
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	int	n;

	TRACE(("...copy_view %d to %d...\n", curview, maxview))
	for (n = maxview; n < PORT_MAX; n++) {
		gbl->base_of[n] = vue->base_file;
		gbl->item_of[n] = gbl->curfile;
		TRACE(("...copy_view[%d] base_of=%d, item_of=%d\n", n, gbl->base_of[n], gbl->item_of[n]))
	}
}

/*
 * Store/restore parameters corresponding to the current viewport
 */
private	void	save_view (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register VIEW *p = &viewlist[curview];
	register RING *q = p->gbl;

	TRACE(("...save_view port=%d\n", curview))
	if (q != 0) {
		TRACE(("...save_view old base_of=%d, item_of=%d\n", vue->base_file, vue->curfile))
		q->base_of[curview] = vue->base_file;
		q->item_of[curview] = vue->gbl->curfile;
	}
	p->gbl       = gbl;
	p->base_file = gbl->base_of[curview];
	p->curfile   = gbl->item_of[curview];
	gbl->curfile = p->curfile;
	TRACE(("...save_view new base_of=%d, item_of=%d\n", vue->base_file, vue->curfile))
}

/*
 * Set 'last_file' as a function of the current viewport and our position in it.
 * Also, set 'last_row' to the row number of the first line after the viewport.
 */
private	void	setup_view (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register unsigned j = curview + 1;

	vue->last_row = (j >= maxview)
		? mark_W
		: (int) viewlist[j].base_row;

	vue->last_file = vue->last_row
		+ vue->base_file
		- (vue->base_row + MINLIST);

	if (vue->last_file >= gbl->numfiles)
		vue->last_file = gbl->numfiles - 1;

	TRACE(("setup_view for port %d cur %d base(%d:%d) last(%d:%d)\n",
		vue-viewlist,
		gbl->curfile,
		vue->base_row, vue->base_file,
		vue->last_row, vue->last_file))
}

/*
 * Initialize pointer 'vue' for the current viewport
 */
private	void	init_view (_AR0)
{
	vue = &viewlist[curview];
	vue->gbl->curfile = vue->curfile;
	(void)to_file(vue->gbl);
}

/*
 * Switch to the next viewport (do not re-display, this is handled elsewhere)
 */
private	void	next_view (_AR0)
{
	if (++curview >= maxview)
		curview = 0;
	init_view();
}

/*
 * Validates (sort of) RING-struct for the current viewport.  If I cannot
 * change directories to that, try to go forward.
 */
private	RING *	ring_view (_AR0)
{
	RING	*gbl = vue->gbl;

	while (chdir(gbl->new_wd) < 0)
		if ((gbl = dedring(gbl, gbl->new_wd, 'F', 1, FALSE, (char *)0)) != NULL)
			save_view(gbl);
		else
			break;

	return gbl;
}

/*
 * Close the current viewport, advance to the next one, if available, and show
 * the new screen contents.
 */
private	RING *	quit_view (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register unsigned j;

	TRACE(("quit_view %d of %d\n", curview, maxview))
	if (maxview > 1) {
		maxview--;
		for (j = curview; j < maxview; j++)
			viewlist[j] = viewlist[j+1];
		viewlist[0].base_row = 0;
		curview--;
		next_view();	/* load current-viewport */
		if (ring_view())
			showFILES(vue->gbl,FALSE);
	} else
		dedmsg(gbl, "no more viewports to quit");
	return vue->gbl;
}

/*
 * Special hack to clear the remainder of the line if we've not already passed
 * it.
 */
private	int	trim_at(
	_AR1(int,	line))
	_DCL(int,	line)
{
	int	y, x;

	getyx(stdscr, y, x);
	if (y == line
#if defined(CURSES_LIKE_BSD44)
	 && (x+1 < wMaxX(stdscr))
#endif
	) {
		clrtoeol();
	}
	return TRUE;
}

/*
 * Display the given line.  If it is tagged, highlight the name.
 */
private	void	show_line(
	_ARX(VIEW *,	vp)
	_AR1(unsigned,	j)
		)
	_DCL(VIEW *,	vp)
	_DCL(unsigned,	j)
{
	RING *	gbl = vp->gbl;
	char	bfr[BUFSIZ];

	if (FILE_VISIBLE(vp,j)) {
		int	line = FILE2ROW(vp,j);
		int	trimmed = FALSE;

		move(line,0);
		ded2s(gbl, j, bfr, sizeof(bfr));
		if (gbl->Xbase < (int)strlen(bfr)) {
			int	adj = gbl->cmdcol[CCOL_NAME];
			int	col = adj - gbl->Xbase;
			int	len = COLS - col;
#if defined(HAVE_HAS_COLORS)
			int	end = gbl->cmdcol[CCOL_NAME] + gENTRY(j).z_namlen;
#endif

			if (col < 0) {
				adj -= col;
				col  = 0;
				len  = COLS;
			}
#if defined(HAVE_HAS_COLORS)
			if (end > COLS + gbl->Xbase)
				end = COLS + gbl->Xbase;
#endif

			PRINTW("%.*s", COLS, &bfr[gbl->Xbase]);
			if (len > 0) { /* filename is visible */
				if (gFLAG(j)) {
					trimmed = trim_at(line);
					move(line, col);
					standout();
					PRINTW("%.*s", len, &bfr[adj]);
					standend();
				}
#if defined(HAVE_HAS_COLORS)
				else if (has_colors()
				 && end > adj) {
					trimmed = trim_at(line);
					move(line, col);
					dedcolor(&(gENTRY(j)));
					PRINTW("%.*s", end-adj, &bfr[adj]);
					dedcolor((FLIST *)0);
				}
#endif /* HAVE_HAS_COLORS */
			}
		}
		if (!trimmed)
			(void)trim_at(line);
	}
}

/*
 * Display all files in the current viewport
 */
private	void	show_view (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register unsigned j;

	setup_view(gbl);

	for (j = vue->base_file; j <= vue->last_file; j++)
		show_line(vue, j);
	for (j = file2row(vue->last_file+1); j < vue->last_row; j++) {
		move(j,0);
		clrtoeol();
	}
}

/*
 * Update the number-of-files-tagged display in the given viewport header
 */
private	void	show_what (
	_AR1(VIEW *,	vp))
	_DCL(VIEW *,	vp)
{
	auto	RING *	gbl = vp->gbl;
	auto	char	buffer[80];
	static	char	datechr[] = "acm";
	int	y, x;

	move(vp->base_row,0);
	if (gbl->tag_count)
		standout();

	buffer[0] = EOS;
	if (gbl->tag_opt) {
		sprintf(buffer, "(%d files, %ld %s) ",
			gbl->tag_count,
			(gbl->tag_opt > 1) ? gbl->tag_bytes : gbl->tag_blocks,
			(gbl->tag_opt > 1) ? "bytes"   : "blocks");
	}
	sprintf(buffer + strlen(buffer), "%d of %d [%ctime]",
		gbl->curfile + 1,
		gbl->numfiles,
		datechr[gbl->dateopt]);
	showpath(gbl->new_wd, 999, -1, strlen(buffer) + 3);
	getyx(stdscr, y, x);
	x = COLS - strlen(buffer) - x;
	while (x-- > 0)
		addch(' ');

	addstr(buffer);
	if (gbl->tag_count)
		standend();
}

/*
 * Recompute viewport line-limits for forward/backward scrolling
 */
private	void	forward(
	_ARX(RING *,	gbl)
	_AR1(int,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	n)
{
	TRACE(("forward %d\n", n))
	setup_view(gbl);
	while (n-- > 0) {
		if ((vue->last_file + 1) < gbl->numfiles) {
			vue->base_file = vue->last_file + 1;
			setup_view(gbl);
		} else
			break;
	}
}

private	void	backward(
	_ARX(RING *,	gbl)
	_AR1(int,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	n)
{
	TRACE(("backward %d\n", n))
	setup_view(gbl);
	while (n-- > 0) {
		if (vue->base_file > 0) {
			unsigned delta = (vue->last_row - vue->base_row - 1);
			if (vue->base_file < delta)
				vue->base_file = 0;
			else
				vue->base_file -= delta;
			setup_view(gbl);
		} else
			break;

	}
}

/************************************************************************
 *	public	utility procedures					*
 ************************************************************************/

/*
 * Translate an index into the file-list to a row-number in the screen for the
 * current viewport.
 */
public	int	file2row (
	_AR1(unsigned,	n))
	_DCL(unsigned,	n)
{
	return FILE2ROW(vue,n);
}

/*
 * Move to the indicated row; return FALSE if it does not correspond to a
 * currently-displayed line.
 */
public	int	move2row(
	_ARX(unsigned,	n)
	_AR1(int,	col)
		)
	_DCL(unsigned,	n)
	_DCL(int,	col)
{
	if (FILE_VISIBLE(vue,n)) {
		if (col >= COLS)
			col = COLS-1;
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
	if (!in_dedtype) {
		move(mark_W + 1, 0);
		clrtobot();
	}
	move(mark_W + 1, 0);
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
public	int	to_file (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register int	code;

	TRACE(("to_file %d in %s\n", gbl->curfile, gbl->new_wd))
	setup_view(gbl);

	code = !FILE_VISIBLE(vue, gbl->curfile);

	while (gbl->curfile > vue->last_file)
		forward(gbl, 1);

	while (gbl->curfile < vue->base_file)
		backward(gbl, 1);

	TRACE(("...done to_file:%d\n", code))
	return(code);
}

public	void	scroll_to_file(
	_ARX(RING *,	gbl)
	_AR1(unsigned,	inx)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	inx)
{
	if (gbl->curfile != inx) {
		gbl->curfile = inx;
		if (to_file(gbl))
			showFILES(gbl,FALSE);
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
	_AR1(unsigned,	num)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	num)
{
	unsigned lo = (vue->base_row + MINLIST * (maxview - curview));
	unsigned hi = (LINES - MINWORK);

	if (num < lo)
		num = lo;

	if (curview < (maxview-1)) {	/* not the last viewport */
		unsigned next_W = viewlist[curview+1].base_row;
		if (num > hi) {		/* multiple-adjust */
			mark_W = hi;
			next_W += (num - hi);
			if (curview < (maxview-2))
				hi = viewlist[curview+2].base_row;
			hi -= MINLIST;
			if (next_W > hi)
				next_W = hi;
		} else if (vue->base_row + MINLIST
			<= (hi = next_W + (num - mark_W))) {
			next_W = hi;
			mark_W = num;
		}
		viewlist[curview+1].base_row = next_W;
	} else {			/* in the last viewport */
		if (num > hi)	num = hi;
		mark_W = num;
	}

	(void)to_file(gbl);
	showFILES(gbl, FALSE);
}

/*
 * Move the cursor up/down the specified number of lines, scrolling
 * to a new screen if necessary.
 */
public	void	upLINE(
	_ARX(RING *,	gbl)
	_AR1(unsigned,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	n)
{
	if (gbl->curfile < n)
		gbl->curfile = 0;
	else
		gbl->curfile -= n;

	if (gbl->curfile < vue->base_file) {
		unsigned savebase = vue->base_file;
		while (gbl->curfile < vue->base_file
		 && vue->base_file > 0) {
			vue->base_file -= 1;
			setup_view(gbl);
		}
#if defined(HAVE_WSCRL) && defined(HAVE_WSETSCRREG)
		if (vue->base_file < savebase
		 && setscrreg(vue->base_row+1, vue->last_row - 1) != ERR) {
			scrl(vue->base_file - savebase);
			setscrreg(0, LINES - 1);
		}
#endif
		showFILES(gbl,FALSE);
	} else {
		showC(gbl);
	}
}

public	void	downLINE(
	_ARX(RING *,	gbl)
	_AR1(unsigned,	n)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	n)
{
	gbl->curfile += n;

	if (gbl->curfile >= gbl->numfiles)
		gbl->curfile = gbl->numfiles-1;

	if (gbl->curfile > vue->last_file) {
		unsigned savebase = vue->base_file;
		while (gbl->curfile > vue->last_file
		 && (vue->last_file + 1) < gbl->numfiles) {
			vue->base_file += 1;
			setup_view(gbl);
		}
#if defined(HAVE_WSCRL) && defined(HAVE_WSETSCRREG)
		if (vue->base_file > savebase
		 && setscrreg(vue->base_row+1, vue->last_row - 1) != ERR) {
			scrl(vue->base_file - savebase);
			setscrreg(0, LINES - 1);
		}
#endif
		showFILES(gbl,FALSE);
	} else
		showC(gbl);
}

public	int	showDOWN (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	showLINE(gbl, gbl->curfile);
	dlog_name(cNAME);
	if ((gbl->curfile + 1) < gbl->numfiles) {
		downLINE(gbl, 1);
	} else {
		showC(gbl);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Show, in the viewport-header, where the cursor points (which file, which
 * path) as well as the current setting of the 'C' command.  If any files are
 * tagged, show the heading highlighted.
 */
public	void	showWHAT (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	auto	int	save = vue->curfile = gbl->curfile;
	register unsigned j;

	for (j = 0; j < maxview; j++)
		if (viewlist[j].gbl == gbl) {
			gbl->curfile = viewlist[j].curfile;
			show_what(&viewlist[j]);
		}
	gbl->curfile = save;
}

/*
 * Display the given line.  If it is tagged, highlight the name.
 */
public	void	showLINE(
	_ARX(RING *,	gbl)
	_AR1(unsigned,	inx)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	inx)
{
	auto	int	save = vue->curfile = gbl->curfile;
	register unsigned j;

	for (j = 0; j < maxview; j++)
		if (viewlist[j].gbl == gbl) {
			gbl->curfile = viewlist[j].curfile;
			show_line(&viewlist[j], inx);
		}
	gbl->curfile = save;
}

/*
 * Display the marker which precedes the workspace
 */
public	void	showMARK (
	_AR1(int,	col))
	_DCL(int,	col)
{
	register int marks, units;
	int	y, x;
	char	value[20];

	getyx(stdscr, y, x);
	move(mark_W,0);
	marks = COLS;
	units = (col % 10);
	col  /= 10;
	while (marks > 0) {
		int	ys, xs;
		int	limit = ((marks > 10) ? 10 : marks) - units;
		int	first;

		if (limit <= 0)
			break;
		getyx(stdscr, ys, xs);
		addchnstr(bar_ruler + units, limit);
		FORMAT(value, "%d", ++col);
		first = 10 - units - strlen(value);
		if (first < limit) {
			move(ys, xs + first);
			PRINTW("%.*s", limit - first, value);
		}
		marks -= (10 - units);
		units = 0;
	}
	move(y,x);
}

/*
 * Display all files in the current screen (all viewports), and then show the
 * remaining stuff on the screen (position in each viewport and workspace
 * marker).
 */
public	void	showFILES(
	_ARX(RING *,	gbl)
	_AR1(int,	reset_cols)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	reset_cols)
{
	auto	int	current = curview;
	register int j;
	register unsigned k;

	TRACE(("showFILES(%s,%d)\n", gbl->new_wd, reset_cols))
	if (reset_cols)
		for (j = 0; j < CCOL_MAX; j++)
			gbl->cmdcol[j] = 0;

	save_view(gbl);
	for (k = 0; k < maxview; k++) {
		show_view(vue->gbl);
		if (maxview > 1) {
			if (vue->gbl != viewlist[current].gbl)
				show_what(vue);
			next_view();
		}
	}
	showMARK(gbl->Xbase);
	clear_work();
	showC(gbl);
	TRACE(("...done showFILES\n"))
}

/*
 * Open a new viewport by splitting the current one after the current file.
 */
public	void	openVIEW (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	if (maxview < PORT_MAX) {
		int	adj,
			r_head;

		copy_view(gbl);
		save_view(gbl);
		if (maxview) {
			adj	= (r_head = file2row(gbl->curfile) + 1)
				- (mark_W - MINLIST);

			if (adj > 0) {
				if (mark_W + adj < (LINES - MINWORK))
					mark_W += adj;
				else
					r_head -= adj;
			}
		} else
			r_head = 0;

		curview = maxview++;	/* patch: no insertion? */
		vue = &viewlist[curview];
		vue->base_row = r_head;

		save_view(gbl);		/* current viewport */
		markset(gbl, mark_W);	/* adjust marker, re-display */
	} else
		dedmsg(gbl, "too many viewports");
}

/*
 * Record instance in which main program switches to a new RING-struct for the
 * current viewport.
 */
public	void	redoVIEW (
	_ARX(RING *,	gbl)
	_AR1(int,	freed)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	freed)
{
	TRACE(("redoVIEW %s => %s\n", vue->gbl->new_wd, gbl->new_wd))
	if (freed)
		viewlist[curview].gbl = 0;
	save_view(gbl);
}

/*
 * Scroll forward/backward in the current view.
 */
public	void	scrollVIEW(
	_ARX(RING *,	gbl)
	_AR1(int,	count)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	count)
{
	if (count >= 0) {
		if ((vue->last_file + 1) == gbl->numfiles) {
			gbl->curfile = vue->last_file;
		} else {
			forward(gbl, count);
			gbl->curfile = vue->base_file;
		}
	} else {
		if (gbl->curfile > vue->base_file)
			count++;
		backward(gbl, -count);
		gbl->curfile = vue->base_file;
	}

	showFILES(gbl,FALSE);
}

/*
 * Toggle between split- and full-view
 */
public	RING *	splitVIEW (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	if (maxview > 1)	gbl = quit_view(gbl);
	else			openVIEW(gbl);
	return gbl;
}

/*
 * Cleanup data, forcing a single viewport.  This is used when quitting
 * RING-structs from the directory tree, so that we don't have an obsolete RING
 * pointer in one of the viewports.
 */
public	void	quitVIEW(
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	if (maxview > 1) {
		register unsigned j;
		for (j = 0; j < maxview; j++) {
			if (viewlist[j].gbl == gbl) {
				maxview = 1;
				curview = 0;
				save_view(gbl);
				break;
			}
		}
	}
}

/*
 * Adjust the viewport to put the current file at the top, or (if it is already
 * at the top) to the bottom of the viewport.
 */
public	void	top2VIEW (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register unsigned inx = gbl->curfile;

	if (baseVIEW() == inx) {
		unsigned delta = (vue->last_row - vue->base_row - 2);
		if (inx < delta)
			inx = 0;
		else
			inx -= delta;
	}
	if (inx != vue->base_file) {
		vue->base_file = inx;
		showFILES(gbl,FALSE);
	}
}

/*
 * Set the cursor to the current file, noting this in the viewport header.
 */
public	void	showC (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register int	x = gbl->cmdcol[CCOL_CMD] - gbl->Xbase;

	if (x < 0)		x = 0;

	showWHAT(gbl);
	markC(gbl,FALSE);
	(void)move2row(gbl->curfile, x);
}

/*
 * Move cursor to the "other" view.
 */
public	RING *	tab2VIEW (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	save_view(gbl);
	if (maxview > 1) {
		next_view();
		if (ring_view())
			showC(vue->gbl);
		showMARK(vue->gbl->Xbase);
	} else
		dedmsg(gbl, "no other viewport");
	return vue->gbl;
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
		register int y,x;

		(void)move2row(gbl->curfile, col);

		if (!on || (gbl->mrkfile < 0)) {
			gbl->mrkfile = on ? (int)gbl->curfile : -1;

			getyx(stdscr,y,x);
			addch((chtype)(on ? '*' : ' '));
			(void)move(y,x);
		}
	}
}

/*
 * Returns the index in file-list of the first item in the current view.
 */
public	unsigned baseVIEW (_AR0)
{
	return vue->base_file;
}

/*
 * Returns the index in file-list of the last item in the current view.
 */
public	int	lastVIEW (_AR0)
{
	return vue->last_file;
}

/*
 * Finds the view containing a given row, and sets the current file to that
 * position. This is used for mouse-positioning within the file-lists.
 */
#ifndef	NO_XTERM_MOUSE
public	RING *	row2VIEW (
		_ARX(RING *,	gbl)
		_AR1(int,	row)
			)
		_DCL(RING *,	gbl)
		_DCL(int,	row)
{
	register VIEW *vp;
	register unsigned n, nn;

	for (n = 0; n < maxview; n++) {
		vp = viewlist + n;
		nn = ROW2FILE(vp, row);
		if (FILE_VISIBLE(vp, nn)) {
			curview = n;
			vp->curfile = nn;
			init_view();
			gbl = vue->gbl;
			showC(gbl);
			break;
		}
	}
	return gbl;
}
#endif
