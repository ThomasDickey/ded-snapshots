#ifndef	lint
static	char	Id[] = "$Id: deddoit.c,v 9.0 1991/05/15 13:38:37 ste_cm Rel $";
#endif

/*
 * Title:	deddoit.c (do it for ded!)
 * Author:	T.E.Dickey
 * Created:	17 Nov 1987
 * $Log: deddoit.c,v $
 * Revision 9.0  1991/05/15 13:38:37  ste_cm
 * BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *
 *		Revision 8.3  91/05/15  13:38:37  dickey
 *		apollo sr10.3 cpp complains about tag on #endif
 *		
 *		Revision 8.2  91/04/18  08:10:16  dickey
 *		modified interface of 'dedwait()'
 *		
 *		Revision 8.1  91/04/16  08:51:20  dickey
 *		absorb backslash only when it precedes "#" or "%", to make
 *		typing commands with backslashes simpler (though inconsistent).
 *		also, made the static buffers auto (cleaner code).
 *		
 *		Revision 8.0  90/03/06  08:27:14  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  90/03/06  08:27:14  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/03/06  08:27:14  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.2  90/03/06  08:27:14  dickey
 *		lint
 *		
 *		Revision 5.1  90/01/30  08:43:30  dickey
 *		pass 'sense' as argument to 'deddoit()' so user can alter the
 *		'clr_sh' flag explicitly.
 *		
 *		Revision 5.0  89/03/14  13:19:15  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.0  89/03/14  13:19:15  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/03/14  13:19:15  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/14  13:19:15  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.16  89/03/14  13:19:15  dickey
 *		sccs2rcs keywords
 *		
 *		14 Mar 1989, interface to 'dlog' module.
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
#endif
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
Expand(code, b_subs, l_subs)
char	*b_subs;
{
	char	temp[BUFSIZ];
	char	name[BUFSIZ],
	*from	= 0;

	if (strchr("NHRET", (size_t)code))
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
	default:
			from = "?";
	}

	if (from) {
		(void)ded2string(temp, sizeof(temp), from, TRUE);
		if (strlen(b_subs) + strlen(temp) < l_subs - 1)
			(void)strcat(b_subs, temp);
		else
			return(l_subs);
	}
	return(strlen(b_subs));
}

/*
 * Prompt for, substitute and execute a shell command.
 */
deddoit(key,sense)
{
	register int	c, j, k;
	register char	*s;
	char	Subs[BUFSIZ];

	if (sense == 0)
		clr_sh = FALSE;
	else if (sense > 1)
		clr_sh = TRUE;

	to_work();
	PRINTW("%c Command: ", clr_sh ? '%' : '!');
	getyx(stdscr,j,k);
	clrtobot();
	move(j,k);

	if ((key != '.') || (bfr_sh[0] == EOS)) {
		if (key != ':')
			*Subs = EOS;
		else
			(void)strcpy(Subs, bfr_sh);
		refresh();
		dlog_string(s = Subs,sizeof(Subs),TRUE);
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

	Subs[k = 0] = EOS;
	for (j = 0; bfr_sh[j]; j++) {
		c = bfr_sh[j];
		if (c == '\\'
		 && (bfr_sh[j+1] == '#' || bfr_sh[j+1] == '%') ) {
			Subs[k++] = bfr_sh[++j];
			Subs[k] = EOS;
		} else if (c == '#') {	/* substitute group */
		int	first	= TRUE, x;
			for (x = 0; x < numfiles; x++) {
				if (GROUPED(x)) {
					s = fixname(x);
					if (!first) {
						(void)strcat(Subs, " ");
						k++;
					}
					if (k + strlen(s) < sizeof(Subs)-1)
						(void)strcat(Subs, s);
					else {
						k = sizeof(Subs);
						break;
					}
					k += strlen(Subs + k);
					first = FALSE;
				}
			}
		} else if (c == '%') {	/* substitute current file */
			c = (bfr_sh[j+1] != EOS) ? bfr_sh[++j] : '?';
			k = Expand(c, Subs, sizeof(Subs));
		} else {
			Subs[k++] = c;
			Subs[k] = EOS;
		}
		if (k >= sizeof(Subs)-1) {
			*Subs = EOS;
			PRINTW("? command is too long\n");
			beep();
			break;
		}
	}
	dedshow("> ", Subs);
	refresh();

	if (*Subs) {
		resetty();
		(void)dedsigs(FALSE);	/* prevent child from killing us */
		dlog_comment("execute %s\n", Subs);
		if (system(Subs) < 0)
			warn("system");
		(void)dedsigs(TRUE);
		rawterm();
		if (clr_sh) dedwait(TRUE);
		dlog_elapsed();
	}
	showC();
}
