#ifndef	lint
static	char	Id[] = "$Id: dedring.c,v 10.15 1992/04/02 16:21:19 dickey Exp $";
#endif

/*
 * Title:	dedring.c (ded: ring of directories)
 * Author:	T.E.Dickey
 * Created:	27 Apr 1988
 * Modified:
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		28 Feb 1992, changed type of 'cmd_sh'.
 *		20 Feb 1992, correction to 'ring_bak()'
 *		17 Feb 1992, added 'ring_rename()' to make renaming work ok.
 *		21 Nov 1991, added 'tag_opt'
 *		18 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		04 Apr 1991, guard against 'getwd()' failure.
 *		23 May 1990, added a missing case to handle 'set_pattern' arg of
 *			     'dedring' (when we are using CTL(E) command like
 *			     CTL(R)).  Modified 'dedring()' so that an initial
 *			     scan-pattern can be specified, to support the
 *			     CTL(E) command.
 *		02 Mar 1990, set 'no_worry' flag after successfully reading new-
 *			     directory.
 *		30 Jan 1990, save/restore T_opt
 *		05 Oct 1989, save/restore 'cmdcol[]' per-list
 *		04 Oct 1989, save/restore A_opt, O_opt
 *		26 May 1989, don't inherit read-expression in new directory (too
 *			     confusing).  Added 'toscan', 'scan_expr' to ring-
 *			     data.  Don't reset 'clr_sh' on entry to ring -- do
 *			     this only in 'deddoit()'
 *		01 Aug 1988, save/restore Xbase,Ybase
 *		11 Jul 1988, added 'tagsort'.
 *		08 Jul 1988, save/restore/clear Y_opt, AT_opt a la Z_opt.
 *		27 Jun 1988, made 'ring_path()' work ok with count.
 *		16 Jun 1988, added code to save/restore AT_opt.
 *		25 May 1988, don't force V/Z-mode continuation on dedscan.
 *		18 May 1988, added 'ring_path()' entry.
 *		06 May 1988, added coercion for paths which may contain a
 *			     symbolic link.
 *		05 May 1988, added 'Q' command.
 *		02 May 1988, fixed repeat count on 'F', 'B' commands.
 *			     Added 'q' command.
 *
 * Function:	Save the current state of the directory editor and scan
 *		a new directory.
 */

#include	"ded.h"

#define	def_alloc	RING_alloc
	/*ARGSUSED*/
	def_ALLOC(RING)

static	RING	*ring;		/* directory-list */

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Translate slashes to newlines, forcing strcmp to yield a nice sort-compare.
 * Just in case we had a newline there, make it a slash (unlikely).
 */
private	char *	Toggle(
	_ARX(char *,	dst)
	_AR1(char *,	src)
		)
	_DCL(char *,	dst)
	_DCL(char *,	src)
{
	auto	char	*base = dst;
	register int	c;
	do {
		switch (c = *src++) {
		case '\n':	c = '/';	break;
		case '/':	c = '\n';	break;
		}
	} while (*dst++ = c);
	return base;
}

#ifdef	TEST
/*
 * Dump the list of ring-paths
 */
private	void	dump_ring _ONE(char *,tag)
{
	auto	RING	*p;

	dlog_comment("RING-%s:\n", tag);
	for (p = ring; p; p = p->_link) {
		dlog_comment("%c%#8x \"%s\"\n",
			!strcmp(p->new_wd, FOO->new_wd) ? '>' : ' ',
			p, p->new_wd);
	}
}
#else
#define	dump_ring(s)
#endif

/*
 * Save the global state into our local storage
 */
#define	SAVE(n)		p->n = FOO->n
private	void	save _ONE(RING *,p)
{
	char	temp[MAXPATHLEN];
	register int	j;

	dyn_init(&(p->sort_key), MAXPATHLEN);
	(void) dyn_append(p->sort_key, Toggle(temp, FOO->new_wd));
	(void) strcpy(p->new_wd, FOO->new_wd);
	SAVE(toscan);
	SAVE(scan_expr);
	dyn_init(&(p->cmd_sh), BUFSIZ); APPEND(p->cmd_sh, dyn_string(FOO->cmd_sh));
	SAVE(flist);
	SAVE(top_argc);
	for (j = 0; j < CCOL_MAX; j++) SAVE(cmdcol[j]);
	SAVE(top_argv);
	SAVE(clr_sh);
	SAVE(Xbase);
	SAVE(Ybase);
	SAVE(curfile);
	SAVE(dateopt);
	SAVE(sortord);
	SAVE(sortopt);
	SAVE(tagsort);
	SAVE(tag_opt);
#ifdef	S_IFLNK
	SAVE(AT_opt);
#endif
	SAVE(A_opt);
	SAVE(G_opt);
	SAVE(I_opt);
#ifdef	apollo_sr10
	SAVE(O_opt);
#endif
	SAVE(P_opt);
	SAVE(S_opt);
	SAVE(T_opt);
	SAVE(U_opt);
#ifdef	Z_RCS_SCCS
	SAVE(V_opt);
	SAVE(Y_opt);
	SAVE(Z_opt);
#endif
	SAVE(numfiles);
}

/*
 * Reload global state from a previously-saved state
 */
#define	UNSAVE(n)	FOO->n = p->n
private	void	unsave _ONE(RING *,p)
{
	register int	j;
	(void) strcpy(FOO->new_wd, p->new_wd);
	UNSAVE(toscan);
	UNSAVE(scan_expr);
	dyn_init(&FOO->cmd_sh, BUFSIZ); APPEND(FOO->cmd_sh, dyn_string(p->cmd_sh));
	UNSAVE(flist);
	UNSAVE(top_argc);
	for (j = 0; j < CCOL_MAX; j++) UNSAVE(cmdcol[j]);
	UNSAVE(top_argv);
	UNSAVE(clr_sh);
	UNSAVE(Xbase);
	UNSAVE(Ybase);
	UNSAVE(curfile);
	UNSAVE(dateopt);
	UNSAVE(sortord);
	UNSAVE(sortopt);
	UNSAVE(tagsort);
	UNSAVE(tag_opt);
#ifdef	S_IFLNK
	UNSAVE(AT_opt);
#endif
	UNSAVE(A_opt);
	UNSAVE(G_opt);
	UNSAVE(I_opt);
#ifdef	apollo_sr10
	UNSAVE(O_opt);
#endif
	UNSAVE(P_opt);
	UNSAVE(S_opt);
	UNSAVE(T_opt);
	UNSAVE(U_opt);
#ifdef	Z_RCS_SCCS
	UNSAVE(V_opt);
	UNSAVE(Y_opt);
	UNSAVE(Z_opt);
#endif
	UNSAVE(numfiles);
}

/*
 * Find the given path in the directory list (sorted in the same way that
 * 'ftree' would sort them).
 * patch: main program quit exits only one list at a time (except 'Q' command)
 */
private	RING *	Insert(
	_ARX(char *,	path)
	_ARX(int,	first)
	_AR1(char *,	pattern)
		)
	_DCL(char *,	path)
	_DCL(int,	first)
	_DCL(char *,	pattern)
{
	RING	*p	= ring,
		*q	= 0;

	while (p) {
	int	cmp = strcmp(path, p->new_wd);
		if (cmp == 0) {
			save(p);
			return (p);
		} else if (cmp < 0)
			break;
		q = p;
		p = p->_link;
	}

	/*
	 * Make a new entry, using all of the current state except for
	 * the actual file-list
	 */
	p = ring_alloc();
	p->cmd_sh = 0;

	save(p);
	(void)strcpy(p->new_wd, path);
	if (first) {
		p->toscan   = pattern;
		p->scan_expr= 0;
		p->flist    = 0;
		p->top_argc = 1;
		p->top_argv = vecalloc(2);
		p->top_argv[0] = txtalloc(path);
		p->curfile  = 0;
		p->numfiles = 0;
	}

	if (q) {
		p->_link	= q->_link;
		q->_link	= p;
	} else {
		p->_link	= ring;
		ring		= p;
	}
	return (p);
}

/*
 * De-link directory-list data from the ring, retaining a pointer to the
 * delinked-structure.
 */
private	RING *	DeLink _ONE(char *,path)
{
	RING	*p	= ring_get(path),
		*q	= ring;

	if (p != 0) {
		if (q == p)
			ring = p->_link;
		else {
			while (q) {
				if (q->_link == p)
					q->_link = p->_link;
				q = q->_link;
			}
		}
	}
	return p;
}

/*
 * Release storage for a given entry in the directory-list
 */
private	void	Remove _ONE(char *,path)
{
	RING	*p;

	if (p = DeLink(path)) {
		p->flist = dedfree(p->flist, p->numfiles);
		if (p->top_argv) {
			if (p->top_argv[0]) {
				txtfree (p->top_argv[0]);
				FREE((char *)p->top_argv);
				/* patch: should free all items in vector */
			}
		}
		FREE((char *)p);
	}
}

/*
 * Scroll forward through the directory-list past the given pathname
 */
private	RING *	ring_fwd _ONE(char *,path)
{
	register RING *p;

	for (p = ring; p; p = p->_link) {
		int	cmp = strcmp(path, p->new_wd);
		if (cmp == 0) {
			if (p = p->_link)
				return (p);
			else
				break;	/* force loop-around */
		} else if (cmp < 0)
			return (p);
	}
	return (ring);
}

/*
 * Scroll backward through the directory list, immediately before the given path
 */
private	RING *	ring_bak _ONE(char *,path)
{
	register RING *p, *q;

	if (p = ring) {
		if (strcmp(path, p->new_wd) <= 0) {
			while (q = p->_link)
				p = q;
		} else {
			while (p) {
				if (q = p->_link) {
					if (strcmp(path, q->new_wd) <= 0)
						break;
					p = q;
				} else
					break;
			}
		}
	}
	return (p);		/* should not ever get here */
}

private	int	do_a_scan _ONE(RING *,newp)
{
	if (dedscan(FOO, newp->top_argc, newp->top_argv)) {
		FOO->curfile = 0;
		dedsort(FOO);
		FOO->curfile = 0;	/* ensure consistent initial */
		if (no_worry < 0)	/* start worrying! */
			no_worry = FALSE;
		return (TRUE);
	}
	return (FALSE);
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * Allocates a new RING structure (used here and in main-program)
 */
public	RING *	ring_alloc()
{
	static	RING q;
	register RING *p = ALLOC(RING,1);
	*p = q;
	return p;
}

/*
 * Initializes the argc/argv portion of a RING structure, given a (argc,argv)
 * pair.  We copy 'argv[]' so we can reallocate it; also trim repeated items.
 */
public	void	ring_args(
	_ARX(RING *,	gbl)
	_ARX(int,	argc)
	_AR1(char **,	argv)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	argc)
	_DCL(char **,	argv)
{
	register int	j, k;
	int	new_argc = 0;

	gbl->top_argv = vecalloc((unsigned)(argc + 1));
	for (j = 0; j < argc; j++) {	/* already adjusted with 'optind' */
		char *s = txtalloc(argv[j]);
		if (*s == EOS)
			continue;
		for (k = 0; k < j; k++)
			/* look for repeats (same pointer) */
			if (gbl->top_argv[k] == s)
				break;
		if (k == j)	/* ... then we never found a repeat */
			gbl->top_argv[new_argc++] = s;
	}

	if (new_argc < 1)
		gbl->top_argv[new_argc++] = ".";

	gbl->top_argv[new_argc] = 0; /* always keep a null pointer on end */
	gbl->top_argc = new_argc;
}

/*
 * Find a directory-list item for a given pathname
 */
public	RING *	ring_get _ONE(char *,path)
{
	RING	*p;

	for (p = ring; p; p = p->_link)
		if (!strcmp(path, p->new_wd))
			return (p);
	return (0);
}

/*
 * Returns true iff we successfully perform the operation specified in 'cmd'.
 */
public	int	dedring(
	_ARX(char *,	path)
	_ARX(int,	cmd)
	_ARX(int,	count)
	_ARX(int,	set_pattern)
	_AR1(char *,	pattern)
		)
	_DCL(char *,	path)
	_DCL(int,	cmd)
	_DCL(int,	count)
	_DCL(int,	set_pattern)
	_DCL(char *,	pattern)
{
	RING	*oldp,
		*newp	= 0;
	int	success	= TRUE;
	char	tmp[BUFSIZ];

	/*
	 * Save the current state
	 */
	oldp = Insert(FOO->new_wd, FALSE, (char *)0);

	/*
	 * Get the appropriate state:
	 */
	switch (cmd) {
	case 'E':
		if ((newp = ring_get(path)) == 0)
			newp = Insert(path, TRUE, pattern);
		break;
	case 'F':
		while (count-- > 0) {
			if ((newp = ring_fwd(path)) == oldp)
				return (FALSE);
			path = newp->new_wd;
		}
		break;
	case 'B':
		while (count-- > 0) {
			if ((newp = ring_bak(path)) == oldp)
				return (FALSE);
			path = newp->new_wd;
		}
		break;
	case 'q':		/* release & move forward */
		path = FOO->new_wd;
		if ((newp = ring_fwd(path)) == oldp)
			return(FALSE);
		Remove(path);
		path = strcpy (tmp, newp->new_wd);
		break;
	case 'Q':		/* release & move backward */
		path = FOO->new_wd;
		if ((newp = ring_bak(path)) == oldp)
			return(FALSE);
		Remove(path);
		path = strcpy (tmp, newp->new_wd);
	}

	/*
	 * Make sure we have a new, legal state
	 */
	if (newp == 0)
		return (FALSE);

	/*
	 * If we have opened this directory before, 'numfiles' is nonzero.
	 * If not, scan the directory.
	 */
	if (newp != oldp) {
		unsave(newp);
		if (FOO->numfiles == 0) {
#ifdef	Z_RCS_SCCS
			FOO->V_opt = 0;
			FOO->Y_opt = 0;
			FOO->Z_opt = 0;
#endif
#ifdef	S_IFLNK
			FOO->AT_opt = 0;

			/*
			 * Coerce translation of pathnames in case part of the
			 * path was a symbolic link.
			 */
			if (path_RESOLVE(FOO, FOO->new_wd)) {
				if (strcmp(FOO->new_wd, path)) {
					Remove (path);
					if (!(newp = ring_get(FOO->new_wd)))
						newp = Insert(FOO->new_wd, TRUE, pattern);
					unsave(newp);
				}
			} else {
				success = FALSE;
			}
			if (success && FOO->numfiles == 0)
#endif
				success = do_a_scan(newp);

			if (!success)
				Remove (path);

		} else if (set_pattern && (FOO->toscan != pattern)) {
			FOO->toscan	= pattern;
			success	= do_a_scan(newp); /* rescan with pattern */
		}
		if (!success) {		/* pop back to last "good" directory */
			unsave(oldp);
			(void)chdir(FOO->new_wd);
		}
	} else if (set_pattern && (FOO->toscan != pattern)) {
		FOO->toscan	= pattern;
		if (!(success = do_a_scan(newp))) {
			unsave(oldp);
			(void)chdir(FOO->new_wd);
		}
	}
	dump_ring("debug");
	return (success);
}

/*
 * Returns a pointer to the nth (forward/or backward) RING-struct
 */
public	RING *	ring_pointer _ONE(int,count)
{
	auto	char	*path	= FOO->new_wd;
	auto	RING	*oldp	= Insert(path, FALSE, (char *)0),
			*newp;

	while (count != 0) {
		if (count > 0) {
			newp = ring_fwd(path);
			count--;
		} else if (count < 0) {
			newp = ring_bak(path);
			count++;
		}
		path = newp->new_wd;
		if (newp == oldp)
			break;
	}
	return newp;
}

/*
 * Returns a pointer to the pathname forward/or backward in the ring.
 */
public	char *	ring_path _ONE(int,count)
{
	static	char	show[BUFSIZ];

	return (strcpy(show, ring_pointer(count)->new_wd));
}

/*
 * Tells this module that a directory has been renamed.  Adjusts our list to
 * match, as well as the current-list pointer.
 *
 * For each ring-entry, if 'oldname[]' is a proper prefix of the path, we
 * remove/insert the entry to move it.
 */
public	void	ring_rename(
	_ARX(RING *,	gbl)
	_ARX(char *,	oldname)
	_AR1(char *,	newname)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	oldname)
	_DCL(char *,	newname)
{
	register RING	*p, *q, *r;
	register RING	*mark	= 0;
	register RING	*curr	= ring_get(gbl->new_wd);
	auto	 RING	old;
	auto	 int	len, n;
	auto	 char	oldtemp[MAXPATHLEN],
			newtemp[MAXPATHLEN],
			tmp[MAXPATHLEN];

	abspath(oldname = pathcat(oldtemp, gbl->new_wd, oldname));
	abspath(newname = pathcat(newtemp, gbl->new_wd, newname));

#ifdef	TEST
	dlog_comment("ring_rename\n");
	dlog_comment("...old:%s\n", oldname);
	dlog_comment("...new:%s\n", newname);
	dump_ring("before");
#endif

	for (p = ring; (p != 0) && (p != mark); p = q) {
		q = p->_link;
		(void) strcpy(tmp, p->new_wd);
		if ((len = is_subpath(oldname, tmp)) >= 0) {
			old = *p;
			(void) DeLink(tmp);

			(void)pathcat(tmp, newname, tmp + len);
			r = Insert(tmp, FALSE, (char *)0);

			(void)strcpy(old.new_wd, r->new_wd);
			old._link = r->_link;
			*r = old;

			for (n = 0; n < r->top_argc; n++) {
				register char	*s = r->top_argv[n];
				auto	char	tmp2[MAXPATHLEN];

				if ((len = is_subpath(oldname, s)) < 0)
					continue;
				r->top_argv[n] = txtalloc(
					pathcat(tmp2, newname, s + len));
			}

#ifdef	TEST
			dlog_comment(".moved:%s\n", tmp);
#endif
			if (p == curr) {
				int	ok = chdir(strcpy(gbl->new_wd, tmp));
				dlog_comment("RING-chdir %s =>%d\n", tmp,ok);
			}
			if (!mark)
				mark = p;
		}
	}
	dump_ring("after");
}
