/*
 * Title:	dedshow.c (ded show-text)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
 *		25 May 2010, fix clang --analyze warnings.
 *		07 Mar 2004, remove K&R support, indent'd
 *		21 Oct 1995, show escaped control chars in printable form.
 *		29 Oct 1993, ifdef-ident, port to HP/UX.
 *		28 Sep 1993, gcc warnings
 *		28 Feb 1992, corrected LINES-limit.
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

MODULE_ID("$Id: dedshow.c,v 12.10 2010/05/25 00:30:13 tom Exp $")

static int
dedshow_c(int ch)
{
    int max_Y = LINES - 1, max_X = COLS - 1;
    int x, y;

    getyx(stdscr, y, x);
    if (addch((chtype) ch) == ERR)
	return FALSE;
    if (++x > max_X) {
	x = 0;
	if (++y >= max_Y)
	    return FALSE;
	move(y, x);
    }
    return TRUE;
}

void
dedshow2(char *arg)
{
    int y, x, ch;
    int max_Y = LINES - 1, literal = lnext_char(), escaped = 0;
    char buf[4];

    if (arg == 0)
	return;

    getyx(stdscr, y, x);
    if (y >= max_Y)
	return;

    while ((ch = *arg++) != EOS) {

	if (isascii(ch) || escaped) {
	    if (ch == literal || ch == '\\')
		escaped = 2;
	    if (!isprint(ch)) {
		if (escaped) {
		    if (ch == 0177) {
			if (!dedshow_c('^'))
			    return;
			ch = '?';
		    } else if (ch >= 0200) {
			sprintf(buf, "%03o", ch & 0xff);
			dedshow2(buf);
			escaped--;
			continue;
		    } else if (iscntrl(ch)) {
			if (!dedshow_c('^'))
			    return;
			ch |= 0100;
		    }
		} else if (ch == '\t') {
		    ch = ' ';
		} else if (ch == '\n') {
		    getyx(stdscr, y, x);
		    x = 0;
		    if (++y > max_Y)
			return;
		    move(y, x);
		    continue;
		} else {
		    /* ignore other chars */
		    continue;
		}
	    }
	    if (!dedshow_c(ch))
		return;
	} else {
	    if (!dedshow_c('{'))
		return;
	    (void) standout();
	    dedshow2("...");
	    (void) standend();
	    if (!dedshow_c('}'))
		return;
	    while ((*arg != EOS) && !isascii(*arg))
		arg++;
	}
	if (escaped > 0)
	    escaped--;
    }
}

void
dedshow(RING * gbl,
	char *tag,
	char *arg)
{
    int y, x;

    getyx(stdscr, y, x);
    if (y < mark_W) {
	to_work(gbl, TRUE);
	x = 0;
    }
    if (x > 0) {
	move(y + 1, 0);
    }

    dedshow2(tag);
    dedshow2(arg);
    dedshow2("\n");

    clrtoeol();
}
