/*
 * Title:	restat.c ('stat()' and display procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		15 Feb 1998, compiler warnings
 *		29 Oct 1993, ifdef-ident
 *
 * Function:	reinvokes 'stat()' for the specified files; redisplays.
 */

#include	"ded.h"

MODULE_ID("$Id: restat.c,v 12.9 2010/07/04 22:13:21 tom Exp $")

/*
 * re-'stat()' the current line, and optionally group
 */
void
restat(RING * gbl, int group)
{
    if (group) {
	unsigned j;

	for_each_file(gbl, j) {
	    if (j != gbl->curfile) {
		if (gFLAG(j)) {
		    statLINE(gbl, j);
		    showLINE(gbl, j);
		}
	    }
	}
    }
    statLINE(gbl, gbl->curfile);
    showLINE(gbl, gbl->curfile);
    showC(gbl);
}

void
restat_l(RING * gbl)
{
    restat(gbl, TRUE);
}

void
restat_W(RING * gbl)
{
    int j;
    int Ylast = lastVIEW();

    for (j = (int) baseVIEW(); j <= Ylast; j++) {
	statLINE(gbl, (unsigned) j);
	showLINE(gbl, (unsigned) j);
    }
    showC(gbl);
}
