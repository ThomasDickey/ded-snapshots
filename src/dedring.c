#ifndef	lint
static	char	sccs_id[] = "@(#)dedring.c	1.12 88/06/01 10:06:33";
#endif	lint

/*
 * Title:	dedring.c (ded: ring of directories)
 * Author:	T.E.Dickey
 * Created:	27 Apr 1988
 * Modified:
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
extern	char	*txtalloc();

/*
 * The RING structure saves global data which lets us restore the state
 * of a file-list (see "ded.h"):
 */
typedef	struct	_ring	{
	struct	_ring	*_link;
	char		new_wd[BUFSIZ],
			bfr_sh[BUFSIZ];
	FLIST		*flist;
	char		**top_argv;
	int		top_argc,
			clr_sh,
			curfile,
			dateopt,
			sortord,
			sortopt,
			G_opt,
			I_opt,
			P_opt,
			S_opt,
			U_opt,
#ifdef	Z_RCS_SCCS
			V_opt,
			Z_opt;
#endif	Z_RCS_SCCS
	unsigned	numfiles;
	} RING;

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
	trans(p->new_wd, new_wd);
	(void)strcpy(p->bfr_sh, bfr_sh);
	SAVE(flist);
	SAVE(top_argc);
	SAVE(top_argv);
	SAVE(clr_sh);
	SAVE(curfile);
	SAVE(dateopt);
	SAVE(sortord);
	SAVE(sortopt);
	SAVE(G_opt);
	SAVE(I_opt);
	SAVE(P_opt);
	SAVE(S_opt);
	SAVE(U_opt);
#ifdef	Z_RCS_SCCS
	SAVE(V_opt);
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
	untrans(new_wd, p->new_wd);
	(void)strcpy(bfr_sh, p->bfr_sh);
	UNSAVE(flist);
	UNSAVE(top_argc);
	UNSAVE(top_argv);
	UNSAVE(clr_sh);
	UNSAVE(curfile);
	UNSAVE(dateopt);
	UNSAVE(sortord);
	UNSAVE(sortopt);
	UNSAVE(G_opt);
	UNSAVE(I_opt);
	UNSAVE(P_opt);
	UNSAVE(S_opt);
	UNSAVE(U_opt);
#ifdef	Z_RCS_SCCS
	UNSAVE(V_opt);
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
insert(path, first)
char	*path;
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
	p = DOALLOC(RING,0,1);
	if (!rang)	rang = p;

	save(p);
	(void)strcpy(p->new_wd, bfr);
	if (first) {
		p->flist    = 0;
		p->top_argc = 1;
		p->top_argv = DOALLOC(char *,0, 2);
		p->top_argv[0] = txtalloc(path);
		p->clr_sh   = FALSE;
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

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/
dedring(path, cmd, count)
char	*path;
{
RING	*oldp,
	*newp	= 0;
int	success	= TRUE;
char	tmp[BUFSIZ];

	/*
	 * Save the current state
	 */
	oldp = insert(new_wd, FALSE);

	/*
	 * Get the appropriate state:
	 */
	switch (cmd) {
	case 'E':
		if ((newp = ring_get(path)) == 0)
			newp = insert(path, TRUE);
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
		while (count-- > 0) {
			if ((newp = ring_fwd(path)) == oldp)
				return(FALSE);
			remove(path);
			path = strcpy (tmp, newp->new_wd);
		}
		break;
	case 'Q':		/* release & move backward */
		path = new_wd;
		while (count-- > 0) {
			if ((newp = ring_bak(path)) == oldp)
				return(FALSE);
			remove(path);
			path = strcpy (tmp, newp->new_wd);
		}
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
#ifndef	SYSTEM5
			/*
			 * Coerce translation of pathnames in case part of the
			 * path was a symbolic link.  We assume that 'getwd()'
			 * does the work:
			 */
			if (chdir(new_wd) >= 0) {
				if (strcmp(getwd(new_wd), path)) {
					remove (path);
					if (!(newp = ring_get(new_wd)))
						newp = insert(new_wd, TRUE);
					unsave(newp);
				}
			} else {
				success = FALSE;
			}
			if (success && numfiles == 0)
#endif	SYSTEM5
			if (dedscan(newp->top_argc, newp->top_argv)) {
				curfile = 0;
				dedsort();
				curfile = 0;	/* ensure consistent initial */
			} else {
				success	= FALSE;
			}

			if (!success) {
				remove (path);
				unsave(oldp);
				(void)chdir(new_wd);
			}
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
RING	*oldp	= insert(new_wd, FALSE),
	*newp;
static
char	show[BUFSIZ];
char	temp[BUFSIZ],
	*path	= new_wd;

	if (count > 0) {
		while (count-- > 0) {
			if ((newp = ring_fwd(path)) == oldp)
				break;
			untrans(path = temp, newp->new_wd);
		}
	} else if (count < 0) {
		while (count++ < 0) {
			if ((newp = ring_bak(path)) == oldp)
				break;
			untrans(path = temp, newp->new_wd);
		}
	}
	return (strcpy(show, path));
}
