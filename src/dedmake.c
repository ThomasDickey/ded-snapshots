#ifndef	lint
static	char	Id[] = "$Id: dedmake.c,v 9.2 1991/10/16 12:44:18 dickey Exp $";
#endif

/*
 * Title:	dedmake.c (make entry for ded)
 * Author:	T.E.Dickey
 * Created:	12 Sep 1988
 * Modified:
 *		16 Oct 1991, modified to allow command-replay
 *		15 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		18 Apr 1991, added command "cL" to create hard link (implicitly
 *			     to the current entry).
 *		05 Oct 1989, modified treatment of 'cmdcol[]' (cf: showFILES)
 *		14 Mar 1989, interface to 'dlog'
 *		28 Feb 1989, invoke 'ft_insert()' for new directories
 *
 * Function:	Create a new directory/file/link
 */

#include	"ded.h"

static
makeit(
_ARX(char *,	name)
_ARX(int,	mode)
_AR1(int,	hard)
	)
_DCL(char *,	name)
_DCL(int,	mode)
_DCL(int,	hard)
{
	int	fid;

	errno = 0;
	if (mode == S_IFDIR) {
		if (mkdir(name, 0777) < 0)
			return (FALSE);
		ft_insert(name);
	}
	if (mode == S_IFREG) {
		if (hard >= 0) {
			if (link(xNAME(hard), name) < 0)
				return (FALSE);
		} else {
			if ((fid = creat(name, 0777)) < 0)
				return (FALSE);
			(void)close(fid);
		}
	}
#ifdef	S_IFLNK
	if (mode == S_IFLNK) {
		if (symlink(cLTXT = txtalloc("."), name) < 0)
			return (FALSE);
	}
#endif
	cNAME = txtalloc(name);
	if (hard >= 0) {
		register int j;
		for (j = 0; j < numfiles; j++) {
			if (j == curfile
			 || xSTAT(j).st_ino == xSTAT(hard).st_ino) {
				statLINE(j);
				showLINE(j);
			}
		}
	} else {
		statLINE(curfile);
		showLINE(curfile);
	}
	return (TRUE);
}

static
made_or_quit(
_ARX(int,	firstc)
_ARX(int,	mode)
_ARX(int,	hard)
_AR1(char *,	to_edit)
	)
_DCL(int,	firstc)
_DCL(int,	mode)
_DCL(int,	hard)
_DCL(char *,	to_edit)
{
	auto	struct	stat	sb;
	auto	int	ok =
			edittext(firstc, cmdcol[CCOL_NAME], BUFSIZ, to_edit);

	clearmsg();

	if (ok) {
		errno = 0;
		if (stat(to_edit, &sb) >= 0)
			errno = EEXIST;
		else if (errno == ENOENT) {
			if (!makeit(to_edit, mode, hard)) {
				waitmsg(sys_errlist[errno]);
				statMAKE(0);	/* undo it -- error */
			}
			showC();
			return TRUE;
		}
		dedmsg(sys_errlist[errno]);
		dedline(-TRUE);			/* force refresh! */
		replay(-1);
		return FALSE;
	}
	statMAKE(0);				/* undo it -- gave up */
	return TRUE;
}

dedmake _ONE(int,firstc)
{
	auto	int	mode;
	auto	int	hard	= -1;
	auto	char	bfr[BUFSIZ];

	/* make a dummy entry */
	switch (firstc) {
	case 'd':	mode = S_IFDIR;	break;
	case 'f':	mode = S_IFREG;	break;
#ifdef	S_IFLNK
	case 'l':	mode = S_IFLNK;	break;
#endif
	case 'L':	if ((mode = (cSTAT.st_mode & S_IFMT)) == S_IFREG) {
				hard = curfile + 1;
				break;
			}
	default:	if (isascii(firstc) && isgraph(firstc)) {
				FORMAT(bfr, "create-%c not defined", firstc);
				dedmsg(bfr);
			} else
				beep();
			return;
	}
	statMAKE(mode);

	/* loop until either the user gives up, or supplies a legal name */
	do {
		(void)strcpy(bfr, (hard >= 0) ? xNAME(hard) : cNAME);
	} while (!made_or_quit(firstc, mode, hard, bfr));
}
