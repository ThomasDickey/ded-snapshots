#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)dedfind.c	1.5 88/05/11 13:07:15";
#endif	NO_SCCS_ID

/*
 * Title:	dedfind.c (find item in ded's file list)
 * Author:	T.E.Dickey
 * Created:	18 Nov 1987
 * Modified:
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
		printw("Target: ");
		getyx(stdscr,j,k);
		clrtobot();
		move(j,k);
		refresh();

		*text = EOS;
		rawgets(text,sizeof(text),FALSE);
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
			} else if (found = GOT_REGEX(expr,flist[j].name)) {
				break;
			} else if (flist[j].ltxt != 0) {
				if (found = GOT_REGEX(expr,flist[j].ltxt))
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
