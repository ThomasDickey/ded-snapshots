#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: dedsize.c,v 12.5 1994/06/30 23:59:13 tom Exp $";
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

private	void	handle_resize (_AR0)
{
	dlog_comment("resizewin LINES=%d, COLS=%d\n", LINES, COLS);
	if (!ft_resize()) {
		markset(save_gbl, mark_W);
		/*patch showFILES(save_gbl, FALSE); */
		if (gets_active)
			dlog_resize();
	}
}

public	void	dedsize (
		_AR1(RING *,	gbl))
		_DCL(RING *,	gbl)
{
	static	void	(*dummy)(_AR0);
	on_winch(dummy);
	save_gbl = gbl;
	on_winch(handle_resize);
}
#endif
