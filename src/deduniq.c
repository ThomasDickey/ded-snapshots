#ifndef	lint
static	char	Id[] = "$Id: deduniq.c,v 10.1 1992/04/01 14:29:24 dickey Exp $";
#endif

/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * Modified:
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		18 Oct 1991, converted to ANSI
 *		11 Jul 1991, interface to 'to_work()'
 *		14 Mar 1989, interface to 'dlog'
 *
 * Function:	Resets the file-tags to show files whose sort-key is not unique.
 */
#include	"ded.h"

deduniq _ONE(int,level)
{
	register int	j, k;
	auto	 int	old, new;

	to_work(TRUE);
	FOO->tagsort = FALSE;	/* don't confuse 'dedsort_cmp()' */

	for (j = (level > 1), old = FALSE; j < FOO->numfiles; j++) {

		k = (level > 1) ? j-1 : FOO->curfile;

		if (new = (k == j)) {
			blip('*');
			dlog_name(xNAME(k));
		} else if (new = (! dedsort_cmp(FOO->flist+k, FOO->flist+j)) ) {
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
