#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: dedshow.c,v 12.4 1993/12/01 16:26:06 dickey Exp $";
#endif

/*
 * Title:	dedshow.c (ded show-text)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * Modified:
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

static
void
Show _ONE(char *,arg)
{
	register int	y, x, ch;
	auto	int	max_Y	= LINES - 1,
			max_X	= COLS  - 1;

	if (arg == 0)
		return;

	getyx(stdscr, y, x);
	if (y >= max_Y)
		return;

	while ((ch = *arg++) != EOS) {

		if (isascii(ch)) {
			if (ch == '\t')
				ch = ' ';
			if (!isprint(ch)) {
				if (ch == '\n') {
					x = 0;
					if (++y > max_Y)
						return;
					move(y,x);
				}
				continue;
			}
			addch((chtype)ch);
			if (++x > max_X) {
				x = 0;
				if (++y >= max_Y)
					return;
				move(y,x);
			}
		} else {
			addch('{');
			standout();
			Show("...");
			standend();
			addch('}');
			getyx(stdscr, y, x);
			while ((*arg != EOS) && !isascii(*arg))
				arg++;
		}
	}
}

void	dedshow(
	_ARX(RING *,	gbl)
	_ARX(char *,	tag)
	_AR1(char *,	arg)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	tag)
	_DCL(char *,	arg)
{
	register int	y,x;

	getyx(stdscr,y,x);
	if (y < mark_W) {
		to_work(gbl,TRUE);
		x = 0;
	}
	if (x > 0) {
		move(y+1,0);
	}

	Show(tag);
	Show(arg);
	Show("\n");

	clrtoeol();
}
