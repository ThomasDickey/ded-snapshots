/*
 * Title:	dedblip.c
 * Author:	T.E.Dickey
 * Created:	23 Nov 1993
 * Modified:
 *
 * Function:	Encapsulates logic that displays (in the work area) a message
 *		showing the progress of long scanning operations.  This was
 *		done with 'blip()', but that proved awkward on very long scans
 *		(when a whole screen would be filled with blips).  This version
 *		displays a status line, showing the total and subtotals.
 */
#include "ded.h"

MODULE_ID("$Id: dedblip.c,v 12.5 1993/11/23 18:40:30 tom Exp $")
 
#define	L_PAREN '('
#define	R_PAREN ')'

typedef	struct	{
	char	*label;
	char	*plural;
	int	total;
	} BLIP;

static	BLIP	blips[] = {
	{".item",	"s",		0},
	{"@link",	"s",		0},
	{"*current",	"",		0},
	{"#match",	"es",		0},
	{"?unknown",	"",		0},
	{(char *)0,	(char *)0,	0}};


void	set_dedblip (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register BLIP *table;

	to_work(gbl, TRUE);
	for (table = blips; table->label != 0; table++) {
		table->total = 0;
	}
}

void	put_dedblip (
	_AR1(int,	code))
	_DCL(int,	code)
{
	register BLIP *	table;
	register int	n;
	register char *	s;

	for (table = blips, n = 0; table->label != 0; table++, n++) {
		if (code == *(table->label) || (n == 0)) {
			table->total++;
		}
	}

	move(mark_W+1,0);
	for (table = blips, n = 0; table->label != 0; table++) {
		if (table->total != 0) {
			if (n != 0) {
				if (n != 1)
					addch(',');
				addch(' ');
			}
			if (n == 1)
				addch(L_PAREN);

			PRINTW("%d", table->total);
			s = table->label;
			if (*(++s)) {
				PRINTW(" %s", s);
				if (table->total > 1)
					addstr(table->plural);
			}
			n++;
		}
	}
	if (n > 1)
		addch(R_PAREN);
	clrtoeol();
	move(mark_W+1,0);
	refresh();
}
