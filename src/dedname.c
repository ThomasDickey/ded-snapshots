#ifndef	lint
static	char	sccs_id[] = "@(#)dedname.c	1.2 88/08/02 12:41:25";
#endif	lint

/*
 * Title:	dedname.c (ded rename)
 * Author:	T.E.Dickey
 * Created:	11 May 1988
 * Modified:
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
extern	char	*txtalloc();

static
fullname(leaf)
char	*leaf;
{
char	path[BUFSIZ];
int	len	= strlen(strcpy(path, new_wd));
	if (path[len-1] != '/')
		(void)strcat(path, "/");
	(void)strcat(path, leaf);
	(void)strcpy(leaf, path);
}

static
findname(path)
char	*path;
{
char	tmp[BUFSIZ];
	do {
		(void)dedring(strcpy(tmp, new_wd), 'F', 1);
	} while (strcmp(new_wd, path));
}

dedname(x, newname)
char	*newname;
{
int	ok	= FALSE;
char	oldname[BUFSIZ],
	bfr[BUFSIZ];

	if (strcmp(strcpy(oldname, xNAME(x)), newname)) {
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
			fullname(oldname);	/* ...for dedring */

			/* Rename it in the directory-tree */
			ft_rename(oldname, newname);

			/* Was it in our display list? */
			if (dedrang(oldname)) {
			char	tmp[BUFSIZ];
				(void)strcpy(bfr, new_wd);
				findname(oldname);
				(void)dedring(tmp, 'Q', 1);
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
