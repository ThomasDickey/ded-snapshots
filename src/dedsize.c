/*
 * Title:	dedsize.c (ded resizing)
 * Author:	T.E.Dickey
 * Created:	26 Jun 1994
 * Modified:
 *		28 Apr 2020, let ncurses handle SIGWINCH
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

MODULE_ID("$Id: dedsize.c,v 12.12 2025/01/07 01:19:00 tom Exp $")

#ifdef	SIGWINCH

static int save_rows;
static int save_cols;
static RING *save_gbl;

static void
handle_resize(void)
{
    static int working;		/* latch for repeated interrupts */

    if (save_gbl != NULL) {
	if (!working++) {
	    dlog_comment("handle_resize LINES=%d, COLS=%d\n", LINES, COLS);
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
    save_gbl = gbl;
#ifdef NCURSES_VERSION
    if (save_rows != LINES || save_cols != COLS) {
	handle_resize();
    }
#else
    {
	static void (*dummy) (void);
	on_winch(gbl ? handle_resize : dummy);
    }
#endif
    save_rows = LINES;
    save_cols = COLS;
#else
    (void) gbl;
#endif /* SIGWINCH */
}
