#ifndef	lint
static	char	sccs_id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/dedfind.c,v 4.0 1989/03/14 11:16:57 ste_cm Rel $";
#endif	lint

/*
 * Title:	dedfind.c (find item in ded's file list)
 * Author:	T.E.Dickey
 * Created:	18 Nov 1987
 * $Log: dedfind.c,v $
 * Revision 4.0  1989/03/14 11:16:57  ste_cm
 * BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *
 *		Revision 3.0  89/03/14  11:16:57  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/14  11:16:57  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.9  89/03/14  11:16:57  dickey
 *		sccs2rcs keywords
 *		
 *		14 Mar 1989, interface to 'dlog' module.
 *		06 May 1988, portable regex.
 *		25 Mar 1988, use 'rawgets()' for input.
 *
 * Function:	Search ded's display list (files only) for a specified target
 *		a la 'vi'.
 *
 */
#include	"ded.h"

dedfind(key)
{
int	j,k,
	found	= FALSE,
	next	= 0;
static	char	text[BUFSIZ], *expr;
static	int	order;		/* saves last legal search order */

	if (key == '/' || key == '?') {

		to_work();
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
		for (j = curfile + next; ; j += next) {
			if (j < 0) {
				j = numfiles;
			} else if (j >= numfiles) {
				j = -1;
			} else if (found = GOT_REGEX(expr,xNAME(j))) {
				break;
			} else if (xLTXT(j) != 0) {
				if (found = GOT_REGEX(expr,xLTXT(j)))
					break;
			}
			if (j == curfile)	break;
		}
		if (found) {
			markC(FALSE);
			curfile = j;
			if (to_file())
				showFILES();
			else
				showC();
			dlog_name(cNAME);
		} else {
		char	msg[BUFSIZ];
			FORMAT(msg, "\"%s\" not found", text);
			dedmsg(msg);
			return;
		}
	} else {
		order = 0;
		BAD_REGEX(expr);
		showC();
	}
}
