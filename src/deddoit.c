#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: deddoit.c,v 12.10 1994/07/24 00:33:59 tom Exp $";
#endif

/*
 * Title:	deddoit.c (do it for ded!)
 * Author:	T.E.Dickey
 * Created:	17 Nov 1987
 * Modified:
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		13 May 1992, corrected handling of errors in 'system()'
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		28 Feb 1992, use dynamic-strings to remove buffer-length limits
 *		18 Oct 1991, converted to ANSI
 *		24 Jul 1991, added codes u,g,v,y
 *		18 Apr 1991, modified interface of 'dedwait()'
 *		16 Apr 1991, absorb backslash only when it precedes "#" or "%",
 *			     to make typing commands with backslashes simpler
 *			     (though inconsistent).  Also, made the static
 *			     buffers auto (cleaner code).
 *		30 Jan 1990, pass 'sense' as argument to 'deddoit()' so user
 *			     can alter the 'clr_sh' flag explicitly.
 *		14 Mar 1989, interface to 'dlog' module.
 *		03 Aug 1988, Use 'dedsigs()' so we can fix signals at one point.
 *		02 Aug 1988, so that if nothing is read from 'rawgets()', we
 *			     don't overwrite the last contents of 'cmd_sh[]'.
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

/*
 * Return a pointer to a leaf of a given name
 */
private	char *	subleaf (
	_AR1(char *,	name))
	_DCL(char *,	name)
{
	char	*leaf	= name;

#ifdef	apollo
	if (*leaf == '/')	leaf++;
#endif
	if ((leaf = strrchr(leaf, '/')) != NULL)
		leaf++;
	else
		leaf = name;
	return (leaf);
}

/*
 * Return a pointer to the "." extension of a given name.
 */
private	char *	subroot (
	_AR1(char *,	name))
	_DCL(char *,	name)
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
private	void	Expand(
	_ARX(RING *,	gbl)
	_ARX(int,	code)
	_AR1(DYN *,	subs)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	code)
	_DCL(DYN *,	subs)
{
	char *	cur_name =  cNAME;
	Stat_t *cur_stat = &cSTAT;
	FLIST *	cur_item = &cENTRY;
	char	temp[BUFSIZ],
		name[BUFSIZ],
		*from;

	if (strchr("NHRET", code))
		abspath(pathcat(name, gbl->new_wd, cur_name));
	else
		(void)strcpy(name, cur_name);

	switch(code) {
	case 'F':	from = ring_path(gbl,1);
			break;

	case 'B':	from = ring_path(gbl,-1);
			break;

	case 'D':	from = old_wd;	/* original working directory */
			break;

	case 'd':	from = gbl->new_wd;	/* current working directory */
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

			/* non-pathname attributes */
	case 'u':	from = uid2s((int)(cur_stat->st_uid));	break;
	case 'g':	from = gid2s((int)(cur_stat->st_gid));	break;
#ifdef	Z_RCS_SCCS
	case 'v':	if (!(from = cur_item->z_vers)) from = "?";
			break;
	case 'y':	if (!(from = cur_item->z_lock)) from = "?";
			break;
#endif
	default:
			from = "?";
	}

	(void)ded2string(gbl, temp, sizeof(temp), from, TRUE);
	APPEND(subs, temp);
}

/*
 * Prompt for, substitute and execute a shell command.
 */
public	void	deddoit(
	_ARX(RING *,	gbl)
	_ARX(int,	key)
	_AR1(int,	sense)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	key)
	_DCL(int,	sense)
{
	char	prompt[80];
	static	DYN	*Subs;
	register int	c, j;
	register char	*s;

	dyn_init(&Subs, BUFSIZ);
	if (!dyn_string(gbl->cmd_sh))
		dyn_init(&gbl->cmd_sh, BUFSIZ);

	if (sense == 0)
		gbl->clr_sh = FALSE;
	else if (sense > 1)
		gbl->clr_sh = TRUE;

	FORMAT(prompt, "%c Command: ", gbl->clr_sh ? '%' : '!');

	if ((key != '.') || (*dyn_string(gbl->cmd_sh) == EOS)) {
		if (key == ':')
			APPEND(Subs, dyn_string(gbl->cmd_sh));

		c = FALSE;
		if (!(s = dlog_string(gbl, prompt, &Subs, (DYN **)0,
				&cmd_history, EOS, 0))) {
			showC(gbl);
			return;
		}
		while (*s) {	/* skip leading blanks */
			if (!isspace(*s)) {
				dyn_init(&gbl->cmd_sh, BUFSIZ);
				APPEND(gbl->cmd_sh, s);
				c = TRUE;
				break;
			}
			s++;
		}
		if (c) {	/* trim trailing blanks */
			(void)strtrim(dyn_string(gbl->cmd_sh));
		} else {
			PRINTW("(no command)");
			showC(gbl);
			return;
		}
	} else {
		dlog_prompt(gbl, prompt);
		PRINTW("(ditto)\n");
	}

	dyn_init(&Subs, BUFSIZ);
	for (j = 0; *(s = dyn_string(gbl->cmd_sh) + j); j++) {
		static	char	This[] = "?",
				Next[] = "?";

		This[0] = s[0];
		Next[0] = s[1];

		if (*This == '\\'
		 && (*Next == '#' || *Next == '%') ) {
			APPEND(Subs, Next);
			j++;
		} else if (*This == '#') {	/* substitute group */
			int	ellipsis = 0,
				others = FALSE,
				len,
				x;

			for (x = 0; x < gbl->numfiles; x++) {
				if (GROUPED(x)) {
					len = strlen(s = fixname(gbl, x));
					if (others++)
						APPEND(Subs, " ");

					if (!ellipsis
					 && (dyn_length(Subs) + len) > 256)
						ellipsis = dyn_length(Subs);
					APPEND(Subs, s);
				}
			}
			if (ellipsis) {
				for (s = dyn_string(Subs) + ellipsis; *s; s++)
					*s |= 0200;
			}

		} else if (*This == '%') {	/* substitute current file */
			if (*Next != EOS)
				j++;
			Expand(gbl, *Next, Subs);
		} else {
			APPEND(Subs, This);
		}
	}
	dedshow(gbl, "> ", dyn_string(Subs));

	if (*dyn_string(Subs)) {
		int	ok = TRUE;
		for (s = dyn_string(Subs); *s; s++)
			if (!isascii(*s))
				*s = toascii(*s);

		cookterm();
		(void)dedsigs(FALSE);	/* prevent child from killing us */
		dlog_comment("execute %s\n", dyn_string(Subs));
		errno = 0;
		if (system(dyn_string(Subs)) < 0 || errno != 0) {
			ok = FALSE;
			warn(gbl, "system");
		}
		(void)dedsigs(TRUE);
		rawterm();
		if (ok && gbl->clr_sh) dedwait(gbl, TRUE);
		dlog_elapsed();
	}
	showC(gbl);
}
