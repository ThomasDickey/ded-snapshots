#ifndef	lint
static	char	Id[] = "$Id: dedname.c,v 10.0 1991/10/18 08:41:40 ste_cm Rel $";
#endif

/*
 * Title:	dedname.c (ded rename)
 * Author:	T.E.Dickey
 * Created:	11 May 1988
 * Modified:
 *		18 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		23 May 1990, modified interface to 'dedring()'
 *		25 Jul 1989, renamed 'fullname()' to 'expand_name()' to avoid
 *			     conflict with curses-function when recompiling this
 *			     under apollo SR10.1
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

static
expand_name _ONE(char *,leaf)
{
	char	path[BUFSIZ];
	(void)strcpy(leaf, pathcat(path, new_wd, leaf));
}

static
findname _ONE(char *,path)
{
char	tmp[BUFSIZ];
	do {
		(void)dedring(strcpy(tmp, new_wd), 'F', 1, FALSE, (char *)0);
	} while (strcmp(new_wd, path));
}

dedname(
_ARX(int,	x)
_AR1(char *,	newname)
	)
_DCL(int,	x)
_DCL(char *,	newname)
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
#else	/* !SYSTEM5 */
		if (rename(oldname, newname) < 0) {
			warn(newname);
			return (-1);
		}
		ok = TRUE;
#endif	/* SYSTEM5/!SYSTEM5 */
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
