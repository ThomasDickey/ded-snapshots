#ifndef	lint
static	char	sccs_id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/deduniq.c,v 5.0 1989/03/14 11:20:15 ste_cm Rel $";
#endif	lint

/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * $Log: deduniq.c,v $
 * Revision 5.0  1989/03/14 11:20:15  ste_cm
 * BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
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

deduniq()
{
	register int	j;
	auto	int	count	= 0;

	to_work();
	for (j = 0; j < numfiles; j++)
		if (xFLAG(j))
			xFLAG(j) = FALSE;

	tagsort = FALSE;	/* don't confuse 'dedsort_cmp()' */

	for (j = 1; j < numfiles; j++) {

		auto	FLIST	*p = flist + j;

		if (! dedsort_cmp(p-1,p)) {
			blip('#');
			xFLAG(j-1) =
			xFLAG(j) = TRUE;
			count++;
		} else
			blip('.');
	}
	if (count) {	/* one or more pairs may overlap */
		count = 0;
		for (j = 0; j < numfiles; j++)
			if (xFLAG(j)) {
				count++;
				dlog_name(xNAME(j));
			}
	}
	return (count);
}
