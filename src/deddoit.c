#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)deddoit.c	1.7 88/04/27 10:23:03";
#endif	NO_SCCS_ID

/*
 * Title:	deddoit.c (do it for ded!)
 * Author:	T.E.Dickey
 * Created:	17 Nov 1987
 * Modified:
 *		27 Apr 1988, modified 'rawgets()' to echo the non-tag string.
 *		25 Mar 1988, use 'rawgets()' for input, helps to implement ':'.
 *			     Recognize '\' as escape character for '#', '%'
 *			     insertion.  Added buffer-overflow check.
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
			if (strlen(subs) + strlen(buffer) < sizeof(subs)-1)
				(void)strcat(subs, buffer);
			else
				return(sizeof(subs));
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

	if (key == '!')
		clr_sh = FALSE;
	else if (key == '%')
		clr_sh = TRUE;

	if ((key != '.') || (bfr_sh[0] == EOS)) {
		if (key != ':')
			*bfr_sh = EOS;
		refresh();
		rawgets(bfr_sh,sizeof(bfr_sh),TRUE);
	} else
		printw("(ditto)\n");

	subs[k = 0] = EOS;
	for (j = 0; bfr_sh[j]; j++) {
		c = bfr_sh[j];
		if (c == '\\') {
			if (bfr_sh[j+1]) {
				subs[k++] = bfr_sh[++j];
				subs[k] = EOS;
			}
		} else if (c == '#') {
			k = expand('n');
		} else if (c == '%') {
			if (bfr_sh[j+1])
				k = expand(bfr_sh[++j]);
			else
				k = expand('?');
		} else {
			subs[k++] = c;
			subs[k] = EOS;
		}
		if (k >= sizeof(subs)-1) {
			*subs = EOS;
			printw("? command is too long\n");
			beep();
			break;
		}
	}
	dedshow("> ", subs);
	refresh();

	if (*subs) {
		resetty();
		system(subs);
		rawterm();
		if (clr_sh) dedwait();
	}
	showC();
}
