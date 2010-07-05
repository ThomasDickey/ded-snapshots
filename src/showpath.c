/*
 * Title:	showpath.c (show pathname)
 * Author:	T.E.Dickey
 * Created:	01 Feb 1990
 * Modified:
 *		25 May 2010, fix clang --analyze warnings.
 *		07 Mar 2004, remove K&R support, indent'd
 *		21 Jul 1998, show hostname prefix for pathname
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

MODULE_ID("$Id: showpath.c,v 12.12 2010/07/04 21:30:39 tom Exp $")

#define	DOTLEN	((int)sizeof(ellipsis)-1)

void
showpath(char *path,		/* pathname to display */
	 int level,		/* level we must show */
	 int base,		/* first-level to highlight */
	 int margin)		/* space to allow on right */
{
    static char the_host[MAXPATHLEN];
    static char ellipsis[] = "...";
    static int len_host;
    char *s = path;
    int marker = (base == -1);
    int cols;
    int len = (int) strlen(s);
    int left = 0;
    int hilite = FALSE;
    char *d = s + len;
    char *t;
    int y, x;

    if (len_host == 0) {
#if defined(HAVE_GETHOSTNAME)
	gethostname(the_host, sizeof(the_host) - 1);
	if (strlen(the_host) != 0)
	    strcat(the_host, ":");
	else
#endif
	    strcpy(the_host, "path: ");
	len_host = (int) strlen(the_host);
    }
    getyx(stdscr, y, x);
    cols = COLS - (x + 2 + margin + len_host);

    if (cols <= 0)
	return;			/* give up (cannot print anything) */

    if (marker)			/* highlight the slash before the level */
	base = level;

    addstr(the_host);

    if (base == 0) {
	hilite = TRUE;
	(void) standout();
    }

    while (len > (cols - left)) {
	if (--level < 0)
	    break;		/* force this to show desired level */
	while (isSlash(*s))
	    s++;
	if ((t = strchr(s, PATH_SLASH)) != NULL) {
	    if (base-- == 0) {
		hilite = TRUE;
		(void) standout();
	    }
	    s = t;
	    len = (int) (d - s);
	} else
	    break;		/* will have to truncate on right */
	if (left == 0)
	    left = DOTLEN;
    }

    if (s != path) {
	PRINTW("%.*s", cols, ellipsis);
	cols -= DOTLEN;
	if (cols <= 0)
	    return;
    }

    len = (int) (d - s);
    if ((len > cols) && (cols > DOTLEN)) {
	d -= ((len - cols) + DOTLEN);
	while ((d > s) && !isSlash(d[-1]))
	    d--;
    }

    /* if we didn't start highlighting, try now */
    len = (int) (d - s);
    if ((base >= 0) && !hilite) {
	int j;
	for (j = 0; j < len; j++) {
	    if (s[j] == EOS)	/* patch */
		break;
	    if (isSlash(s[j])) {
		if (base-- == 0) {
		    hilite = TRUE;
		    if (j != 0) {
			PRINTW("%.*s", j, s);
			len -= j;
			s += j;
		    }
		    (void) standout();
		    break;
		}
	    }
	}
    }

    if (len > 0)
	PRINTW("%.*s", len, s);
    if (*d != EOS)
	PRINTW("%s", ellipsis);
    if (hilite) {
	(void) standend();
    }
}
