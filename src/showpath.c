/*
 * Title:	showpath.c (show pathname)
 * Author:	T.E.Dickey
 * Created:	01 Feb 1990
 * Modified:
 *		16 Feb 1998, compiler warnings
 *		04 Sep 1995, mods for bsd4.4 curses
 *		17 Jul 1994, if base is -1, highlight the level-marker.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		18 Oct 1991, converted to ANSI
 *		31 May 1991, added 'base' argument to control highlighting of a
 *			     portion of the path.
 *		16 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *
 * Function:	Shows an arbitrarily long directory-path using curses on the
 *		current line (up to the right margin).
 */

#include	"ded.h"

MODULE_ID("$Id: showpath.c,v 12.7 1998/02/16 18:27:34 tom Exp $")

#define	DOTLEN	((int)sizeof(ellipsis)-1)

public	void	showpath(
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
	static	char	ellipsis[] = "...";
	register char	*s	= path;
	auto	int	marker	= (base == -1);
	auto	int	cols;
	auto	int	len	= strlen(s);
	auto	int	left	= 0;
	auto	int	hilite	= FALSE;
	auto	char	*d	= s + len;
	auto	char	*t;
	auto	int	y, x;

	getyx(stdscr, y, x);
	cols = COLS - (x + 2 + margin);

	if (cols <= 0)
		return;		/* give up (cannot print anything) */

	if (marker)		/* highlight the slash before the level */
		base = level;

	if (base == 0) {
		hilite = TRUE;
		standout();
	}

	while (len > (cols - left)) {
		if (--level < 0)
			break;	/* force this to show desired level */
		while (isSlash(*s))
			s++;
		if ((t = strchr(s, PATH_SLASH)) != NULL) {
			if (base-- == 0) {
				hilite = TRUE;
				standout();
			}
			s = t;
			len = d - s;
		} else
			break;	/* will have to truncate on right */
		if (left == 0)
			left = DOTLEN;
	}

	if (s != path) {
		PRINTW("%.*s", cols, ellipsis);
		cols -= DOTLEN;
		if (cols <= 0)
			return;
	}

	len = d - s;
	if ((len > cols) && (cols > DOTLEN)) {
		d -= ((len - cols) + DOTLEN);
		while ((d > s) && !isSlash(d[-1]))
			d--;
	}

	/* if we didn't start highlighting, try now */
	len = d - s;
	if ((base >= 0) && !hilite) {
		register int	j;
		for (j = 0; j < len; j++) {
			if (s[j] == EOS)	/* patch */
				break;
			if (isSlash(s[j])) {
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
	}

	if (len > 0)
		PRINTW("%.*s", len, s);
	if (*d != EOS)
		PRINTW(ellipsis);
	if (hilite) {
		standend();
	}
}
