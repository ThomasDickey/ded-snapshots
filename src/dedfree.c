#ifndef	lint
static	char	Id[] = "$Id: dedfree.c,v 9.0 1991/05/15 13:39:19 ste_cm Rel $";
#endif

/*
 * Title:	dedfree.c (free ded's main structure)
 * Author:	T.E.Dickey
 * Created:	02 May 1988
 * $Log: dedfree.c,v $
 * Revision 9.0  1991/05/15 13:39:19  ste_cm
 * BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *
 *		Revision 8.1  91/05/15  13:39:19  dickey
 *		apollo sr10.3 cpp complains about tag on #endif
 *		
 *		Revision 8.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  88/05/10  13:18:03  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.4  88/05/10  13:18:03  dickey
 *		sccs2rcs keywords
 *		
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
			if (fp[j].name)	txtfree(fp[j].name);
			if (fp[j].ltxt)	txtfree(fp[j].ltxt);
		}
		FREE((char *)fp);
	}
	return (0);
}
