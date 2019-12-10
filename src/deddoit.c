/*
 * Title:	deddoit.c (do it for ded!)
 * Author:	T.E.Dickey
 * Created:	17 Nov 1987
 * Modified:
 *		09 Dec 2019, improve string-handling for non-ASCII chars.
 *		12 Dec 2014, fix coverity warnings
 *		07 Mar 2004, remove K&R support, indent'd
 *		10 Aug 1999, ignore errno if system() doesn't return < 0.
 *		15 Feb 1998, compiler-warnings
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

MODULE_ID("$Id: deddoit.c,v 12.25 2019/12/10 01:59:04 tom Exp $")

/*
 * Return a pointer to a leaf of a given name
 */
static char *
subleaf(char *name)
{
    char *leaf = name;

#ifdef	apollo
    if (*leaf == '/')
	leaf++;
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
static char *
subroot(char *name)
{
    char *root;

    if (!(root = strrchr(name, '.')))
	root = name + strlen(name);
    return (root);
}

/*
 * Perform '%' expansions for current-entry.  The substitutions are modified
 * from the ":" modifiers defined for "csh".
 */
static void
Expand(RING * gbl, int code, DYN * subs)
{
    char *cur_name = cNAME;
    Stat_t *cur_stat = &cSTAT;
    FLIST *cur_item = &cENTRY;
    char temp[MAXPATHLEN];
    char name[MAXPATHLEN];
    const char *from;

    if (strchr("NHRET", code)) {
	abspath(pathcat2(name, gbl->new_wd, cur_name));
    } else if (strlen(cur_name) < sizeof(name)) {
	(void) strcpy(name, cur_name);
    } else {
	(void) strcpy(name, "?");
    }

    switch (code) {
    case 'F':
	from = ring_path(gbl, 1);
	break;

    case 'B':
	from = ring_path(gbl, -1);
	break;

    case 'D':
	from = old_wd;		/* original working directory */
	break;

    case 'd':
	from = gbl->new_wd;	/* current working directory */
	break;

    case 'N':
    case 'n':			/* current entry-name */
	from = name;
	break;

    case 'H':
    case 'h':			/* Remove a pathname component, leaving head */
	from = name;
	*subleaf(name) = EOS;
	if (*name == EOS)
	    (void) strcpy(name, "./");
	break;

    case 'R':
    case 'r':			/* Remove a trailing ".xxx" component, leaving root */
	from = name;
	*subroot(subleaf(name)) = EOS;
	break;

    case 'E':
    case 'e':			/* Remove all but trailing ".xxx" component */
	from = subroot(subleaf(name));
	break;

    case 'T':
    case 't':			/* Remove all leading pathname components, leave tail */
	from = subleaf(name);
	break;

	/* non-pathname attributes */
    case 'u':
	from = uid2s(cur_stat->st_uid);
	break;
    case 'g':
	from = gid2s(cur_stat->st_gid);
	break;
#ifdef	Z_RCS_SCCS
    case 'v':
	if (!(from = cur_item->z_vers))
	    from = "?";
	break;
    case 'o':
	if (!(from = cur_item->z_lock))
	    from = "?";
	break;
#endif
    default:
	from = "?";
    }

    (void) ded2string(gbl, temp, sizeof(temp), from, TRUE);
    dyn_append(subs, temp);
}

/*
 * Prompt for, substitute and execute a shell command.
 */
void
deddoit(RING * gbl, int key, int sense)
{
    char prompt[80];
    static DYN *Subs;
    int c, j;
    char *s, *d;

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
	    dyn_append(Subs, dyn_string(gbl->cmd_sh));

	c = FALSE;
	if (!(s = dlog_string(gbl, prompt, -1, &Subs, (DYN **) 0,
			      &cmd_history, EOS, 0))) {
	    showC(gbl);
	    return;
	}
	while (*s) {		/* skip leading blanks */
	    if (!isspace(UCH(*s))) {
		dyn_init(&gbl->cmd_sh, BUFSIZ);
		dyn_append(gbl->cmd_sh, s);
		c = TRUE;
		break;
	    }
	    s++;
	}
	if (c) {		/* trim trailing blanks */
	    (void) strtrim(dyn_string(gbl->cmd_sh));
	} else {
	    PRINTW("(no command)");
	    showC(gbl);
	    return;
	}
    } else {
	dlog_prompt(gbl, prompt, -1);
	PRINTW("(ditto)\n");
    }

    dyn_init(&Subs, BUFSIZ);
    for (j = 0; *(s = dyn_string(gbl->cmd_sh) + j); j++) {
	static char This[2];
	static char Next[2];

	This[0] = s[0];
	Next[0] = s[1];

	if (*This == '\\'
	    && (*Next == '#' || *Next == '%')) {
	    dyn_append(Subs, Next);
	    j++;
	} else if (*This == '#') {	/* substitute group */
	    int ellipsis = FALSE;
	    int others = FALSE;
	    int len;
	    unsigned x;

	    for_each_file(gbl, x) {
		if (GROUPED(x)) {
		    len = (int) strlen(s = fixname(gbl, x));
		    if (others++)
			dyn_append(Subs, " ");

		    if (!ellipsis && ((int) dyn_length(Subs) + len) > 256) {
			ellipsis = TRUE;
			dyn_append_c(Subs, ELIDE_B);
		    }
		    dyn_append(Subs, s);
		}
	    }
	    if (ellipsis) {
		dyn_append_c(Subs, ELIDE_E);
	    }

	} else if (*This == '%') {	/* substitute current file */
	    if (*Next != EOS)
		j++;
	    Expand(gbl, *Next, Subs);
	} else {
	    dyn_append(Subs, This);
	}
    }
    dedshow(gbl, "> ", dyn_string(Subs));

    if (*dyn_string(Subs)) {
	int ok = TRUE;
	int ch;
	int escaped = FALSE;

	/* strip ellipsis markers */
	for (s = d = dyn_string(Subs); (ch = UCH(*s)) != EOS; s++) {
	    if (ch == BACK_SL) {
		escaped = TRUE;
		*d++ = (char) ch;
	    } else if (escaped) {
		escaped = FALSE;
		*d++ = (char) ch;
	    } else if (ch != ELIDE_B && ch != ELIDE_E) {
		*d++ = (char) ch;
	    }
	}
	*d = EOS;

	cookterm();
	(void) dedsigs(FALSE);	/* prevent child from killing us */
	dlog_comment("execute %s\n", dyn_string(Subs));
	errno = 0;
	if (system(dyn_string(Subs)) < 0) {
	    ok = FALSE;
	    warn(gbl, "system");
	}
	(void) dedsigs(TRUE);
	rawterm();
	if (ok && gbl->clr_sh)
	    dedwait(gbl, TRUE);
	dlog_elapsed();
    }
    showC(gbl);
}
