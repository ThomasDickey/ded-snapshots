/*
 * Title:	dedname.c (ded rename)
 * Author:	T.E.Dickey
 * Created:	11 May 1988
 * Modified:
 *		01 Nov 1993, Sys5.4 can rename directories.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		17 Feb 1992, use 'ring_rename()' to simplify list-renaming.
 *		18 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		23 May 1990, modified interface to 'dedring()'
 *		12 Sep 1988, use 'pathcat()'
 *
 * Function:	Rename a file in the file-list, then perform cleanup of the
 *		directory tree and display-ring.
 */

#include	"ded.h"

MODULE_ID("$Id: dedname.c,v 12.7 1995/09/03 19:42:14 tom Exp $")

int	dedname(
	_ARX(RING *,	gbl)
	_ARX(int,	x)
	_AR1(char *,	newname)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	x)
	_DCL(char *,	newname)
{
	int	ok	= FALSE;
	char	oldname[MAXPATHLEN];

	if (strcmp(strcpy(oldname, gNAME(x)), newname)) {
		dlog_comment("rename \"%s\" (name=%s)\n", newname, gNAME(x));
#if HAVE_RENAME
		if (rename(oldname, newname) < 0) {
			warn(gbl, newname);
			return (-1);
		}
		ok = TRUE;
#else
		if (isFILE(gSTAT(x).st_mode)) {
			if (link(oldname, newname) < 0) {
				warn(gbl, newname);
				return (-1);
			}
			if (unlink(oldname) < 0) {
				warn(gbl, oldname);
				return (-1);
			}
			ok = TRUE;
		} else {
			char	bfr[MAXPATHLEN];
			FORMAT(bfr, "cannot rename \"%s\"", gNAME(x));
			dedmsg(gbl, bfr);
			return (-1);
		}
		/* patch: should do 'system()' to rename directory, etc. */
#endif	/* HAVE_RENAME */
		/*
		 * If we renamed a directory, update ftree.
		 */
		if (isDIR(gSTAT(x).st_mode)) {
			ring_rename(gbl, oldname, newname);
			ft_rename(oldname, newname);
		}
	}

	if (ok) {
		txtfree(gNAME(x));
		gNAME(x) = txtalloc(newname);
	}
	return (0);
}
