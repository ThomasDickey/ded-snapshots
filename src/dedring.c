#ifndef	lint
static	char	Id[] = "$Id: dedring.c,v 10.18 1992/04/03 11:00:56 dickey Exp $";
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
private	MakeSortKey _ONE(RING *,gbl)
{
	auto	char	*dst;
	register int	c;
	dst = dyn_string(gbl->sort_key = dyn_copy(gbl->sort_key, gbl->new_wd));
	do {
		switch (c = *dst) {
		case '\n':	c = '/';	break;
		case '/':	c = '\n';	break;
		}
	} while (*dst++ = c);
}

#ifdef	TEST
/*
 * Dump the list of ring-paths
 */
private	void	dump_ring(
	_ARX(RING *,	gbl)
	_AR1(char *,	tag)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	tag)
{
	auto	RING	*p;

	dlog_comment("RING-%s:\n", tag);
	for (p = ring; p; p = p->_link) {
		dlog_comment("%c%#8x \"%s\"\n",
			!strcmp(p->new_wd, gbl->new_wd) ? '>' : ' ',
			p, p->new_wd);
	}
}
#else
#define	dump_ring(g,s)
#endif

/*
 * Save the global state into our local storage
 */
#define	SAVE(n)		dst->n = src->n
private	void	ring_copy(
	_ARX(RING *,	dst)
	_AR1(RING *,	src)
		)
	_DCL(RING *,	dst)
	_DCL(RING *,	src)
{
	register int	j;

	if (dst == src)
		return;

	dst->cmd_sh = dyn_copy(dst->cmd_sh, dyn_string(src->cmd_sh));
	(void) strcpy(dst->new_wd, src->new_wd);
	MakeSortKey(dst);
	SAVE(toscan);
	SAVE(scan_expr);
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
 * Find the given path in the directory list (sorted in the same way that
 * 'ftree' would sort them).
 * patch: main program quit exits only one list at a time (except 'Q' command)
 */
private	RING *	Insert(
	_ARX(RING *,	gbl)
	_ARX(char *,	path)
	_AR1(char *,	pattern)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(char *,	pattern)
{
	RING	*p	= ring,
		*q	= 0;

	while (p) {
	int	cmp = strcmp(path, p->new_wd);
		if (cmp == 0) {
			ring_copy(p,gbl); /* overwrite this slot */
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
	ring_copy(p = ring_alloc(), gbl);
	(void)strcpy(p->new_wd, path);
	p->toscan      = pattern;
	p->scan_expr   = 0;
	p->flist       = 0;
	p->top_argc    = 1;
	p->top_argv    = vecalloc(2);
	p->top_argv[0] = txtalloc(path);
	p->curfile     = 0;
	p->numfiles    = 0;

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
	if (dedscan(newp, newp->top_argc, newp->top_argv)) {
		newp->curfile = 0;
		dedsort(newp);
		newp->curfile = 0;	/* ensure consistent initial */
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

	gbl->top_argv = vecalloc((unsigned)(argc + 2));
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

	(void)strcpy(gbl->new_wd, old_wd);
	MakeSortKey(gbl);
	ring = gbl;	/* set initial linked-list */
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
public	RING *	dedring(
	_ARX(RING *,	gbl)		/* current/reference data */
	_ARX(char *,	path)		/* pathname we want to use */
	_ARX(int,	cmd)		/* command (see cases) */
	_ARX(int,	count)		/* repeat-count for command */
	_ARX(int,	set_pattern)
	_AR1(char *,	pattern)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(int,	cmd)
	_DCL(int,	count)
	_DCL(int,	set_pattern)
	_DCL(char *,	pattern)
{
	RING	*oldp	= gbl,
		*newp	= 0;
	int	success	= TRUE;

#ifdef	TEST
	dlog_comment("dedring(%d%c) %s\n", count, cmd, path);
	dump_ring(gbl, "before");
#endif

	/*
	 * Get the appropriate state:
	 */
	switch (cmd) {
	case 'E':
		if ((newp = ring_get(path)) == 0)
			newp = Insert(gbl, path, pattern);
		break;
	case 'F':
		while (count-- > 0) {
			if ((newp = ring_fwd(path)) == oldp)
				return (0);
			path = newp->new_wd;
		}
		break;
	case 'B':
		while (count-- > 0) {
			if ((newp = ring_bak(path)) == oldp)
				return (0);
			path = newp->new_wd;
		}
		break;
	case 'q':		/* release & move forward */
		path = gbl->new_wd;
		if ((newp = ring_fwd(path)) == oldp)
			return(0);
		Remove(path);
		path = newp->new_wd;
		break;
	case 'Q':		/* release & move backward */
		path = gbl->new_wd;
		if ((newp = ring_bak(path)) == oldp)
			return(0);
		Remove(path);
		path = newp->new_wd;
	}

	/*
	 * Make sure we have a new, legal state
	 */
	if (newp == 0)
		return (0);

	/*
	 * If we have opened this directory before, 'numfiles' is nonzero.
	 * If not, scan the directory.
	 */
	if (newp != oldp) {
		if (newp->numfiles == 0) {
#ifdef	Z_RCS_SCCS
			newp->V_opt = 0;
			newp->Y_opt = 0;
			newp->Z_opt = 0;
#endif
#ifdef	S_IFLNK
			newp->AT_opt = 0;

			/*
			 * Coerce translation of pathnames in case part of the
			 * path was a symbolic link.
			 */
			if (path_RESOLVE(newp, newp->new_wd)) {
				if (strcmp(newp->new_wd, path)) {
					RING	*tstp;
					Remove (path);
					if (!(tstp = ring_get(newp->new_wd)))
						tstp = Insert(newp, newp->new_wd, pattern);
					newp = tstp;
				}
			} else {
				success = FALSE;
			}
			if (success && newp->numfiles == 0)
#endif
				success = do_a_scan(newp);

			if (!success)
				Remove (path);

		} else if (set_pattern && (newp->toscan != pattern)) {
			newp->toscan = pattern;
			success	= do_a_scan(newp); /* rescan with pattern */
		}
		if (!success)		/* pop back to last "good" directory */
			(void)chdir(oldp->new_wd);
	} else if (set_pattern && (newp->toscan != pattern)) {
		newp->toscan = pattern;
		if (!(success = do_a_scan(newp)))
			(void)chdir(oldp->new_wd);
	}
	dump_ring(gbl, "debug");
#ifdef	TEST
	dlog_comment("...%s\n", success ? "ok" : "not-successful");
	dump_ring(newp, "after");
#endif
	return (success ? newp : 0);
}

/*
 * Returns a pointer to the nth (forward/or backward) RING-struct
 */
public	RING *	ring_pointer(
	_ARX(RING *,	gbl)
	_AR1(int,	count)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	count)
{
	auto	char	*path	= gbl->new_wd;
	auto	RING	*oldp	= gbl,
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
public	char *	ring_path(
	_ARX(RING *,	gbl)
	_AR1(int,	count)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	count)
{
	static	char	show[BUFSIZ];

	return (strcpy(show, ring_pointer(gbl, count)->new_wd));
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
	dump_ring(gbl, "before");
#endif

	for (p = ring; (p != 0) && (p != mark); p = q) {
		q = p->_link;
		(void) strcpy(tmp, p->new_wd);
		if ((len = is_subpath(oldname, tmp)) >= 0) {
			old = *p;
			(void) DeLink(tmp);

			(void)pathcat(tmp, newname, tmp + len);
			r = gbl;

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
	dump_ring(gbl, "after");
}
