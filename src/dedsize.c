#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: dedsize.c,v 12.2 1994/06/27 23:32:56 tom Exp $";
#endif

/*
 * Title:	dedsize.c (ded resizing)
 * Author:	T.E.Dickey
 * Created:	26 Jun 1994
 * Modified:
 *
 * Function:	Detect resizing of the window within which DED is running,
 *		and adjust the curses screen (if possible).
 *
 * Notes:	When we're running with a BSD curses, it's possible to
 *		reallocate the stdscr structures to effect a resize.  That's
 *		part of the reason that DED uses no subwindows.
 *
 *		Some Sys5 curses implementations (e.g., on HP/UX) do resizing
 *		of the window structures automatically.
 */

#include "ded.h"

#ifdef	SIGWINCH

static	RING	*save_gbl;
static	int	*save_row;

static
void	handle_resize (_AR0)
{
	dlog_comment("resizewin(%d,%d)\n", LINES, COLS);
	if (save_gbl != 0) {
		markset(save_gbl, mark_W);
		showFILES(save_gbl, FALSE);
	} else {
	}
}

void	dedsize (
	_ARX(RING *,	gbl)
	_AR1(int *,	row)
		)
	_DCL(RING *,	gbl)
	_DCL(int *,	row)
{
	on_winch(0);
	save_gbl = gbl;
	save_row = row;
	on_winch(handle_resize);
}
#endif
