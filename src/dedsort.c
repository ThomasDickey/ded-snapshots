/*
 * Title:	dedsort.c (ded-sort)
 * Author:	T.E.Dickey
 * Created:	11 Nov 1987
 * Modified:
 *		04 Mar 1998, rename 'y' sort to 'o'.
 *		15 Feb 1998, remove special code for apollo sr10
 *		12 Jul 1994, defined 'CMPF()' macro for 'ded_blocks()' hack.
 *		06 Dec 1993, added 'S' sort.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		20 Nov 1992, use 'cm_qsort.h' definitions.
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		06 Feb 1992, make 'Z' sort by difference between checkin-time
 *			     and modification-time.
 *		18 Oct 1991, converted to ANSI
 *		17 Jul 1991, added '@' and 'D' sort.
 *		28 Jun 1991, added P-sort (same as p-sort, but keeps "+" for
 *			     apollo-sr10 extended-acls sorted into groups)
 *		22 Jan 1990, corrections to 'v'-sort
 *		12 Oct 1989, refined inode-, uid-, gid-sorts so that if I_opt
 *			     or G_opt are in two-column mode, we sort what the
 *			     user sees.
 *		11 Oct 1989, added apollo-only fix for t-sort for DSEE-directory
 *			     names (ending with "$.*.$").
 *		06 Oct 1989, modified 't' sort so that names beginning with '.'
 *			     are sorted in a more natural manner
 *		04 Oct 1989, added o,O sorts
 *		23 Jan 1989, added 'N' sort.
 *		18 Jan 1989, made 'dedsort_cmp()' public, for use by 'deduniq()'
 *		13 Sep 1988, use external 'ftype()', 'ftype2()'.
 *		27 Jul 1988, corrected 'y' sort, in case no RCS/SCCS file exists
 *		11 Jul 1988, use 'tagsort' variable to group tagged-files.
 *		01 Jun 1988, added 'y' sort.
 *		23 May 1988, use 'dotcmp()' for version, '.'-sorts.
 *		09 May 1988, make devices sort-size separately from files.
 *
 * Function:	Perform display-list sorting for DED directory editor.
 */

#include	<ded.h>
#define	QSORT_SRC	const FLIST
#include	<td_qsort.h>

MODULE_ID("$Id: dedsort.c,v 12.10 1998/03/04 23:50:19 tom Exp $")

#define	CHECKED(p)	(p->z_time == p->s.st_mtime)
#define	CMPF(f)	(f(&(p1->s)) > f(&(p2->s)) ? -1 : (f(&(p1->s)) < f(&(p2->s)) ? 1 : 0))
#define	CMPX(m)		(p1->m > p2->m ? -1 : (p1->m < p2->m ? 1 : 0))
#define	CMP(m)		CMPX(s.m)
#define	CMP2S(f,m)	cmp = strcmp(strcpy(bfr, f((int)p1->s.m)),\
						 f((int)p2->s.m))

/* sort types so that names beginning with '.' are treated specially */
private	char *	f_type (
	_AR1(char *,	s))
	_DCL(char *,	s)
{
	register char	*t = ftype(s);
	if (t != s) {
		if (t[-1] == '/')
			t = f_type(t);
#ifdef	apollo
		/* dsee directories ? */
		else if (t[-1] == '$') {
			if ((t-1) == s || t[-2] == '/') {
				auto	int	len = strlen(t);
				if (len > 2 && !strncmp(t+len-2, ".$", 2))
					t += strlen(t);
			}
		}
#endif	/* apollo */
	} else {
		if (dotname(t))
			t += strlen(t);
		else
			t = ftype(t+1);
	}
	return (t);
}

/*
 * Compare the specified file-list entries, returning 0 iff their sort-key is
 * equivalent, +/- according to the direction of the inequality.
 */
public	int	dedsort_cmp(
	_ARX(RING *,		gbl)
	_ARX(const FLIST *,	p1)
	_AR1(const FLIST *,	p2)
		)
	_DCL(RING *,		gbl)
	_DCL(const FLIST *,	p1)
	_DCL(const FLIST *,	p2)
{
	register int	cmp = 0;
	auto	 char	bfr[BUFSIZ];
	auto	 char	*s1, *s2;

	if (gbl->tagsort) {
		if (p1->flag && !p2->flag)
			return (-1);
		if (p2->flag && !p1->flag)
			return (1);
	}

	switch (gbl->sortopt) {
			/* patch: N sort from 'fl' would be nice... */

			/* sort by '.'-separators in name */
	case '.':	cmp = dotcmp(p1->name, p2->name);
			break;

#ifdef	S_IFLNK
			/* sort by link-text */
	case '@':
			if (p1->ltxt != 0 && p2->ltxt != 0)
				cmp = strcmp(p1->ltxt, p2->ltxt);
			else if (p1->ltxt != 0)
				cmp = -1;
			else if (p2->ltxt != 0)
				cmp = 1;
			else
				cmp = 0;
			break;
#endif
			/* sort by file types (suffixes) */
	case 't':	cmp = strcmp(f_type(p1->name), f_type(p2->name));
			break;
	case 'T':	cmp = strcmp(ftype2(p1->name), ftype2(p2->name));
			break;

			/* sort filemodes within the mode-character */
	case 'p':	cmp	= modechar((unsigned)(p1->s.st_mode))
				- modechar((unsigned)(p2->s.st_mode));
			if (!cmp) {
				cmp = CMP(st_mode);
			}
			break;

			/* sort by the various file dates: */
	case 'w':	cmp = CMP(st_mtime);	break;
	case 'r':	cmp = CMP(st_atime);	break;
	case 'c':	cmp = CMP(st_ctime);	break;
#ifdef	Z_RCS_SCCS
	case 'Z':
			cmp = (p1->z_time - p1->s.st_mtime)
			    - (p2->z_time - p2->s.st_mtime);
			break;

	case 'z':	if (!(cmp = CMPX(z_time)))
				cmp = CMP(st_mtime);
			break;
	case 'v':	if (!(s1 = p1->z_vers))	s1 = "";
			if (!(s2 = p2->z_vers))	s2 = "";
			if (*s1 && *s2)
				cmp = -dotcmp(s1, s2);
			else if (*s1)
				cmp = -1;
			else if (*s2)
				cmp = 1;
			break;
	case 'o':	if (p1->z_time && p2->z_time)
				cmp = strcmp(p1->z_lock, p2->z_lock);
			else if (p1->z_time)
				cmp = -1;
			else if (p2->z_time)
				cmp = 1;
			break;
#endif	/* Z_RCS_SCCS */

	case 'S':
	case 's':
			if (isDEV(p1->s.st_mode) && isDEV(p2->s.st_mode))
				cmp = CMP(st_rdev);
			else if (isDEV(p1->s.st_mode))
				cmp = -1;
			else if (isDEV(p2->s.st_mode))
				cmp = 1;
			else
				cmp = (gbl->sortopt == 'S')
					? CMPF(ded_blocks)
					: CMP(st_size);
			break;

	case 'l':	cmp = CMP(st_nlink);	break;
	case 'i':	if (gbl->I_opt == 2) {
				cmp = CMP(st_dev);
				if (cmp == 0)
					cmp = CMP(st_ino);
			} else
				cmp = CMP(st_ino);
			break;
	case 'd':	cmp = p1->dord - p2->dord;	break;
	case 'D':	cmp = CMP(st_dev);		break;

			/* compare uid/gid fields numerically */
	case 'U':	cmp = CMP(st_uid);
			if (cmp == 0 && gbl->G_opt == 2)
				cmp = CMP(st_gid);
			break;
	case 'G':	cmp = CMP(st_gid);
			if (cmp == 0 && gbl->G_opt == 2)
				cmp = CMP(st_uid);
			break;

			/* compare uid/gid fields lexically */
	case 'u':	cmp  = CMP2S(uid2s,st_uid);
			if (cmp == 0 && gbl->G_opt == 2)
				cmp  = CMP2S(gid2s,st_gid);
			break;
	case 'g':	cmp  = CMP2S(gid2s,st_gid);
			if (cmp == 0 && gbl->G_opt == 2)
				cmp  = CMP2S(gid2s,st_uid);
			break;

	case 'N':	(void)strcpy(bfr, pathleaf(p2->name));
			cmp  = strcmp(pathleaf(p1->name), bfr);	break;
	default:	cmp  = strcmp(p1->name, p2->name);
	}
	return (cmp);
}

/*
 * Always returns a reasonable qsort-value for sorting the file-list.  This is
 * used only via 'dedsort()'.
 */
private	RING *	local;	/* so we can hack qsort's interface */
private	QSORT_FUNC(compare)
{
	QSORT_CAST(q1,p1)
	QSORT_CAST(q2,p2)
	int	cmp = dedsort_cmp(local, p1, p2);

	if (!cmp) {
		if (local->sortopt == 'Z')
			cmp = CMP(st_mtime);
		else
			cmp = strcmp(p1->name, p2->name);
	}
	return (local->sortord ? -cmp : cmp);
}

/*
 * perform the requested sort-operation, restoring current-file pointer
 * to point to the original name.
 */
public	void	dedsort (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	if (gbl->numfiles > 1) {
		char	*name = cNAME;
		local = gbl;
		qsort((char *)gbl->flist, (LEN_QSORT)gbl->numfiles, sizeof(FLIST), compare);
		gbl->curfile = findFILE(gbl, name);
	} else
		gbl->curfile = 0;
}
