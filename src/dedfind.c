/*
 * Title:	dedfind.c (find item in ded's file list)
 * Author:	T.E.Dickey
 * Created:	18 Nov 1987
 * Modified:
 *		16 Feb 1998, make 'curfile' unsigned.
 *		18 Nov 1993, corrected infinite loop when current-file happened
 *			     to be a symbolic link.
 *		29 Oct 1993, ifdef-ident, port to HP/UX.
 *		28 Sep 1993, gcc warnings
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

MODULE_ID("$Id: dedfind.c,v 12.11 1998/02/16 13:26:31 tom Exp $")

public	void	dedfind(
	_ARX(RING *,	gbl)
	_AR1(int,	key)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	key)
{
	static	DYN	*text;
	static	HIST	*History;
	static	REGEX_T	expr;
	static	int	ok_expr;
	static	int	order = 1;	/* saves last legal search order */
	register char	*s;
	unsigned j;
	int	found,
		next	= order;

	if (key == '/' || key == '?') {
		dyn_init(&text, BUFSIZ);
		if (!(s = dlog_string(gbl, "Target: ", -1, &text,(DYN **)0, &History, EOS, 0)))
			return;
		if (key == '/')	order = 1;
		if (key == '?') order = -1;
		next = order;
	} else if ((s = dyn_string(text)) != NULL) {
		if (key == 'n')	next = order;
		if (key == 'N')	next = -order;
	} else {
		dedmsg(gbl, "No previous regular expression");
		return;
	}

	if (*s == EOS) {
		if (!ok_expr) {
			dedmsg(gbl, "No regular expression");
			return;
		}
	} else {
		if (ok_expr)
			OLD_REGEX(expr);
		if ((ok_expr = NEW_REGEX(expr,s)) == 0) {
			BAD_REGEX(expr);
			showC(gbl);
		}
	}
	if (ok_expr) {
		for (j = gbl->curfile, found = FALSE; ; ) {
			if (next > 0) {
				if ((j += next) >= gbl->numfiles)
					j = 0;
			} else {
				if (j == 0)
					j = gbl->numfiles;
				j += next;
			}
			if ((found = GOT_REGEX(expr,gNAME(j))) != 0) {
				break;
			} else if ((gLTXT(j) != 0)
			    && (found = GOT_REGEX(expr,gLTXT(j))) != 0) {
				break;
			} else if (j == gbl->curfile) {
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
	}
}
