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
 *
 *		I ifdef'd the guts of the 'dedsize()' function because if I
 *		simply defined 'dedsize()' to nothing, the CLIX 3.1 compiler
 *		decided that the empty ';' was a syntax error.
 */

#include "ded.h"

MODULE_ID("$Id: dedsize.c,v 12.8 1995/04/02 00:35:30 tom Exp $")

#ifdef	SIGWINCH

static	RING	*save_gbl;

private	void	handle_resize (_AR0)
{
	static	int	working;	/* latch for repeated interrupts */

	if (save_gbl != 0) {
		if (!working++) {
			dlog_comment("resizewin LINES=%d, COLS=%d\n", LINES, COLS);
			if (!ft_resize()) {
				markset(save_gbl, mark_W);
				/*patch showFILES(save_gbl, FALSE); */
				if (gets_active)
					dlog_resize();
			}
		}
		working--;
	}
}
#endif	/* SIGWINCH */

public	void	dedsize (
		_AR1(RING *,	gbl))
		_DCL(RING *,	gbl)
{
#ifdef	SIGWINCH
	static	void	(*dummy)(_AR0);
	save_gbl = gbl;
	on_winch(gbl ? handle_resize : dummy);
#endif	/* SIGWINCH */
}