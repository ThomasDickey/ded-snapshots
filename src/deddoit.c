#ifndef	lint
static	char	sccs_id[] = "@(#)deddoit.c	1.14 88/09/12 15:36:13";
#endif	lint

/*
 * Title:	deddoit.c (do it for ded!)
 * Author:	T.E.Dickey
 * Created:	17 Nov 1987
 * Modified:
 *		03 Aug 1988, Use 'dedsigs()' so we can fix signals at one point.
 *		02 Aug 1988, so that if nothing is read from 'rawgets()', we
 *			     don't overwrite the last contents of 'bfr_sh[]'.
 *		17 May 1988, recoded '%'-substitution to work only on the
 *			     current entry, and to do a variety of subs for it.
 *		27 Apr 1988, modified 'rawgets()' to echo the non-tag string.
 *		25 Mar 1988, use 'rawgets()' for input, helps to implement ':'.
 *			     Recognize '\' as escape character for '#', '%'
 *			     insertion.  Added buffer-overflow check.
 *
 * Function:	Execute a shell command
 *
 * patch:	should permit repeat-count to 'r', 'e' commands, as well as F,B.
 */
#include	"ded.h"
extern	char	*fixname();
extern	char	*dedrung();
extern	char	*pathcat();
extern	char	*strchr();
extern	char	*strrchr();

static	char	temp[BUFSIZ];
static	char	subs[BUFSIZ];

/*
 * Return a pointer to a leaf of a given name
 */
static
char *
subleaf(name)
char	*name;
{
char	*leaf	= name;
#ifdef	apollo
	if (*leaf == '/')	leaf++;
#endif	apollo
	if (leaf = strrchr(leaf, '/'))
		leaf++;
	else
		leaf = name;
	return (leaf);
}

/*
 * Return a pointer to the "." extension of a given name.
 */
static
char *
subroot(name)
char	*name;
{
char	*root;
	if (!(root = strrchr(name, '.')))
		root = name + strlen(name);
	return (root);
}

/*
 * Perform '%' expansions for current-entry.  The substitutions are modified
 * from the ":" modifiers defined for "csh".
 */
static
expand(code)
{
	char	name[BUFSIZ],
	*from	= 0;

	if (strchr("NHRET", code))
		abspath(pathcat(name, new_wd, cNAME));
	else
		(void)strcpy(name, cNAME);

	switch(code) {
	case 'F':	from = dedrung(1);
			break;

	case 'B':	from = dedrung(-1);
			break;

	case 'D':	from = old_wd;	/* original working directory */
			break;

	case 'd':	from = new_wd;	/* current working directory */
			break;

	case 'N':
	case 'n':	/* current entry-name */
			from = name;
			break;

	case 'H':
	case 'h':	/* Remove a pathname component, leaving head */
			*subleaf(from = name) = EOS;
			if (*from == EOS)
				(void)strcpy(from, "./");
			break;

	case 'R':
	case 'r':	/* Remove a trailing ".xxx" component, leaving root */
			*subroot(subleaf(from = name)) = EOS;
			break;

	case 'E':
	case 'e':	/* Remove all but trailing ".xxx" component */
			from = subroot(subleaf(name));
			break;

	case 'T':
	case 't':	/* Remove all leading pathname components, leave tail */
			from = subleaf(name);
			break;
	}

	if (from) {
		(void)ded2string(temp, sizeof(temp), from, TRUE);
		if (strlen(subs) + strlen(temp) < sizeof(subs)-1)
			(void)strcat(subs, temp);
		else
			return(sizeof(subs));
	}
	return(strlen(subs));
}

/*
 * Prompt for, substitute and execute a shell command.
 */
deddoit(key)
{
	register int	c, j, k;
	register char	*s;

	to_work();
	PRINTW("Command: ");
	getyx(stdscr,j,k);
	clrtobot();
	move(j,k);

	if (key == '!')
		clr_sh = FALSE;
	else if (key == '%')
		clr_sh = TRUE;

	if ((key != '.') || (bfr_sh[0] == EOS)) {
		if (key != ':')
			*subs = EOS;
		else
			(void)strcpy(subs, bfr_sh);
		refresh();
		rawgets(s = subs,sizeof(subs),TRUE);
		c = FALSE;
		while (*s) {	/* skip leading blanks */
			if (!isspace(*s)) {
				s = strcpy(bfr_sh, s);
				c = TRUE;
				break;
			}
			s++;
		}
		if (c) {	/* trim trailing blanks */
			s += strlen(s);
			while (--s > bfr_sh && isspace(*s))
				*s = EOS;
		} else {
			PRINTW("(no command)");
			showC();
			return;
		}
	} else
		PRINTW("(ditto)\n");

	subs[k = 0] = EOS;
	for (j = 0; bfr_sh[j]; j++) {
		c = bfr_sh[j];
		if (c == '\\') {
			if (bfr_sh[j+1]) {
				subs[k++] = bfr_sh[++j];
				subs[k] = EOS;
			}
		} else if (c == '#') {	/* substitute group */
		int	first	= TRUE, x;
			for (x = 0; x < numfiles; x++) {
				if (GROUPED(x)) {
					s = fixname(x);
					if (!first) {
						(void)strcat(subs, " ");
						k++;
					}
					if (k + strlen(s) < sizeof(subs)-1)
						(void)strcat(subs, s);
					else {
						k = sizeof(subs);
						break;
					}
					k += strlen(subs + k);
					first = FALSE;
				}
			}
		} else if (c == '%') {	/* substitute current file */
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
			PRINTW("? command is too long\n");
			beep();
			break;
		}
	}
	dedshow("> ", subs);
	refresh();

	if (*subs) {
		resetty();
		(void)dedsigs(FALSE);	/* prevent child from killing us */
		if (system(subs) < 0)
			warn("system");
		(void)dedsigs(TRUE);
		rawterm();
		if (clr_sh) dedwait();
	}
	showC();
}
