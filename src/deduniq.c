#ifndef	lint
static	char	Id[] = "$Id: deduniq.c,v 5.2 1990/02/08 13:05:15 dickey Exp $";
#endif	lint

/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * $Log: deduniq.c,v $
 * Revision 5.2  1990/02/08 13:05:15  dickey
 * don't tag current entry unless other entries match!
 *
 *		Revision 5.1  90/02/07  08:29:56  dickey
 *		rewrote, using 'level' argument to provide reset/set/all
 *		modes of operation.
 *		
 *		Revision 5.0  89/03/14  11:20:15  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.0  89/03/14  11:20:15  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/03/14  11:20:15  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/14  11:20:15  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.3  89/03/14  11:20:15  dickey
 *		sccs2rcs keywords
 *		
 *		14 Mar 1989, interface to 'dlog'
 *
 * Function:	Resets the file-tags to show files whose sort-key is not unique.
 */
#include	"ded.h"

deduniq(level)
{
	register int	j, k;

	to_work();
	tagsort = FALSE;	/* don't confuse 'dedsort_cmp()' */

	for (j = (level > 1); j < numfiles; j++) {

		k = (level > 1) ? j-1 : curfile;

		if (k == j)
			blip('*');
		else if (! dedsort_cmp(flist+k, flist+j)) {
			blip('#');
			xFLAG(k) =
			xFLAG(j) = (level > 0);
			if (j > 0 && (level > 1))
				dlog_name(xNAME(k));
			dlog_name(xNAME(j));
		} else
			blip('.');
	}
}
