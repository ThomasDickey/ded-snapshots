#ifndef	lint
static	char	Id[] = "$Id: deduniq.c,v 9.0 1991/05/16 07:45:16 ste_cm Rel $";
#endif

/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * $Log: deduniq.c,v $
 * Revision 9.0  1991/05/16 07:45:16  ste_cm
 * BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *
 *		Revision 8.1  91/05/16  07:45:16  dickey
 *		apollo sr10.3 cpp complains about tag on #endif
 *		
 *		Revision 8.0  90/03/05  13:18:44  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  90/03/05  13:18:44  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/03/05  13:18:44  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.3  90/03/05  13:18:44  dickey
 *		corrected logging of selected-names
 *		
 *		Revision 5.2  90/02/08  13:08:56  dickey
 *		don't tag current entry unless other entries match!
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
	auto	 int	old, new;

	to_work();
	tagsort = FALSE;	/* don't confuse 'dedsort_cmp()' */

	for (j = (level > 1), old = FALSE; j < numfiles; j++) {

		k = (level > 1) ? j-1 : curfile;

		if (new = (k == j)) {
			blip('*');
			dlog_name(xNAME(k));
		} else if (new = (! dedsort_cmp(flist+k, flist+j)) ) {
			blip('#');
			xFLAG(k) =
			xFLAG(j) = (level > 0);
			if (!old)
				dlog_name(xNAME(k));
			dlog_name(xNAME(j));
		} else
			blip('.');
		old = new;
	}
}
