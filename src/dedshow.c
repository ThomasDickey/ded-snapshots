#ifndef	lint
static	char	sccs_id[] = "@(#)dedshow.c	1.1 87/12/01 10:41:50";
#endif	lint

/*
 * Title:	dedshow.c (ded show-text)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 *
 * Function:	Display text in the workspace area, either directly as
 *		a result of a command, or in continuation of one which
 *		has already begun there.
 *
 * patch:	Should handle continuation lines!
 */

#include	"ded.h"

dedshow(tag, arg)
char	*tag, *arg;
{
char	bfr[BUFSIZ];
int	len	= (COLS-1) - strlen(strcpy(bfr, tag)),
	y,x;

	getyx(stdscr,y,x);
	if (y < mark_W) {
		to_work();
		x = 0;
	}
	if (x > 0) {
		move(y+1,0);
	}

	printw("%s%.*s\n", bfr, len, arg);
	clrtoeol();
	refresh();
}
