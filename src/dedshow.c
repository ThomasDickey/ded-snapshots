#ifndef	lint
static	char	sccs_id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/dedshow.c,v 8.0 1988/09/12 08:19:14 ste_cm Rel $";
#endif	lint

/*
 * Title:	dedshow.c (ded show-text)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1987
 * $Log: dedshow.c,v $
 * Revision 8.0  1988/09/12 08:19:14  ste_cm
 * BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *
 *		Revision 7.0  88/09/12  08:19:14  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  88/09/12  08:19:14  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.0  88/09/12  08:19:14  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.0  88/09/12  08:19:14  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  88/09/12  08:19:14  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  88/09/12  08:19:14  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.3  88/09/12  08:19:14  dickey
 *		sccs2rcs keywords
 *		
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

dedshow(tag, arg)
char	*tag, *arg;
{
	auto	int	len	= strlen(arg),
			base	= 0,
			y,x;

	getyx(stdscr,y,x);
	if (y < mark_W) {
		to_work();
		x = 0;
	}
	if (x > 0) {
		move(y+1,0);
	}

	PRINTW("%s", tag);
	while (arg[base] != EOS) {
		if (len > (x = (COLS-1) - strlen(tag)))
			len = x;
		PRINTW("%.*s\n", len, arg + base);
		if (++y >= LINES)
			break;
		base += len;
		len   = strlen(&arg[base]);
		tag   = "";
	}
	clrtoeol();
	refresh();
}
