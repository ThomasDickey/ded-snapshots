#ifndef	lint
static	char	Id[] = "$Id: deddump.c,v 10.0 1991/10/18 09:49:40 ste_cm Rel $";
#endif

/*
 * Title:	deddump.c (dump ded's screen)
 * Author:	T.E.Dickey
 * Created:	07 Jun 1988
 * Modified:
 *		18 Oct 1991, converted to ANSI
 *		09 Sep 1991, flag if err on 'win2file()'
 *		12 Sep 1988, use 'pathcat()'
 *
 * Function:	Dumps the screen to the file "~/ded.log".
 */

#include	"ded.h"

deddump(_AR0)
{
	char	bfr[BUFSIZ];
	if (win2file(stdscr, pathcat(bfr, gethome(), "ded.log")) < 0)
		warn(bfr);
}
