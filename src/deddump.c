#ifndef	lint
static	char	Id[] = "$Id: deddump.c,v 9.1 1991/09/09 07:57:05 dickey Exp $";
#endif

/*
 * Title:	deddump.c (dump ded's screen)
 * Author:	T.E.Dickey
 * Created:	07 Jun 1988
 * Modified:
 *		09 Sep 1991, flag if err on 'win2file()'
 *		12 Sep 1988, use 'pathcat()'
 *
 * Function:	Dumps the screen to the file "~/ded.log".
 */

#include	"ded.h"
extern	char	*gethome(),
		*pathcat();

deddump()
{
	char	bfr[BUFSIZ];
	if (win2file(stdscr, pathcat(bfr, gethome(), "ded.log")) < 0)
		warn(bfr);
}
