/*
 * Title:	restat.c ('stat()' and display procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		29 Oct 1993, ifdef-ident
 *
 * Function:	reinvokes 'stat()' for the specified files; redisplays.
 */

#include	"ded.h"

MODULE_ID("$Id: restat.c,v 12.3 1994/07/02 20:19:06 tom Exp $")

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
	register int j;
		for (j = 0; j < gbl->numfiles; j++) {
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
	auto	int	Ylast = lastVIEW(gbl);

	for (j = baseVIEW(gbl); j <= Ylast; j++) {
		statLINE(gbl, j);
		showLINE(gbl, j);
	}
	showC(gbl);
}
