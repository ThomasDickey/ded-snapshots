#ifndef	lint
static	char	Id[] = "$Id: showpath.c,v 12.0 1991/10/18 09:58:07 ste_cm Rel $";
#endif

/*
 * Title:	showpath.c (show pathname)
 * Author:	T.E.Dickey
 * Created:	01 Feb 1990
 * Modified:
 *		18 Oct 1991, converted to ANSI
 *		31 May 1991, added 'base' argument to control highlighting of a
 *			     portion of the path.
 *		16 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *
 * Function:	Shows an arbitrarily long directory-path using curses on the
 *		current line (up to the right margin).
 */

#define	STR_PTYPES
#include	"ded.h"

#define	LEFT	4
#define	RIGHT	3

showpath(
_ARX(char *,	path)		/* pathname to display */
_ARX(int,	level)		/* level we must show */
_ARX(int,	base)		/* first-level to highlight */
_AR1(int,	margin)		/* space to allow on right */
	)
_DCL(char *,	path)
_DCL(int,	level)
_DCL(int,	base)
_DCL(int,	margin)
{
	register char	*s	= path;
	auto	int	cols	= COLS - ((stdscr->_curx) + 2 + margin);
	auto	int	len	= strlen(s);
	auto	int	left	= 0;
	auto	int	hilite	= FALSE;
	auto	char	*d	= s + len;
	auto	char	*t;

	if (cols <= 0)
		return;		/* give up (cannot print anything) */

	if (base == 0) {
		hilite = TRUE;
		standout();
	}

	while (len > (cols - left)) {
		if (--level < 0)
			break;	/* force this to show desired level */
		while (*s == '/')
			s++;
		if (t = strchr(s, '/')) {
			if (base-- == 0) {
				hilite = TRUE;
				standout();
			}
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

	/* if we didn't start highlighting, try now */
	len = d - s;
	if (base > 0) {
		register int	j;
		for (j = 0; j < len; j++) {
			if (s[j] == '/')
				if (base-- == 0) {
					hilite = TRUE;
					if (j != 0) {
						PRINTW("%.*s", j, s);
						len -= j;
						s   += j;
					}
					standout();
					break;
				}
		}
	}

	if (len > 0)
		PRINTW("%.*s", len, s);
	if (*d != EOS)
		PRINTW("...");
	if (hilite) standend();
}
