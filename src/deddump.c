/*
 * Title:	deddump.c (dump ded's screen)
 * Author:	T.E.Dickey
 * Created:	07 Jun 1988
 * Modified:
 *		29 Oct 1993, ifdef-ident
 *		18 Oct 1991, converted to ANSI
 *		09 Sep 1991, flag if err on 'win2file()'
 *		12 Sep 1988, use 'pathcat()'
 *
 * Function:	Dumps the screen to the file "~/ded.log".
 */

#include	"ded.h"

MODULE_ID("$Id: deddump.c,v 12.3 1994/07/02 20:01:12 tom Exp $")

public	void	deddump (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	char	bfr[BUFSIZ];
	if (win2file(stdscr, pathcat(bfr, gethome(), "ded.log")) < 0)
		warn(gbl, bfr);
}
