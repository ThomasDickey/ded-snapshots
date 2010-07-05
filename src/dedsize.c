/*
 * Title:	dedsize.c (ded resizing)
 * Author:	T.E.Dickey
 * Created:	26 Jun 1994
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
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

MODULE_ID("$Id: dedsize.c,v 12.10 2010/07/04 22:05:01 tom Exp $")

#ifdef	SIGWINCH

static RING *save_gbl;

static void
handle_resize(void)
{
    static int working;		/* latch for repeated interrupts */

    if (save_gbl != 0) {
	if (!working++) {
	    dlog_comment("resizewin LINES=%d, COLS=%d\n", LINES, COLS);
	    if (!ft_resize()) {
		markset(save_gbl, (unsigned) mark_W);
		/*patch showFILES(save_gbl, FALSE); */
		if (gets_active)
		    dlog_resize();
	    }
	}
	working--;
    }
}
#endif /* SIGWINCH */

void
dedsize(RING * gbl)
{
#ifdef	SIGWINCH
    static void (*dummy) (void);
    save_gbl = gbl;
    on_winch(gbl ? handle_resize : dummy);
#endif /* SIGWINCH */
}
