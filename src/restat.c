/*
 * Title:	restat.c ('stat()' and display procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		15 Feb 1998, compiler warnings
 *		29 Oct 1993, ifdef-ident
 *
 * Function:	reinvokes 'stat()' for the specified files; redisplays.
 */

#include	"ded.h"

MODULE_ID("$Id: restat.c,v 12.7 1998/02/16 18:22:50 tom Exp $")

/*
 * re-'stat()' the current line, and optionally group
 */
public	void	restat(
	_ARX(RING *,	gbl)
	_AR1(int,	group)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	group)
{
	if (group) {
		register unsigned j;

		for_each_file(gbl,j) {
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

public	void	restat_l (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	restat(gbl,TRUE);
}

public	void	restat_W (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register int j;
	auto	int	Ylast = lastVIEW();

	for (j = baseVIEW(); j <= Ylast; j++) {
		statLINE(gbl, j);
		showLINE(gbl, j);
	}
	showC(gbl);
}
