/*
 * Title:	dedmake.c (make entry for ded)
 * Author:	T.E.Dickey
 * Created:	12 Sep 1988
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		01 Mar 1998, mods to build on OS/2 EMX 0.9b
 *		15 Feb 1998, compiler-warnings
 *		29 Oct 1993, ifdef-ident
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		16 Jan 1992, force 'dedline()' off on exit.
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

MODULE_ID("$Id: dedmake.c,v 12.12 2004/03/07 23:25:18 tom Exp $")

static int
makeit(RING * gbl, char *name, int mode, int hard)
{
    int fid;

    errno = 0;
    if (mode == S_IFDIR) {
	if (mkdir(name, 0777) < 0)
	    return (FALSE);
	ft_insert(name);
    }
    if (mode == S_IFREG) {
#if defined(HAVE_LINK)
	if (hard >= 0) {
	    if (link(gNAME(hard), name) < 0)
		return (FALSE);
	} else
#endif
	{
	    if ((fid = creat(name, 0777)) < 0)
		return (FALSE);
	    (void) close(fid);
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
	unsigned j;
	for_each_file(gbl, j) {
	    if (j == gbl->curfile
		|| gSTAT(j).st_ino == gSTAT(hard).st_ino) {
		statLINE(gbl, j);
		showLINE(gbl, j);
	    }
	}
    } else {
	statLINE(gbl, gbl->curfile);
	showLINE(gbl, gbl->curfile);
    }
    return (TRUE);
}

static int
made_or_quit(RING * gbl, int firstc, int mode, int hard, char *to_edit)
{
    Stat_t sb;
    int ok;

    ok = edittext(gbl, firstc, gbl->cmdcol[CCOL_NAME], MAXPATHLEN, to_edit);

    clearmsg();

    if (ok) {
	errno = 0;
	if (stat(to_edit, &sb) >= 0)
	    errno = EEXIST;
	else if (errno == ENOENT) {
	    if (!makeit(gbl, to_edit, mode, hard)) {
		waitmsg(strerror(errno));
		statMAKE(gbl, 0);	/* undo it -- error */
	    }
	    showC(gbl);
	    return TRUE;
	}
	dedmsg(gbl, strerror(errno));
	(void) edit_inline(-1);	/* force refresh! */
	(void) ReplayTrim();
	return FALSE;
    }
    statMAKE(gbl, 0);		/* undo it -- gave up */
    return TRUE;
}

void
dedmake(RING * gbl, int firstc)
{
    int mode;
    int hard = -1;
    char bfr[MAXPATHLEN];

    /* make a dummy entry */
    switch (firstc) {
    case 'd':
	mode = S_IFDIR;
	break;
    case 'f':
	mode = S_IFREG;
	break;
#ifdef	S_IFLNK
    case 'l':
	mode = S_IFLNK;
	break;
#endif
    case 'L':
	if ((mode = (cSTAT.st_mode & S_IFMT)) == S_IFREG) {
	    hard = gbl->curfile + 1;
	    break;
	}
    default:
	if (isascii(firstc) && isgraph(firstc)) {
	    FORMAT(bfr, "create-%c not defined", firstc);
	    dedmsg(gbl, bfr);
	} else
	    beep();
	return;
    }
    statMAKE(gbl, mode);

    /* loop until either the user gives up, or supplies a legal name */
    do {
	(void) strcpy(bfr, (hard >= 0) ? gNAME(hard) : cNAME);
    } while (!made_or_quit(gbl, firstc, mode, hard, bfr));
    (void) edit_inline(FALSE);
}
