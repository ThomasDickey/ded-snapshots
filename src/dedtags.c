/*
 * Title:	dedtags.c (directory-editor tag-file procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		15 Feb 1998, add 'count' param to tag/untag functions so caller
 *			     can repaint at the end, making it faster.
 *		29 Oct 1993, ifdef-ident
 *
 * Function:	Manages flags and summary-counts for tagged-files.
 */

#include	"ded.h"

MODULE_ID("$Id: dedtags.c,v 12.7 1998/02/16 01:32:58 tom Exp $")

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
	_ARX(unsigned,	inx)
	_AR1(unsigned,	count)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	inx)
	_DCL(unsigned,	count)
{
	while (count && (inx < gbl->numfiles)) {
		if (!gFLAG(inx)) {
			gFLAG(inx) = TRUE;
			gbl->tag_count++;
			gbl->tag_bytes += gSTAT(inx).st_size;
			gbl->tag_blocks += ded_blocks(&(gSTAT(inx)));
		}
		count--;
		inx++;
	}
}

public	void	untag_entry(
	_ARX(RING *,	gbl)
	_ARX(unsigned,	inx)
	_AR1(unsigned,	count)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	inx)
	_DCL(unsigned,	count)
{
	while (count && (inx < gbl->numfiles)) {
		if (gFLAG(inx)) {
			gFLAG(inx) = FALSE;
			gbl->tag_count--;
			gbl->tag_bytes -= gSTAT(inx).st_size;
			gbl->tag_blocks -= ded_blocks(&(gSTAT(inx)));
		}
		count--;
		inx++;
	}
}

/*
 * Re-count the files which are tagged
 */
public	void	count_tags (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register unsigned j;

	init_tags(gbl);
	for_each_file(gbl,j) {
		if (gFLAG(j)) {
			gFLAG(j) = FALSE;
			tag_entry(gbl, j, 1);
		}
	}
}
