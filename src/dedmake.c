#ifndef	lint
static	char	Id[] = "$Id: dedmake.c,v 8.1 1991/04/18 08:58:58 dickey Exp $";
#endif	lint

/*
 * Title:	dedmake.c (make entry for ded)
 * Author:	T.E.Dickey
 * Created:	12 Sep 1988
 * $Log: dedmake.c,v $
 * Revision 8.1  1991/04/18 08:58:58  dickey
 * added command "cL" to create hard link (implicitly to the
 * current entry).
 *
 *		Revision 8.0  89/10/05  17:02:20  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  89/10/05  17:02:20  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  89/10/05  17:02:20  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.0  89/10/05  17:02:20  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.1  89/10/05  17:02:20  dickey
 *		modified treatment of 'cmdcol[]' (cf: showFILES)
 *		
 *		Revision 4.0  89/03/15  08:41:59  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/03/15  08:41:59  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/15  08:41:59  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.6  89/03/15  08:41:59  dickey
 *		sccs2rcs keywords
 *		
 *		14 Mar 1989, interface to 'dlog'
 *		28 Feb 1989, invoke 'ft_insert()' for new directories
 *
 * Function:	Create a new directory/file/link
 */

#include	"ded.h"
#include	<sys/errno.h>
extern	char	*txtalloc();
extern	int	errno;
extern	char	*sys_errlist[];

static
makeit(name, mode, hard)
char	*name;
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
#endif	S_IFLNK
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

dedmake()
{
	auto	struct	stat	sb;
	auto	int	mode;
	auto	int	hard	= -1;
	auto	char	bfr[BUFSIZ];

	/* make a dummy entry */
	switch (dlog_char((int *)0,0)) {
	case 'd':	mode = S_IFDIR;	break;
	case 'f':	mode = S_IFREG;	break;
#ifdef	S_IFLNK
	case 'l':	mode = S_IFLNK;	break;
#endif	S_IFLNK
	case 'L':	if ((mode = (cSTAT.st_mode & S_IFMT)) == S_IFREG) {
				hard = curfile + 1;
				break;
			}
	default:	beep();
			return;
	}
	statMAKE(mode);

	/* loop until either the user gives up, or supplies a legal name */
	(void)strcpy(bfr, (hard >= 0) ? xNAME(hard) : cNAME);
	while (edittext('q', cmdcol[CCOL_NAME], sizeof(bfr), bfr)) {
		errno = 0;
		if (stat(bfr, &sb) >= 0)
			errno = EEXIST;
		else if (errno == ENOENT) {
			if (!makeit(bfr, mode, hard)) {
				waitmsg(sys_errlist[errno]);
				statMAKE(0);	/* undo it -- error */
			}
			showC();
			return;
		}
		waitmsg(sys_errlist[errno]);
	}
	statMAKE(0);				/* undo it -- gave up */
}
