#ifndef	lint
static	char	sccs_id[] = "@(#)dedwait.c	1.3 88/04/22 06:54:07";
#endif	lint

/*
 * Title:	dedwait.c (ded wait-for-RETURN)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
 *
 * Function:	Wait for a carriage-return after completion of a process.
 *
 */

#include	"ded.h"

dedwait()
{
int	c;

	/* assume we are already in raw-mode */
	standout();
	printw("%.*s", COLS-1, "Hit <RETURN> to continue");
	standend();
	printw(" ");
	clrtoeol();
	refresh();

	do	c = cmdch((int *)0);
	while	(c != '\n' && c != '\r');
	retouch(0);
}
