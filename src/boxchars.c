/*
 * Author:	T.E.Dickey
 * Created:	16 Jul 1994
 * Modified:
 *
 * Function:	This module encapsulates our knowledge of curses-supported
 *		alternate characters.
 */
#include	"ded.h"

MODULE_ID("$Id: boxchars.c,v 12.6 1995/12/17 01:31:43 tom Exp $")

public	chtype	bar_space[BAR_WIDTH+1];
public	chtype	bar_hline[BAR_WIDTH+1];
public	chtype	bar_ruler[11];

/*ARGSUSED*/
public	void	boxchars(
		_AR1(int,	flag))
		_DCL(int,	flag)
{
	register int	j;
	chtype	Vline, Hline, ltee, plus;

#if defined(ACS_LTEE) && defined(ACS_HLINE)
	if (flag)
		{
		Vline = ACS_VLINE;
		Hline = ACS_HLINE;
		ltee  = ACS_LTEE;
		plus  = ACS_PLUS;
		}
	else
#endif
		{
		Vline = '|';
		Hline = '-';
		ltee  = '|';
		plus  = '+';
		}

	for (j = 0; j < 10; j++)
		bar_ruler[j] = Hline;
	bar_ruler[4] = plus;

	for (j = 0; j < BAR_WIDTH; j++) {
		bar_space[j] = (j != 0) ? ' '   : Vline;
		bar_hline[j] = (j != 0) ? Hline : ltee;
	}

	bar_ruler[10] =
	bar_space[BAR_WIDTH] =
	bar_hline[BAR_WIDTH] = 0;
}
