#ifndef	lint
static	char	Id[] = "$Id: dedfind.c,v 11.0 1992/04/06 11:56:51 ste_cm Rel $";
#endif

/*
 * Title:	dedfind.c (find item in ded's file list)
 * Author:	T.E.Dickey
 * Created:	18 Nov 1987
 * Modified:
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		18 Oct 1991, converted to ANSI
 *		11 Jul 1991, interface to 'to_work()'
 *		25 Aug 1989, use 'scroll_to_file()'
 *		14 Mar 1989, interface to 'dlog' module.
 *		06 May 1988, portable regex.
 *		25 Mar 1988, use 'rawgets()' for input.
 *
 * Function:	Search ded's display list (files only) for a specified target
 *		a la 'vi'.
 *
 */
#include	"ded.h"

public	void	dedfind(
	_ARX(RING *,	gbl)
	_AR1(int,	key)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	key)
{
	int	j,k,
		found	= FALSE,
		next	= 0;
	static	char	text[BUFSIZ], *expr;
	static	int	order;		/* saves last legal search order */

	if (key == '/' || key == '?') {

		to_work(gbl,TRUE);
		PRINTW("Target: ");
		getyx(stdscr,j,k);
		clrtobot();
		move(j,k);
		refresh();

		*text = EOS;
		dlog_string(text,sizeof(text),FALSE);
		if (key == '/')	order = 1;
		if (key == '?') order = -1;
		next = order;
	} else if (order) {
		if (key == 'n')	next = order;
		if (key == 'N')	next = -order;
	}

	OLD_REGEX(expr);
	if (NEW_REGEX(expr,text)) {
		for (j = gbl->curfile + next; ; j += next) {
			if (j < 0) {
				j = gbl->numfiles;
			} else if (j >= gbl->numfiles) {
				j = -1;
			} else if (found = GOT_REGEX(expr,gNAME(j))) {
				break;
			} else if (gLTXT(j) != 0) {
				if (found = GOT_REGEX(expr,gLTXT(j)))
					break;
			}
			if (j == gbl->curfile)	break;
		}
		if (found) {
			markC(gbl,FALSE);
			scroll_to_file(gbl, j);
			dlog_name(gNAME(j));
		} else {
		char	msg[BUFSIZ];
			FORMAT(msg, "\"%s\" not found", text);
			dedmsg(gbl, msg);
			return;
		}
	} else {
		order = 0;
		BAD_REGEX(expr);
		showC(gbl);
	}
}
