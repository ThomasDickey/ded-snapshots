#ifndef	lint
static	char	Id[] = "$Id: deduniq.c,v 9.1 1991/07/11 12:50:54 dickey Exp $";
#endif

/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * Modified:
 *		11 Jul 1991, interface to 'to_work()'
 *		14 Mar 1989, interface to 'dlog'
 *
 * Function:	Resets the file-tags to show files whose sort-key is not unique.
 */
#include	"ded.h"

deduniq(level)
{
	register int	j, k;
	auto	 int	old, new;

	to_work(TRUE);
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
