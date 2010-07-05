/*
 * Title:	dedblip.c
 * Author:	T.E.Dickey
 * Created:	23 Nov 1993
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		10 Apr 1996, made this work before curses is initialized
 *
 * Function:	Encapsulates logic that displays (in the work area) a message
 *		showing the progress of long scanning operations.  This was
 *		done with 'blip()', but that proved awkward on very long scans
 *		(when a whole screen would be filled with blips).  This version
 *		displays a status line, showing the total and subtotals.
 */
#include "ded.h"

MODULE_ID("$Id: dedblip.c,v 12.8 2010/07/04 19:42:30 tom Exp $")

#define	L_PAREN '('
#define	R_PAREN ')'

typedef struct {
    const char *label;
    const char *plural;
    int total;
} BLIP;

static BLIP blips[] =
{
    {".item", "s", 0},
    {"@link", "s", 0},
    {"*current", "", 0},
    {"#match", "es", 0},
    {"?unknown", "", 0},
    {(char *) 0, (char *) 0, 0}};

void
set_dedblip(RING * gbl)
{
    BLIP *table;

    to_work(gbl, TRUE);
    for (table = blips; table->label != 0; table++) {
	table->total = 0;
    }
}

static void
PutChar(int c)
{
    if (in_screen)
	addch((chtype) c);
    else
	fputc(c, stderr);
}

static void
PutText(const char *s)
{
    while (*s != EOS)
	PutChar(*s++);
}

void
put_dedblip(int code)
{
    BLIP *table;
    int n;
    const char *s;
    char temp[20];

    for (table = blips, n = 0; table->label != 0; table++, n++) {
	if (code == *(table->label) || (n == 0)) {
	    table->total++;
	}
    }

    if (in_screen)
	move(mark_W + 1, 0);
    for (table = blips, n = 0; table->label != 0; table++) {
	if (table->total != 0) {
	    if (n != 0) {
		if (n != 1)
		    PutChar(',');
		PutChar(' ');
	    }
	    if (n == 1)
		PutChar(L_PAREN);

	    FORMAT(temp, "%d", table->total);
	    PutText(temp);
	    s = table->label;
	    if (*(++s)) {
		PutChar(' ');
		PutText(s);
		if (table->total > 1)
		    PutText(table->plural);
	    }
	    n++;
	}
    }
    if (n > 1)
	PutChar(R_PAREN);

    if (in_screen) {
	clrtoeol();
	move(mark_W + 1, 0);
	refresh();
    } else {
	fputc('\r', stderr);
	fflush(stderr);
    }
}
