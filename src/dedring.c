#ifndef	lint
static	char	sccs_id[] = "@(#)dedring.c	1.2 88/04/28 16:04:08";
#endif	lint

/*
 * Title:	dedring.c (ded: ring of directories)
 * Author:	T.E.Dickey
 * Created:	27 Apr 1988
 * Modified:
 *
 * Function:	Save the current state of the directory editor and scan
 *		a new directory.
 */

#include	"ded.h"
#include	<sys/errno.h>
extern	int	errno;
extern	char	*stralloc();

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
#ifdef	Z_SCCS
			V_opt,
			Z_opt;
#endif	Z_SCCS
	unsigned	numfiles;
	} RING;

static	RING	*ring;

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
#ifdef	Z_SCCS
	SAVE(V_opt);
	SAVE(Z_opt);
#endif	Z_SCCS
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
#ifdef	Z_SCCS
	UNSAVE(V_opt);
	UNSAVE(Z_opt);
#endif	Z_SCCS
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
	p = (RING *)doalloc((char *)0, sizeof(RING));

	save(p);
	(void)strcpy(p->new_wd, bfr);
	if (first) {
		p->flist    = 0;
		p->top_argc = 1;
		p->top_argv = (char **)doalloc((char *)0, 2 * sizeof(char **));
		p->top_argv[0] = stralloc(path);
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
lookup(path)
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
RING	*p	= lookup(path),
	*q	= ring;

	if (p) {
		if (q == p)
			ring->_link = p->_link;
		else {
			while (q) {
				if (q->_link == p)
					q->_link = p->_link;
				q = q->_link;
			}
		}
		if (p->flist)	free (p->flist);
		if (p->top_argv) {
			if (p->top_argv[0]) {
				free (p->top_argv[0]);
				free ((char *)p->top_argv);
			}
		}
		free (p);
	}
}

/*
 * Scroll forward through the directory-list past the given pathname
 */
static
RING *
forward(path)
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
backward(path)
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

	/*
	 * Save the current state
	 */
	oldp = insert(new_wd, FALSE);

	/*
	 * Get the appropriate state:
	 */
	switch (cmd) {
	case 'E':
		if ((newp = lookup(path)) == 0)
			newp = insert(path, TRUE);
		break;
	case 'F':
		while (count-- > 0) {
			if ((newp = forward(path)) == oldp)
				return (FALSE);
		}
		break;
	case 'B':
		while (count-- > 0) {
			if ((newp = backward(path)) == oldp)
				return (FALSE);
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
			if (dedscan(newp->top_argc, newp->top_argv)) {
				curfile = 0;
				dedsort();
				curfile = 0;	/* ensure consistent initial */
			} else {
				remove (path);
				unsave(oldp);
				errno = ENOENT;
				warn(path);
				return (FALSE);
			}
		}
	}
	return (TRUE);
}

/*
 * Returns true if the given path is present in the ring
 */
dedrang(path)
char	*path;
{
	return (lookup(path) != 0);
}
