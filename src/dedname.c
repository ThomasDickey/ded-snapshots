#ifndef	lint
static	char	Id[] = "$Id: dedname.c,v 10.1 1992/02/17 13:56:54 dickey Exp $";
#endif

/*
 * Title:	dedname.c (ded rename)
 * Author:	T.E.Dickey
 * Created:	11 May 1988
 * Modified:
 *		17 Feb 1992, use 'dedrering()' to simplify list-renaming.
 *		18 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		23 May 1990, modified interface to 'dedring()'
 *		12 Sep 1988, use 'pathcat()'
 *
 * Function:	Rename a file in the file-list, then perform cleanup of the
 *		directory tree and display-ring.
 */

#include	"ded.h"

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
			dedrering(oldname, newname);
			ft_rename(oldname, newname);
		}
	}

	if (ok) {
		txtfree(xNAME(x));
		xNAME(x) = txtalloc(newname);
	}
	return (0);
}
