/*
 * Title:	dedwait.c (ded wait-for-RETURN)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
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

MODULE_ID("$Id: dedwait.c,v 12.5 1994/07/10 02:16:47 tom Exp $")

/*ARGSUSED*/
public	void	dedwait(
	_ARX(RING *,	gbl)
	_AR1(int,	cursed)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	cursed)
{
	register int	c;
	static	 char	*msg = "Hit <RETURN> to continue";

	PRINTF("\n");
	if (cursed) {	/* assume we are already in raw-mode */
		move(LINES-1,0);
		standout();
		PRINTW("%.*s", COLS-1, msg);
		standend();
		PRINTW(" ");
		clrtoeol();
	} else {
		PRINTF("%s", msg);
		(void)fflush(stdout);
	}

	dlog_flush();
	do {
		c = dlog_char(gbl, (int *)0,0);
	} while	(c != '\n');
	dlog_comment("%s\n", msg);
	retouch(gbl,0);
}
