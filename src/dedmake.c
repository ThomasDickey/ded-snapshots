#ifndef	lint
static	char	sccs_id[] = "@(#)dedmake.c	1.4 89/03/14 10:33:55";
#endif	lint

/*
 * Title:	dedmake.c (make entry for ded)
 * Author:	T.E.Dickey
 * Created:	12 Sep 1988
 * Modified:
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
makeit(name, mode)
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
		if ((fid = creat(name, 0777)) < 0)
			return (FALSE);
		(void)close(fid);
	}
#ifdef	S_IFLNK
	if (mode == S_IFLNK) {
		if (symlink(cLTXT = txtalloc("."), name) < 0)
			return (FALSE);
	}
#endif	S_IFLNK
	cNAME = txtalloc(name);
	statLINE(curfile);
	showLINE(curfile);
	return (TRUE);
}

dedmake()
{
	auto	struct	stat	sb;
	auto	int	mode;
	auto	char	bfr[BUFSIZ];

	/* make a dummy entry */
	switch (dlog_char((int *)0,1)) {
	case 'd':	mode = S_IFDIR;	break;
	case 'f':	mode = S_IFREG;	break;
#ifdef	S_IFLNK
	case 'l':	mode = S_IFLNK;	break;
#endif	S_IFLNK
	default:	beep();
			return;
	}
	statMAKE(mode);

	/* loop until either the user gives up, or supplies a legal name */
	(void)strcpy(bfr, cNAME);
	while (edittext('q', cmdcol[3], sizeof(bfr), bfr)) {
		errno = 0;
		if (stat(bfr, &sb) >= 0)
			errno = EEXIST;
		else if (errno == ENOENT) {
			if (!makeit(bfr, mode)) {
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
