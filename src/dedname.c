#ifndef	lint
static	char	Id[] = "$Id: dedname.c,v 8.0 1990/05/23 09:08:25 ste_cm Rel $";
#endif	lint

/*
 * Title:	dedname.c (ded rename)
 * Author:	T.E.Dickey
 * Created:	11 May 1988
 * $Log: dedname.c,v $
 * Revision 8.0  1990/05/23 09:08:25  ste_cm
 * BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *
 *		Revision 7.1  90/05/23  09:08:25  dickey
 *		modified interface to 'dedring()'
 *		
 *		Revision 7.0  89/07/25  10:50:39  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  89/07/25  10:50:39  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.0  89/07/25  10:50:39  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.0  89/07/25  10:50:39  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.1  89/07/25  10:50:39  dickey
 *		renamed 'fullname()' to 'expand_name()' to avoid conflict with curses-function
 *		when recompiling this under apollo SR10.1
 *		
 *		Revision 3.0  89/03/14  10:03:08  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/14  10:03:08  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.5  89/03/14  10:03:08  dickey
 *		sccs2rcs keywords
 *		
 *		12 Sep 1988, use 'pathcat()'
 *
 * Function:	Rename a file in the file-list, then perform cleanup of the
 *		directory tree and display-ring.
 *
 * patch:	If a directory is renamed, ftree will retain only the top-level
 *		node -- the tree will be lost.
 *
 *		If a directory in the display-ring is renamed, it will be
 *		removed -- should provide renaming in both dedring and ftree.
 */

#include	"ded.h"
extern	char	*pathcat();
extern	char	*txtalloc();

static
expand_name(leaf)
char	*leaf;
{
	char	path[BUFSIZ];
	(void)strcpy(leaf, pathcat(path, new_wd, leaf));
}

static
findname(path)
char	*path;
{
char	tmp[BUFSIZ];
	do {
		(void)dedring(strcpy(tmp, new_wd), 'F', 1, FALSE, (char *)0);
	} while (strcmp(new_wd, path));
}

dedname(x, newname)
char	*newname;
{
int	ok	= FALSE;
char	oldname[BUFSIZ],
	bfr[BUFSIZ];

	if (strcmp(strcpy(oldname, xNAME(x)), newname)) {
		dlog_comment("rename \"%s\" (name=%s)\n", newname, xNAME(x));
#ifdef	SYSTEM5
		if (isFILE(xSTAT(x).st_mode)) {
			if (link(oldname, newname) < 0) {
				warn(newname);
				return (-1);
			}
			if (unlink(oldname) < 0) {
				warn(oldname);
				return (-1);
			}
			ok = TRUE;
		} else {
			FORMAT(bfr, "cannot rename \"%s\"", xNAME(x));
			dedmsg(bfr);
			return (-1);
		}
		/* patch: should do 'system()' to rename directory, etc. */
#else	SYSTEM5
		if (rename(oldname, newname) < 0) {
			warn(newname);
			return (-1);
		}
		ok = TRUE;
#endif	SYSTEM5
		/*
		 * If we renamed a directory, update ftree.
		 */
		if (isDIR(xSTAT(x).st_mode)) {
			expand_name(oldname);	/* ...for dedring */

			/* Rename it in the directory-tree */
			ft_rename(oldname, newname);

			/* Was it in our display list? */
			if (dedrang(oldname)) {
			char	tmp[BUFSIZ];
				(void)strcpy(bfr, new_wd);
				findname(oldname);
				(void)dedring(tmp, 'Q', 1, FALSE, (char *)0);
				findname(bfr);
			}
		}
	}

	if (ok) {
		txtfree(xNAME(x));
		xNAME(x) = txtalloc(newname);
	}
	return (0);
}
