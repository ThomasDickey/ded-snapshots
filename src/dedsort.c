#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)dedsort.c	1.1 87/11/20 08:13:40";
#endif	NO_SCCS_ID

/*
 * Function:	Perform sorting for DED directory editor.
 */
#include	"ded.h"
extern	void	qsort();

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

#define	CMP(m)	(p1->s.m > p2->s.m ? -1 : (p1->s.m < p2->s.m ? 1 : 0))

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

	case 's':	cmp = CMP(st_size);	break;
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
