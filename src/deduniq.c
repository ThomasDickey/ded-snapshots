#ifndef	lint
static	char	sccs_id[] = "@(#)deduniq.c	1.1 89/01/18 10:49:05";
#endif	lint

/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * Modified:
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
			if (xFLAG(j))
				count++;
	}
	return (count);
}
