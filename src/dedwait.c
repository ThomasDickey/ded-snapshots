#ifndef	lint
static	char	Id[] = "$Id: dedwait.c,v 9.0 1991/05/16 07:45:25 ste_cm Rel $";
#endif

/*
 * Title:	dedwait.c (ded wait-for-RETURN)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * $Log: dedwait.c,v $
 * Revision 9.0  1991/05/16 07:45:25  ste_cm
 * BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *
 *		Revision 8.3  91/05/16  07:45:25  dickey
 *		apollo sr10.3 cpp complains about tag on #endif
 *		
 *		Revision 8.2  91/04/22  08:25:24  dickey
 *		lint
 *		
 *		Revision 8.1  91/04/18  08:09:32  dickey
 *		added flag to allow this to be used for debugging (non-curses)
 *		
 *		Revision 8.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/14  10:58:48  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.6  89/03/14  10:58:48  dickey
 *		sccs2rcs keywords
 *		
 *		14 Mar 1989, interface to 'dlog'
 *
 * Function:	Wait for a carriage-return after completion of a process.
 *
 */

#include	"ded.h"

dedwait(cursed)
{
	register int	c;
	static	 char	*msg = "Hit <RETURN> to continue";

	if (cursed) {	/* assume we are already in raw-mode */
		standout();
		PRINTW("%.*s", COLS-1, msg);
		standend();
		PRINTW(" ");
		clrtoeol();
		refresh();
	} else {
		PRINTF("%s", msg);
		(void)fflush(stdout);
	}

	dlog_flush();
	do	c = dlog_char((int *)0,0);
	while	(c != '\n' && c != '\r');
	dlog_comment("%s\n", msg);
	retouch(0);
}
