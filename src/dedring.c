#ifndef	lint
static	char	Id[] = "$Id: dedring.c,v 8.1 1991/04/04 09:08:41 dickey Exp $";
#endif	lint

/*
 * Title:	dedring.c (ded: ring of directories)
 * Author:	T.E.Dickey
 * Created:	27 Apr 1988
 * $Log: dedring.c,v $
 * Revision 8.1  1991/04/04 09:08:41  dickey
 * guard against 'getwd()' failure.
 *
 *		Revision 8.0  90/05/23  11:16:30  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.2  90/05/23  11:16:30  dickey
 *		added a missing case to handle 'set_pattern' arg of 'dedring'
 *		(when we are using CTL(E) command like CTL(R)).
 *		
 *		Revision 7.1  90/05/23  09:30:35  dickey
 *		modified 'dedring()' so that an initial scan-pattern can be
 *		specified, to support the CTL(E) command.
 *		
 *		Revision 7.0  90/03/02  12:09:20  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/03/02  12:09:20  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.2  90/03/02  12:09:20  dickey
 *		set 'no_worry' flag after successfully reading new-directory
 *		
 *		Revision 5.1  90/01/30  08:42:26  dickey
 *		save/restore T_opt
 *		
 *		Revision 5.0  89/10/05  16:58:38  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.2  89/10/05  16:58:38  dickey
 *		save/restore 'cmdcol[]' per-list
 *		
 *		Revision 4.1  89/10/04  15:20:09  dickey
 *		save/restore A_opt, O_opt
 *		
 *		Revision 4.0  89/05/26  14:15:32  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/05/26  14:15:32  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.2  89/05/26  14:15:32  dickey
 *		don't inherit read-expression in new directory (too confusing)
 *		
 *		Revision 2.1  89/05/26  13:06:58  dickey
 *		added 'toscan', 'scan_expr' to ring-data.
 *		don't reset 'clr_sh' on entry to ring -- do this only in
 *		'deddoit()'
 *		
 *		Revision 2.0  88/09/12  15:52:31  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.20  88/09/12  15:52:31  dickey
 *		sccs2rcs keywords
 *		
 *		01 Aug 1988, save/restore Xbase,Ybase
 *		11 Jul 1988, added 'tagsort'.
 *		08 Jul 1988, save/restore/clear Y_opt, AT_opt a la Z_opt.
 *		27 Jun 1988, made 'dedrung()' work ok with count.
 *		16 Jun 1988, added code to save/restore AT_opt.
 *		25 May 1988, don't force V/Z-mode continuation on dedscan.
 *		18 May 1988, added 'dedrung()' entry.
 *		06 May 1988, added coercion for paths which may contain a
 *			     symbolic link.
 *		05 May 1988, added 'Q' command.
 *		02 May 1988, fixed repeat count on 'F', 'B' commands.
 *			     Added 'q' command.
 *
 * Function:	Save the current state of the directory editor and scan
 *		a new directory.
 *
 * patch:	loop on ring_fwd/ring_bak may be confused with 'trans'.
 */

#include	"ded.h"
extern	FLIST	*dedfree();
extern	char	**vecalloc();
extern	char	*txtalloc();
extern	int	no_worry;

/*
 * The RING structure saves global data which lets us restore the state
 * of a file-list (see "ded.h"):
 */
typedef	struct	_ring	{
	struct	_ring	*_link;
	char		new_wd[BUFSIZ],
			*toscan,	/* directory-scan expression	*/
			*scan_expr,	/* compiled version of 'toscan'	*/
			bfr_sh[BUFSIZ];	/* last shell-command		*/
	FLIST		*flist;
	char		**top_argv;
	int		top_argc,
			cmdcol[CCOL_MAX],
			clr_sh,
			Xbase,
			Ybase,
			curfile,
			dateopt,
			sortord,
			sortopt,
			tagsort,
#ifdef	S_IFLNK
			AT_opt,
#endif	S_IFLNK
			A_opt,
			G_opt,
			I_opt,
#ifdef	apollo_sr10
			O_opt,
#endif
			P_opt,
			S_opt,
			T_opt,
			U_opt,
#ifdef	Z_RCS_SCCS
			V_opt,
			Y_opt,
			Z_opt;
#endif	Z_RCS_SCCS
	unsigned	numfiles;
	} RING;

#define	def_alloc	RING_alloc
	/*ARGSUSED*/
	def_ALLOC(RING)

static	RING	*ring,		/* directory-list */
		*rang;		/* reference so we don't free real argv! */

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Translate pathname to internal form, so strcmp will sort it properly
 */
static
trans(dst,src)
char	*dst, *src;
{
register int c;
	do {
		if ((c = *src++) == '/')
			c = '\n';
	} while (*dst++ = c);
}

/*
 * Translate pathname to external form
 */
static
untrans(dst,src)
char	*dst, *src;
{
register int c;
	do {
		if ((c = *src++) == '\n')
			c = '/';
	} while (*dst++ = c);
}

/*
 * Save the global state into our local storage
 */
#define	SAVE(n)		p->n = n
static
save(p)
RING	*p;
{
	register int	j;
	trans(p->new_wd, new_wd);
	SAVE(toscan);
	SAVE(scan_expr);
	(void)strcpy(p->bfr_sh, bfr_sh);
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
#ifdef	S_IFLNK
	SAVE(AT_opt);
#endif	S_IFLNK
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
#endif	Z_RCS_SCCS
	SAVE(numfiles);
}

/*
 * Reload global state from a previously-saved state
 */
#define	UNSAVE(n)	n = p->n
static
unsave(p)
RING	*p;
{
	register int	j;
	untrans(new_wd, p->new_wd);
	UNSAVE(toscan);
	UNSAVE(scan_expr);
	(void)strcpy(bfr_sh, p->bfr_sh);
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
#ifdef	S_IFLNK
	UNSAVE(AT_opt);
#endif	S_IFLNK
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
#endif	Z_RCS_SCCS
	UNSAVE(numfiles);
}

/*
 * Find the given path in the directory list (sorted in the same way that
 * 'ftree' would sort them).
 * patch: main program quit exits only one list at a time (except 'Q' command)
 */
static
RING *
insert(path, first, pattern)
char	*path;
char	*pattern;
{
RING	*p	= ring,
	*q	= 0;
char	bfr[BUFSIZ];

	trans(bfr, path);
	while (p) {
	int	cmp = strcmp(bfr, p->new_wd);
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
	p = ALLOC(RING,1);
	if (!rang)	rang = p;

	save(p);
	(void)strcpy(p->new_wd, bfr);
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
 * Find a directory-list item for a given pathname
 */
static
RING *
ring_get(path)
char	*path;
{
RING	*p;
char	tmp[BUFSIZ];
	trans(tmp, path);

	for (p = ring; p; p = p->_link)
		if (!strcmp(tmp, p->new_wd))
			return (p);
	return (0);
}

/*
 * Release storage for a given entry in the directory-list
 */
static
remove (path)
char	*path;
{
RING	*p	= ring_get(path),
	*q	= ring;

	if (p) {
		if (q == p)
			ring = p->_link;
		else {
			while (q) {
				if (q->_link == p)
					q->_link = p->_link;
				q = q->_link;
			}
		}
		p->flist = dedfree(p->flist, p->numfiles);
		if (p != rang) {
			if (p->top_argv) {
				if (p->top_argv[0]) {
					txtfree (p->top_argv[0]);
					FREE((char *)p->top_argv);
				}
			}
		}
		FREE((char *)p);
	}
}

/*
 * Scroll forward through the directory-list past the given pathname
 */
static
RING *
ring_fwd(path)
char	*path;
{
register RING *p;
char	tmp[BUFSIZ];
	trans(tmp, path);

	for (p = ring; p; p = p->_link) {
	int	cmp = strcmp(tmp, p->new_wd);
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
static
RING *
ring_bak(path)
char	*path;
{
register RING *p, *q;
int	cmp;
char	tmp[BUFSIZ];
	trans(tmp, path);

	if (p = ring) {
		cmp = strcmp(tmp, p->new_wd);
		if (cmp <= 0) {	/* loop-backward to end */
			while (q = p->_link)
				p = q;
		} else while (p) {
			if (q = p->_link) {
				cmp = strcmp(tmp, q->new_wd);
				if (cmp == 0)
					break;
				else if (cmp < 0)
					return (q);
				else
					p = q;
			} else
				break;
		}
	}
	return (p);		/* should not ever get here */
}

static
do_a_scan(newp)
RING	*newp;
{
	if (dedscan(newp->top_argc, newp->top_argv)) {
		curfile = 0;
		dedsort();
		curfile = 0;	/* ensure consistent initial */
		if (no_worry < 0)	/* start worrying! */
			no_worry = FALSE;
		return (TRUE);
	}
	return (FALSE);
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/
dedring(path, cmd, count, set_pattern, pattern)
char	*path;
int	cmd;
int	count;
int	set_pattern;
char	*pattern;
{
RING	*oldp,
	*newp	= 0;
int	success	= TRUE;
char	tmp[BUFSIZ];

	/*
	 * Save the current state
	 */
	oldp = insert(new_wd, FALSE, (char *)0);

	/*
	 * Get the appropriate state:
	 */
	switch (cmd) {
	case 'E':
		if ((newp = ring_get(path)) == 0)
			newp = insert(path, TRUE, pattern);
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
		path = new_wd;
		if ((newp = ring_fwd(path)) == oldp)
			return(FALSE);
		remove(path);
		path = strcpy (tmp, newp->new_wd);
		break;
	case 'Q':		/* release & move backward */
		path = new_wd;
		if ((newp = ring_bak(path)) == oldp)
			return(FALSE);
		remove(path);
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
		if (numfiles == 0) {
#ifdef	Z_RCS_SCCS
			V_opt = 0;
			Y_opt = 0;
			Z_opt = 0;
#endif	Z_RCS_SCCS
#ifdef	S_IFLNK
			AT_opt = 0;

			/*
			 * Coerce translation of pathnames in case part of the
			 * path was a symbolic link.
			 */
			if (path_RESOLVE(new_wd)) {
				if (strcmp(new_wd, path)) {
					remove (path);
					if (!(newp = ring_get(new_wd)))
						newp = insert(new_wd, TRUE, pattern);
					unsave(newp);
				}
			} else {
				success = FALSE;
			}
			if (success && numfiles == 0)
#endif	S_IFLNK
				success = do_a_scan(newp);

			if (!success)
				remove (path);

		} else if (set_pattern && (toscan != pattern)) {
			toscan	= pattern;
			success	= do_a_scan(newp); /* rescan with pattern */
		}
		if (!success) {		/* pop back to last "good" directory */
			unsave(oldp);
			(void)chdir(new_wd);
		}
	} else if (set_pattern && (toscan != pattern)) {
		toscan	= pattern;
		if (!(success = do_a_scan(newp))) {
			unsave(oldp);
			(void)chdir(new_wd);
		}
	}
	return (success);
}

/*
 * Returns true if the given path is present in the ring
 */
dedrang(path)
char	*path;
{
	return (ring_get(path) != 0);
}

/*
 * Returns a pointer to the pathname forward/or backward in the ring.
 */
char *
dedrung(count)
{
RING	*oldp	= insert(new_wd, FALSE, (char *)0),
	*newp;
static
char	show[BUFSIZ];
char	temp[BUFSIZ],
	*path	= new_wd;

	while (count != 0) {
		if (count > 0) {
			newp = ring_fwd(path);
			count--;
		} else if (count < 0) {
			newp = ring_bak(path);
			count++;
		}
		if (newp != 0) {
			untrans(path = temp, newp->new_wd);
			if (newp == oldp)
				break;
		} else
			break;
	}
	return (strcpy(show, path));
}
