#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)deddoit.c	1.3 87/12/01 11:08:30";
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

static	char	subs[BUFSIZ];

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
	if (key == '!')
		clr_sh = FALSE;
	else if (key == '%')
		clr_sh = TRUE;

	if ((key != '.') || (bfr_sh[0] == EOS)) {
		getstr(bfr_sh);
	} else
		printw("(ditto)\n");

	subs[k = 0] = EOS;
	for (j = 0; bfr_sh[j]; j++) {
		c = bfr_sh[j];
		if (c == '#')
			k = expand('n');
		else if (c == '%') {
			if (bfr_sh[j+1])
				k = expand(bfr_sh[++j]);
			else
				k = expand('?');
		} else {
			subs[k++] = c;
			subs[k] = EOS;
		}
	}
	dedshow("> ", subs);
	refresh();

	if (*subs) system(subs);
	rawterm();
	if (clr_sh) dedwait();
	showC();
}
