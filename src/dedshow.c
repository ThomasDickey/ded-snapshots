#ifndef	lint
static	char	Id[] = "$Id: dedshow.c,v 10.0 1991/10/18 08:41:27 ste_cm Rel $";
#endif

/*
 * Title:	dedshow.c (ded show-text)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
 *		18 Oct 1991, converted to ANSI
 *		11 Jul 1991, interface to 'to_work'
 *		12 Sep 1988, to handle continuation lines
 *
 * Function:	Display text in the workspace area, either directly as
 *		a result of a command, or in continuation of one which
 *		has already begun there.
 *
 *		Assumes that there is always enough room on the screen to
 *		print 'tag', and at least one character of 'arg'.
 */

#include	"ded.h"

dedshow(
_ARX(char *,	tag)
_AR1(char *,	arg)
	)
_DCL(char *,	tag)
_DCL(char *,	arg)
{
	auto	int	len	= strlen(arg),
			base	= 0,
			y,x;

	getyx(stdscr,y,x);
	if (y < mark_W) {
		to_work(TRUE);
		x = 0;
	}
	if (x > 0) {
		move(y+1,0);
	}

	PRINTW("%s", tag);
	while (arg[base] != EOS) {
		if (len > (x = (COLS-1) - strlen(tag)))
			len = x;
		PRINTW("%.*s\n", len, arg + base);
		if (++y >= LINES)
			break;
		base += len;
		len   = strlen(&arg[base]);
		tag   = "";
	}
	clrtoeol();
	refresh();
}
