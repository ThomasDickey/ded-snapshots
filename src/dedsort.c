#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)dedsort.c	1.3 88/05/09 09:13:47";
#endif	NO_SCCS_ID

/*
 * Title:	dedsort.c (ded-sort)
 * Author:	T.E.Dickey
 * Created:	11 Nov 1987
 * Modified:
 *		09 May 1988, make devices sort-size separately from files.
 *
 * Function:	Perform display-list sorting for DED directory editor.
 */
#include	"ded.h"

static
char *
ftype(name)			/* file type: after first '.' in name */
char	*name;
{
extern	char	*strchr(), *strrchr();
register char *s = strrchr(name, '/'),
	*t;
	if (s)	s++;
	else	s = name;
	if (t = strchr(s, '.'))		t++;
	else				t = name+strlen(name);
	return(t);
}

static
char *
fTYPE(name)			/* file type: after last '.' in name */
char	*name;
{
extern	char	*strchr(), *strrchr();
register char *s = strrchr(name, '/'),
	*t;
	if (s)	s++;
	else	s = name;
	if (t = strrchr(s, '.'))	t++;
	else				t = name+strlen(name);
	return(t);
}

#define	CHECKED(p)	(p->z_time == p->s.st_mtime)
#define	CMPX(m)		(p1->m > p2->m ? -1 : (p1->m < p2->m ? 1 : 0))
#define	CMP(m)		CMPX(s.m)

static
compare(p1, p2)
FLIST	*p1, *p2;
{
register int cmp = 0;
char	bfr[BUFSIZ];

	switch (sortopt) {
			/* patch: N,X,x sort from 'fl' would be nice... */

			/* sort by file types (suffixes) */
	case 't':	cmp = strcmp(ftype(p1->name), ftype(p2->name));
			break;
	case 'T':	cmp = strcmp(fTYPE(p1->name), fTYPE(p2->name));
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
#ifdef	Z_SCCS
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
	case 'v':	if (!(cmp = CMPX(z_rels)))
				cmp = CMPX(z_vers);
			break;
#endif	Z_SCCS

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

			/* compare uid/gid fields numerically */
	case 'U':	cmp = CMP(st_uid);	break;
	case 'G':	cmp = CMP(st_gid);	break;

			/* compare uid/gid fields lexically */
	case 'u':	(void) strcpy(bfr, uid2s(p1->s.st_uid));
			cmp  = strcmp(bfr, uid2s(p2->s.st_uid));
			break;
	case 'g':	(void) strcpy(bfr, gid2s(p1->s.st_gid));
			cmp  = strcmp(bfr, gid2s(p2->s.st_gid));
			break;
	}
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
char	*name = flist[curfile].name;
	qsort(flist, numfiles, sizeof(FLIST), compare);
	for (curfile = 0; curfile < numfiles; curfile++)
		if (!strcmp(flist[curfile].name, name))
			return;
	curfile = 0;
}
