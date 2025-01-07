/*
 * Title:	dedshow.c (ded show-text)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
 *		09 Dec 2019, improve string-handling for non-ASCII chars.
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

MODULE_ID("$Id: dedshow.c,v 12.13 2025/01/07 01:19:00 tom Exp $")

static int
dedshow_c(int ch)
{
    int max_Y = LINES - 1;
    int max_X = COLS - 1;
    int x, y;

    getyx(stdscr, y, x);
    if (addch((ch & 0xff)) == ERR)
	return FALSE;
    if (++x > max_X) {
	x = 0;
	if (++y >= max_Y)
	    return FALSE;
	move(y, x);
    }
    return TRUE;
}

static void
show_invalid(int ch)
{
    char buf[10];
    char *s = buf;

    if (ch == 0x7f) {
	strcpy(buf, "^?");
    } else if (ch >= 0x80) {
	sprintf(buf, "%03o", ch & 0xff);
    } else if (iscntrl(ch)) {
	sprintf(buf, "^%c", ch | 0x40);
    } else {
	sprintf(buf, "%c", ch);
    }
    while (*s != EOS) {
	if (!dedshow_c(UCH(*s++)))
	    break;
    }
}

void
dedshow2(const char *arg)
{
    int y, x, ch;
    int max_Y = LINES - 1;
    int literal = lnext_char();
    int escaped = 0;

    if (arg == NULL)
	return;

    getyx(stdscr, y, x);
    if (y >= max_Y) {
	return;
    }

    while ((ch = *arg++) != EOS) {

	if (valid_shell_char(ch) || ch == '\n' || escaped) {
	    if (ch == literal || ch == '\\')
		escaped = 2;
	    if (valid_shell_char(ch) || ch == '\n') {
		if (!dedshow_c(ch))
		    break;
	    } else {
		if (escaped) {
		    show_invalid(ch);
		    escaped--;
		} else if (ch == '\t') {
		    if (!dedshow_c(' '))
			break;
		} else if (ch == '\n') {
		    getyx(stdscr, y, x);
		    x = 0;
		    if (++y > max_Y)
			break;
		    move(y, x);
		    continue;
		} else {
		    show_invalid(ch);
		}
	    }
	} else if (ch == ELIDE_B) {
	    if (!dedshow_c('{'))
		break;
	    (void) standout();
	    dedshow2("...");
	    (void) standend();
	    if (!dedshow_c('}'))
		break;
	    while ((*arg != EOS) && *arg != ELIDE_E)
		arg++;
	    if (*arg == ELIDE_E)
		arg++;
	} else {
	    show_invalid(ch);
	}
	if (escaped > 0)
	    escaped--;
    }
}

void
dedshow(RING * gbl,
	const char *tag,
	const char *arg)
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
