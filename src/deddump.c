#ifndef	lint
static	char	sccs_id[] = "@(#)deddump.c	1.1 88/06/07 07:19:54";
#endif	lint

/*
 * Title:	deddump.c (dump ded's screen)
 * Author:	T.E.Dickey
 * Created:	07 Jun 1988
 * Modified:
 *
 * Function:	Dumps the screen to the file "~/ded.log".
 *
 */

#include	"ded.h"
extern	char	*gethome(),
		*strcat(),
		*strcpy();

deddump()
{
char	bfr[BUFSIZ];
	win2file(stdscr,
		strcat(strcat(strcpy(bfr, gethome()), "/"), "ded.log"));
}
