#ifndef	lint
static	char	Id[] = "$Id: deduniq.c,v 12.0 1992/04/06 08:45:57 ste_cm Rel $";
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

public	void	deduniq (
	_ARX(RING *,	gbl)
	_AR1(int,	level)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	level)
{
	register int	j, k;
	auto	 int	old, new;

	to_work(gbl,TRUE);
	gbl->tagsort = FALSE;	/* don't confuse 'dedsort_cmp()' */

	for (j = (level > 1), old = FALSE; j < gbl->numfiles; j++) {

		k = (level > 1) ? j-1 : gbl->curfile;

		if (new = (k == j)) {
			blip('*');
			dlog_name(gNAME(k));
		} else if (new = (! dedsort_cmp(gbl, gbl->flist+k, gbl->flist+j)) ) {
			blip('#');
			gFLAG(k) =
			gFLAG(j) = (level > 0);
			if (!old)
				dlog_name(gNAME(k));
			dlog_name(gNAME(j));
		} else
			blip('.');
		old = new;
	}
}
