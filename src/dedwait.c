#ifndef	lint
static	char	Id[] = "$Id: dedwait.c,v 11.0 1992/04/06 10:12:10 ste_cm Rel $";
#endif

/*
 * Title:	dedwait.c (ded wait-for-RETURN)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
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

public	void	dedwait(
	_ARX(RING *,	gbl)
	_AR1(int,	cursed)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	cursed)
{
	register int	c;
	static	 char	*msg = "Hit <RETURN> to continue";

	if (cursed) {	/* assume we are already in raw-mode */
		standout();
		PRINTW("%.*s", COLS-1, msg);
		standend();
		PRINTW(" ");
		clrtoeol();
		refresh();
	} else {
		PRINTF("%s", msg);
		(void)fflush(stdout);
	}

	dlog_flush();
	do	c = dlog_char((int *)0,0);
	while	(c != '\n' && c != '\r');
	dlog_comment("%s\n", msg);
	retouch(gbl,0);
}
