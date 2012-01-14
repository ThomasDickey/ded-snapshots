/*
 * Title:	dedtags.c (directory-editor tag-file procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		15 Feb 1998, add 'count' param to tag/untag functions so caller
 *			     can repaint at the end, making it faster.
 *		29 Oct 1993, ifdef-ident
 *
 * Function:	Manages flags and summary-counts for tagged-files.
 */

#include	"ded.h"

MODULE_ID("$Id: dedtags.c,v 12.9 2012/01/13 18:58:19 tom Exp $")

/*
 * Initialize counters associated with tags
 */
void
init_tags(RING * gbl)
{
    gbl->tag_count = 0;
    gbl->tag_bytes = 0;
    gbl->tag_blocks = 0;
}

void
tag_entry(RING * gbl,
	  unsigned inx,
	  unsigned count)
{
    while (count && (inx < gbl->numfiles)) {
	if (!gFLAG(inx)) {
	    gFLAG(inx) = TRUE;
	    gbl->tag_count++;
	    gbl->tag_bytes += (long) gSTAT(inx).st_size;
	    gbl->tag_blocks += (long) ded_blocks(&(gSTAT(inx)));
	}
	count--;
	inx++;
    }
}

void
untag_entry(RING * gbl,
	    unsigned inx,
	    unsigned count)
{
    while (count && (inx < gbl->numfiles)) {
	if (gFLAG(inx)) {
	    gFLAG(inx) = FALSE;
	    gbl->tag_count--;
	    gbl->tag_bytes -= (long) gSTAT(inx).st_size;
	    gbl->tag_blocks -= (long) ded_blocks(&(gSTAT(inx)));
	}
	count--;
	inx++;
    }
}

/*
 * Re-count the files which are tagged
 */
void
count_tags(RING * gbl)
{
    unsigned j;

    init_tags(gbl);
    for_each_file(gbl, j) {
	if (gFLAG(j)) {
	    gFLAG(j) = FALSE;
	    tag_entry(gbl, j, 1);
	}
    }
}
