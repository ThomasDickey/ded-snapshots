#ifndef	lint
static	char	Id[] = "$Id: dedsort.c,v 9.0 1991/05/16 07:43:46 ste_cm Rel $";
#endif

/*
 * Title:	dedsort.c (ded-sort)
 * Author:	T.E.Dickey
 * Created:	11 Nov 1987
 * $Log: dedsort.c,v $
 * Revision 9.0  1991/05/16 07:43:46  ste_cm
 * BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *
 *		Revision 8.1  91/05/16  07:43:46  dickey
 *		apollo sr10.3 cpp complains about tag on #endif
 *		
 *		Revision 8.0  90/01/22  15:04:29  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  90/01/22  15:04:29  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/01/22  15:04:29  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.1  90/01/22  15:04:29  dickey
 *		corrections to 'v'-sort
 *		
 *		Revision 5.0  89/10/12  15:47:10  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.4  89/10/12  15:47:10  dickey
 *		refined inode-, uid-, gid-sorts so that if I_opt or G_opt are
 *		in two-column mode, we sort what the user sees.
 *		
 *		Revision 4.3  89/10/11  16:29:49  dickey
 *		added apollo-only fix for t-sort for DSEE-directory names
 *		(ending with "$.*.$").
 *		
 *		Revision 4.2  89/10/06  11:40:05  dickey
 *		modified 't' sort so that names beginning with '.' are
 *		sorted in a more natural manner
 *		
 *		Revision 4.1  89/10/04  16:48:36  dickey
 *		added o,O sorts
 *		
 *		Revision 4.0  89/01/23  09:57:37  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/01/23  09:57:37  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/01/23  09:57:37  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.16  89/01/23  09:57:37  dickey
 *		sccs2rcs keywords
 *		
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
#include	"ded.h"
extern	char	*ftype();
extern	char	*ftype2();
extern	char	*pathleaf();

#ifdef	apollo_sr10
extern	char	*type_uid2s();
#endif

#define	CHECKED(p)	(p->z_time == p->s.st_mtime)
#define	CMPX(m)		(p1->m > p2->m ? -1 : (p1->m < p2->m ? 1 : 0))
#define	CMP(m)		CMPX(s.m)
#define	CMP2S(f,m)	cmp = strcmp(strcpy(bfr, f((int)p1->s.m)),\
						 f((int)p2->s.m))

/* sort types so that names beginning with '.' are treated specially */
static
char	*
f_type(s)
char	*s;
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
dedsort_cmp(p1, p2)
FLIST	*p1, *p2;
{
	register int	cmp = 0;
	auto	 char	bfr[BUFSIZ];
	auto	 char	*s1, *s2;

	if (tagsort) {
		if (p1->flag && !p2->flag)
			return (-1);
		if (p2->flag && !p1->flag)
			return (1);
	}

	switch (sortopt) {
			/* patch: N sort from 'fl' would be nice... */

#ifdef	apollo_sr10
			/* sort by object-types (numeric) */
	case 'o':	cmp = strcmp(type_uid2s(&p1->s), type_uid2s(&p2->s));
			break;

	case 'O':	if (!(cmp = p1->s.st_rfu4[0] - p2->s.st_rfu4[0]))
				cmp = p1->s.st_rfu4[1] - p2->s.st_rfu4[1];
			break;
#endif
			/* sort by '.'-separators in name */
	case '.':	cmp = dotcmp(p1->name, p2->name);
			break;

			/* sort by file types (suffixes) */
	case 't':	cmp = strcmp(f_type(p1->name), f_type(p2->name));
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
	case 'v':	if (!(s1 = p1->z_vers))	s1 = "";
			if (!(s2 = p2->z_vers))	s2 = "";
			if (*s1 && *s2)
				cmp = -dotcmp(s1, s2);
			else if (*s1)
				cmp = -1;
			else if (*s2)
				cmp = 1;
			break;
	case 'y':	if (p1->z_time && p2->z_time)
				cmp = strcmp(p1->z_lock, p2->z_lock);
			else if (p1->z_time)
				cmp = -1;
			else if (p2->z_time)
				cmp = 1;
			break;
#endif	/* Z_RCS_SCCS */

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
	case 'i':	if (I_opt == 2) {
				cmp = CMP(st_dev);
				if (cmp == 0)
					cmp = CMP(st_ino);
			} else
				cmp = CMP(st_ino);
			break;
	case 'd':	cmp = p1->dord - p2->dord;	break;

			/* compare uid/gid fields numerically */
	case 'U':	cmp = CMP(st_uid);
			if (cmp == 0 && G_opt == 2)
				cmp = CMP(st_gid);
			break;
	case 'G':	cmp = CMP(st_gid);
			if (cmp == 0 && G_opt == 2)
				cmp = CMP(st_uid);
			break;

			/* compare uid/gid fields lexically */
	case 'u':	cmp  = CMP2S(uid2s,st_uid);
			if (cmp == 0 && G_opt == 2)
				cmp  = CMP2S(gid2s,st_gid);
			break;
	case 'g':	cmp  = CMP2S(gid2s,st_gid);
			if (cmp == 0 && G_opt == 2)
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
