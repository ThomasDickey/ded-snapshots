#ifndef	lint
static	char	Id[] = "$Id: showpath.c,v 8.0 1990/02/01 15:22:29 ste_cm Rel $";
#endif	lint

/*
 * Title:	showpath.c (show pathname)
 * Author:	T.E.Dickey
 * Created:	01 Feb 1990
 * $Log: showpath.c,v $
 * Revision 8.0  1990/02/01 15:22:29  ste_cm
 * BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *
 *		Revision 7.0  90/02/01  15:22:29  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/02/01  15:22:29  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.1  90/02/01  15:22:29  dickey
 *		RCS_BASE
 *		
 *
 * Function:	Shows an arbitrarily long directory-path using curses on the
 *		current line (up to the right margin).
 */

#define	STR_PTYPES
#include	"ded.h"

#define	LEFT	4
#define	RIGHT	3

showpath(path, level, margin)
char	*path;
int	level;
int	margin;
{
	register char	*s	= path;
	auto	int	cols	= COLS - ((stdscr->_curx) + 2 + margin);
	auto	int	len	= strlen(s);
	auto	int	left	= 0;
	auto	char	*d	= s + len;
	auto	char	*t;

	if (cols <= 0)
		return;		/* give up (cannot print anything) */

	while (len > (cols - left)) {
		if (--level < 0)
			break;	/* force this to show desired level */
		while (*s == '/')
			s++;
		if (t = strchr(s, '/')) {
			s = t + 1;
			len = d - s;
		} else
			break;	/* will have to truncate on right */
		if (left == 0)
			left = LEFT;
	}

	if (s != path) {
		PRINTW("%.*s", cols, ".../");
		cols -= LEFT;
		if (cols <= 0)
			return;
	}

	len = d - s;
	if ((len > cols) && (cols > RIGHT)) {
		d -= ((len - cols) + RIGHT);
		while ((d > s) && (d[-1] != '/'))
			d--;
	}
	PRINTW("%.*s", d - s, s);
	if (*d != EOS)
		PRINTW("...");
}
