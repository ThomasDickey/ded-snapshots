/*
 * Title:	dedwait.c (ded wait-for-RETURN)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		29 Oct 1993, ifdef-ident
 *		18 Oct 1991, converted to ANSI
 *		16 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		22 Apr 1991, lint
 *		18 Apr 1991, added flag to allow this to be used for debugging
 *			     (non-curses)
 *		14 Mar 1989, interface to 'dlog'
 *
 * Function:	Wait for a carriage-return after completion of a process.
 *
 */

#include	"ded.h"

MODULE_ID("$Id: dedwait.c,v 12.6 2004/03/07 23:25:18 tom Exp $")

/*ARGSUSED*/
void
dedwait(RING * gbl, int cursed)
{
    int c;
    static char *msg = "Hit <RETURN> to continue";

    PRINTF("\n");
    if (cursed) {		/* assume we are already in raw-mode */
	move(LINES - 1, 0);
	standout();
	PRINTW("%.*s", COLS - 1, msg);
	standend();
	PRINTW(" ");
	clrtoeol();
    } else {
	PRINTF("%s", msg);
	(void) fflush(stdout);
    }

    dlog_flush();
    do {
	c = dlog_char(gbl, (int *) 0, 0);
    } while (c != '\n');
    dlog_comment("%s\n", msg);
    retouch(gbl, 0);
}
