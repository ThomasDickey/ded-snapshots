#ifndef	lint
static	char	Id[] = "$Id: restat.c,v 12.0 1992/04/07 15:09:14 ste_cm Rel $";
#endif

/*
 * Title:	restat.c ('stat()' and display procedures)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *
 * Function:	reinvokes 'stat()' for the specified files; redisplays.
 */

#include	"ded.h"

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

public	void	restat_l _ONE(RING *,gbl)
{
	restat(gbl,TRUE);
}

public	void	restat_W _ONE(RING *,gbl)
{
	register int j;
	auto	int	Ylast = lastVIEW(gbl);

	for (j = baseVIEW(gbl); j <= Ylast; j++) {
		statLINE(gbl, j);
		showLINE(gbl, j);
	}
	showC(gbl);
}
