#ifndef	lint
static	char	sccs_id[] = "@(#)dedfree.c	1.2 88/05/09 10:47:17";
#endif	lint

/*
 * Title:	dedfree.c (free ded's main structure)
 * Author:	T.E.Dickey
 * Created:	02 May 1988
 * Modified:
 *
 * Function:	Free storage used in the 'flist[]' array (or
 *		whatever copy of it that the called passes here).
 */

#include	"ded.h"

FLIST *
dedfree(fp, num)
FLIST	*fp;
unsigned num;
{
register int j;

	if (fp != 0) {	/* we are rescanning display-list */
		for (j = 0; j < num; j++) {
			if (fp[j].name)	free(fp[j].name);
			if (fp[j].ltxt)	free(fp[j].ltxt);
		}
		free((char *)fp);
	}
	return (0);
}
