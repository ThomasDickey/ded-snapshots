#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)deddoit.c	1.2 87/11/24 13:40:59";
#endif	NO_SCCS_ID

/*
 * Title:	deddoit.c (do it for ded!)
 * Author:	T.E.Dickey
 * Created:	17 Nov 1987
 * Modified:
 *
 * Function:	Execute a shell command
 *
 */
#include	"ded.h"
extern	char	*fixname();

static	char	text[BUFSIZ],
		subs[BUFSIZ];

static
expand(code)
{
register int j;
int	first	= TRUE;
char	buffer[BUFSIZ];

	for (j = 0; j < numfiles; j++) {
		if (flist[j].flag || (j == curfile)) {
			switch(code) {
			case 'n':	strcpy(buffer, fixname(j));	break;
			default:	return(strlen(subs));
			}
			if (!first)
				(void)strcat(subs, " ");
			(void)strcat(subs, buffer);
			/* patch: check for buffer overflow */
			first = FALSE;
		}
	}
	return(strlen(subs));
}

deddoit(key)
{
int	c, j, k;

	to_work();
	printw("Command: ");
	getyx(stdscr,j,k);
	clrtobot();
	move(j,k);
	refresh();

	resetty();
	if ((key != '.') || (text[0] == EOS)) {
		getstr(text);
	} else
		printw("(ditto)\n");

	subs[k = 0] = EOS;
	for (j = 0; text[j]; j++) {
		c = text[j];
		if (c == '#')
			k = expand('n');
		else if (c == '%') {
			if (text[j+1])
				k = expand(text[++j]);
			else
				k = expand('?');
		} else {
			subs[k++] = c;
			subs[k] = EOS;
		}
	}
	printw("> \"%.*s\"\n", COLS-1, subs);
	refresh();

	if (*subs) system(subs);
	rawterm();
	showC();
}
