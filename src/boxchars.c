/*
 * Author:	T.E.Dickey
 * Created:	16 Jul 1994
 * Modified:
 *
 * Function:	This module encapsulates our knowledge of curse-supported
 *		alternate characters.
 */
#include	"ded.h"

MODULE_ID("$Id: boxchars.c,v 12.5 1994/08/12 21:09:59 tom Exp $")

#undef	vline
#undef	hline

public	chtype	bar_space[BAR_WIDTH+1];
public	chtype	bar_hline[BAR_WIDTH+1];
public	chtype	bar_ruler[11];

/*ARGSUSED*/
public	void	boxchars(
		_AR1(int,	flag))
		_DCL(int,	flag)
{
	register int	j;
	chtype	vline, hline, ltee, plus;

#if defined(ACS_LTEE) && defined(ACS_HLINE)
	if (flag)
		{
		vline = ACS_VLINE;
		hline = ACS_HLINE;
		ltee  = ACS_LTEE;
		plus  = ACS_PLUS;
		}
	else
#endif
		{
		vline = '|';
		hline = '-';
		ltee  = '|';
		plus  = '+';
		}

	for (j = 0; j < 10; j++)
		bar_ruler[j] = hline;
	bar_ruler[4] = plus;

	for (j = 0; j < BAR_WIDTH; j++) {
		bar_space[j] = (j != 0) ? ' '   : vline;
		bar_hline[j] = (j != 0) ? hline : ltee;
	}

	bar_ruler[10] =
	bar_space[BAR_WIDTH] =
	bar_hline[BAR_WIDTH] = 0;
}
