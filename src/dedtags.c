#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: dedtags.c,v 12.2 1994/07/02 20:09:53 tom Exp $";
#endif

/*
 * Title:	dedtags.c (directory-editor tag-file procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		29 Oct 1993, ifdef-ident
 *
 * Function:	Manages flags and summary-counts for tagged-files.
 */

#include	"ded.h"

/*
 * Initialize counters associated with tags
 */
public	void	init_tags (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	gbl->tag_count = 0;
	gbl->tag_bytes = 0;
	gbl->tag_blocks= 0;
}

public	void	tag_entry(
	_ARX(RING *,	gbl)
	_AR1(int,	inx)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inx)
{
	if (!gFLAG(inx)) {
		gFLAG(inx) = TRUE;
		gbl->tag_count++;
		gbl->tag_bytes += gSTAT(inx).st_size;
		gbl->tag_blocks += gSTAT(inx).st_blocks;
	}
}

public	void	untag_entry(
	_ARX(RING *,	gbl)
	_AR1(int,	inx)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inx)
{
	if (gFLAG(inx)) {
		gFLAG(inx) = FALSE;
		gbl->tag_count--;
		gbl->tag_bytes -= gSTAT(inx).st_size;
		gbl->tag_blocks -= gSTAT(inx).st_blocks;
	}
}

/*
 * Re-count the files which are tagged
 */
public	void	count_tags (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register int j;
	init_tags(gbl);
	for (j = 0; j < gbl->numfiles; j++) {
		if (gFLAG(j)) {
			gFLAG(j) = FALSE;
			tag_entry(gbl,j);
		}
	}
}
