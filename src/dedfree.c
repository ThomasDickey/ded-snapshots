#ifndef	lint
static	char	Id[] = "$Id: dedfree.c,v 12.0 1991/10/18 08:41:43 ste_cm Rel $";
#endif

/*
 * Title:	dedfree.c (free ded's main structure)
 * Author:	T.E.Dickey
 * Created:	02 May 1988
 * Modified:
 *		18 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *
 * Function:	Free storage used in the 'flist[]' array (or
 *		whatever copy of it that the called passes here).
 */

#include	"ded.h"

FLIST *
dedfree(
_ARX(FLIST *,	fp)
_AR1(unsigned,	num)
	)
_DCL(FLIST *,	fp)
_DCL(unsigned,	num)
{
register int j;

	if (fp != 0) {	/* we are rescanning display-list */
		for (j = 0; j < num; j++) {
			if (fp[j].name)	txtfree(fp[j].name);
			if (fp[j].ltxt)	txtfree(fp[j].ltxt);
		}
		FREE((char *)fp);
	}
	return (0);
}
