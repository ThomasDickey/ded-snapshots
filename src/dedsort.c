#ifndef	lint
static	char	sccs_id[] = "@(#)dedsort.c	1.14 89/01/18 10:27:28";
#endif	lint

/*
 * Title:	dedsort.c (ded-sort)
 * Author:	T.E.Dickey
 * Created:	11 Nov 1987
 * Modified:
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
#include	"ded.h"
extern	char	*ftype();
extern	char	*ftype2();

#define	CHECKED(p)	(p->z_time == p->s.st_mtime)
#define	CMPX(m)		(p1->m > p2->m ? -1 : (p1->m < p2->m ? 1 : 0))
#define	CMP(m)		CMPX(s.m)
#define	CMP2S(f,m)	cmp = strcmp(strcpy(bfr, f((int)p1->s.m)),\
						 f((int)p2->s.m))

/*
 * Compare the specified file-list entries, returning 0 iff their sort-key is
 * equivalent, +/- according to the direction of the inequality.
 */
dedsort_cmp(p1, p2)
FLIST	*p1, *p2;
{
register int cmp = 0;
char	bfr[BUFSIZ];

	if (tagsort) {
		if (p1->flag && !p2->flag)
			return (-1);
		if (p2->flag && !p1->flag)
			return (1);
	}

	switch (sortopt) {
			/* patch: N sort from 'fl' would be nice... */

			/* sort by '.'-separators in name */
	case '.':	cmp = dotcmp(p1->name, p2->name);
			break;

			/* sort by file types (suffixes) */
	case 't':	cmp = strcmp(ftype(p1->name), ftype(p2->name));
			break;
	case 'T':	cmp = strcmp(ftype2(p1->name), ftype2(p2->name));
			break;

			/* sort filemodes within the mode-character */
	case 'p':	cmp	= modechar(p1->s.st_mode)
				- modechar(p2->s.st_mode);
			if (!cmp)
				cmp = CMP(st_mode);
			break;

			/* sort by the various file dates: */
	case 'w':	cmp = CMP(st_mtime);	break;
	case 'r':	cmp = CMP(st_atime);	break;
	case 'c':	cmp = CMP(st_ctime);	break;
#ifdef	Z_RCS_SCCS
	case 'Z':
	case 'z':	if (!(cmp = CMPX(z_time)))
				cmp = CMP(st_mtime);
			if (sortopt == 'Z') {
				if (CHECKED(p1) && !CHECKED(p2))
					cmp = 1;
				else if (!CHECKED(p1) && CHECKED(p2))
					cmp = -1;
			}
			break;
	case 'v':	if (p1->z_time && p2->z_time)
				cmp = -dotcmp(p1->z_vers, p2->z_vers);
			else if (p1->z_time)
				cmp = -1;
			else if (p2->z_time)
				cmp = 1;
			break;
	case 'y':	if (p1->z_time && p2->z_time)
				cmp = strcmp(p1->z_lock, p2->z_lock);
			else if (p1->z_time)
				cmp = -1;
			else if (p2->z_time)
				cmp = 1;
			break;
#endif	Z_RCS_SCCS

	case 's':
			if (isDEV(p1->s.st_mode) && isDEV(p2->s.st_mode))
				cmp = CMP(st_rdev);
			else if (isDEV(p1->s.st_mode))
				cmp = -1;
			else if (isDEV(p2->s.st_mode))
				cmp = 1;
			else
				cmp = CMP(st_size);
			break;

	case 'l':	cmp = CMP(st_nlink);	break;
	case 'i':	cmp = CMP(st_ino);	break;
	case 'd':	cmp = p1->dord - p2->dord;	break;

			/* compare uid/gid fields numerically */
	case 'U':	cmp = CMP(st_uid);	break;
	case 'G':	cmp = CMP(st_gid);	break;

			/* compare uid/gid fields lexically */
	case 'u':	cmp  = CMP2S(uid2s,st_uid);	break;
	case 'g':	cmp  = CMP2S(gid2s,st_gid);	break;
	default:	cmp  = strcmp(p1->name, p2->name);
	}
	return (cmp);
}

/*
 * Always returns a reasonable qsort-value for sorting the file-list.  This is
 * used only via 'dedsort()'.
 */
static
compare(p1, p2)
FLIST	*p1, *p2;
{
	int	cmp = dedsort_cmp(p1,p2);
	if (!cmp)
		cmp = strcmp(p1->name, p2->name);
	return(sortord ? -cmp : cmp);
}

/*
 * perform the requested sort-operation, restoring current-file pointer
 * to point to the original name.
 */
dedsort()
{
	char	*name = cNAME;
	qsort((char *)flist, (LEN_QSORT)numfiles, sizeof(FLIST), compare);
	curfile = findFILE(name);
}
