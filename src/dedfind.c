#ifndef	lint
static	char	Id[] = "$Id: dedfind.c,v 12.1 1993/09/21 17:32:06 dickey Exp $";
#endif

/*
 * Title:	dedfind.c (find item in ded's file list)
 * Author:	T.E.Dickey
 * Created:	18 Nov 1987
 * Modified:
 *		02 Dec 1992, show message "no other match".
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
	register char	*s;
	int	j,k,
		found,
		next	= 0;
	static	DYN	*text;
	static	HIST	*History;
	static	char	*expr;
	static	int	order;		/* saves last legal search order */

	if (key == '/' || key == '?') {

		to_work(gbl,TRUE);
		PRINTW("Target: ");
		getyx(stdscr,j,k);
		clrtobot();
		move(j,k);
		refresh();

		dyn_init(&text, BUFSIZ);
		if (!(s = dlog_string(&text,(DYN **)0, &History, EOS, 0)))
			return;
		if (key == '/')	order = 1;
		if (key == '?') order = -1;
		next = order;
	} else if ((s = dyn_string(text)) != NULL) {
		if (order) {
			if (key == 'n')	next = order;
			if (key == 'N')	next = -order;
		}
	} else {
		dedmsg(gbl, "No previous regular expression");
		return;
	}

	OLD_REGEX(expr);
	if (NEW_REGEX(expr,s)) {
		for (j = gbl->curfile + next; ; j += next) {
			if (j < 0) {
				j = gbl->numfiles;
			} else if (j >= gbl->numfiles) {
				j = -1;
			} else if ((found = GOT_REGEX(expr,gNAME(j))) != 0) {
				break;
			} else if (gLTXT(j) != 0) {
				if ((found = GOT_REGEX(expr,gLTXT(j))) != 0)
					break;
			} else if (j == gbl->curfile) {
				found = FALSE;
				break;
			}
		}
		if (found) {
			markC(gbl,FALSE);
			if (j == gbl->curfile)
				dedmsg(gbl, "no other matches");
			scroll_to_file(gbl, j);
			dlog_name(gNAME(j));
		} else {
		char	msg[BUFSIZ];
			FORMAT(msg, "\"%s\" not found", s);
			dedmsg(gbl, msg);
			return;
		}
	} else {
		order = 0;
		BAD_REGEX(expr);
		showC(gbl);
	}
}
