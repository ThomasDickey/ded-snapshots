/*
 * Title:	deduniq.c (mark non-unique files)
 * Author:	T.E.Dickey
 * Created:	18 Jan 1989
 * Modified:
 *		29 May 1998, compile with g++
 *		16 Feb 1998, compiler-warnings
 *		23 Nov 1993, new blip-code.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		18 Oct 1991, converted to ANSI
 *		11 Jul 1991, interface to 'to_work()'
 *		14 Mar 1989, interface to 'dlog'
 *
 * Function:	Resets the file-tags to show files whose sort-key is not unique.
 */
#include	"ded.h"

MODULE_ID("$Id: deduniq.c,v 12.7 1998/05/30 02:09:20 tom Exp $")

public	void	deduniq (
	_ARX(RING *,	gbl)
	_AR1(int,	level)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	level)
{
	register unsigned j, k;
	auto	 int	old, tmp;

	set_dedblip(gbl);
	gbl->tagsort = FALSE;	/* don't confuse 'dedsort_cmp()' */

	for (j = (level > 1), old = FALSE; j < gbl->numfiles; j++) {

		k = (level > 1) ? j-1 : gbl->curfile;

		if ((tmp = (k == j)) != 0) {
			put_dedblip('*');
			dlog_name(gNAME(k));
		} else if ((tmp = (! dedsort_cmp(gbl, gbl->flist+k, gbl->flist+j)) ) != 0) {
			put_dedblip('#');
			gFLAG(k) =
			gFLAG(j) = (level > 0);
			if (!old)
				dlog_name(gNAME(k));
			dlog_name(gNAME(j));
		} else
			put_dedblip('.');
		old = tmp;
	}
}
