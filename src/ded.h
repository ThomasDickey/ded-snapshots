#ifdef	MAIN
#ifndef	lint
static	char	*ded_h = "$Id: ded.h,v 5.1 1989/12/08 08:10:40 dickey Exp $";
#endif	/* lint */
#endif

/*
 * Created:	09 Nov 1987
 * Function:	Common definitions for 'ded' (directory editor)
 */

#define		CUR_PTYPES	/* use "curses" */
#define		STR_PTYPES	/* use "strrchr" */
#include	"ptypes.h"
#include	<ctype.h>
#include	<sys/errno.h>
#include	"cmdch.h"
extern	char	*getenv();
extern	int	errno;

extern	char	*gethome(),	/* find home-directory		*/
		*gid2s(),	/* translate gid to string	*/
		*uid2s();	/* translate uid to string	*/

/*
 * Definitions to make linting easier
 */
#define	FREE(p)		dofree(p)

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	BUFSIZ
#endif	/* MAXPATHLEN */

/*
 * SYSTEM5/BSD4.x differences between regular-expression handling:
 */
#ifdef	SYSTEM5
extern	char	*regcmp(),
		*regex();
#define	OLD_REGEX(expr)		if (expr) free(expr)
#define	NEW_REGEX(expr,pattern)	((expr = regcmp(pattern,0)) != 0)
#define	GOT_REGEX(expr,string)	(regex(expr, string, 0) != 0)
#define	BAD_REGEX(expr)		dedmsg("illegal expression")
#else	SYSTEM5
extern	char	*re_comp();	/* returns 0 or error message */
extern	int	re_exec();	/* (return > 0): match */
#define	OLD_REGEX(expr)
#define	NEW_REGEX(expr,pattern)	((expr = re_comp(pattern)) == 0)
#define	GOT_REGEX(expr,string)	(re_exec(string) != 0)
#define	BAD_REGEX(expr)		dedmsg(expr)
#endif	/* SYSTEM5 */

#define	OFF_T	long		/* lint libraries should have 'off_t' */

/*
 * Conditional-compilation variables
 */
#define	Z_SCCS			/* compile-in '-z' sccs support */
#define	Z_RCS			/* compile-in '-z' rcs support */

#ifdef	Z_SCCS
#define	Z_RCS_SCCS		/* compile-in common sccs/rcs support */
#else
#ifdef	Z_RCS
#define	Z_RCS_SCCS
#endif
#endif

/*
 * Miscellaneous definitions
 */
extern	char	*dftenv();
#define	ENV(n)	dftenv(n,"n")

#define	UIDLEN	9		/* length of uid/gid field */

#define	HOUR		(60*60)	/* unix time for one hour */

#define	isDIR(mode)	((mode & S_IFMT) == S_IFDIR)
#define	isDEV(mode)	(	(mode & S_IFMT) == S_IFBLK\
			||	(mode & S_IFMT) == S_IFCHR)
#define	isFILE(mode)	((mode & S_IFMT) == S_IFREG)
#define	isLINK(mode)	((mode & S_IFMT) == S_IFLNK)

#ifndef	MAIN
#define	MAIN	extern
#endif	/* MAIN */

/*
 * We store an array of FLIST structures to describe all files which can
 * be displayed in a given viewport.  This is denoted the 'display list'.
 */
typedef	struct	{
	char		*name;	/* name (within working-directory)	*/
	char		*ltxt;	/* what link resolves to		*/
	struct	stat	s;	/* stat()-block				*/
	short		dord;	/* directory-order, for "d" sort	*/
	char		flag;	/* tag-flag				*/
#ifdef	Z_RCS_SCCS
	char	*z_vers;	/* last sccs-release, version		*/
	char	*z_lock;	/* current locker (user-name)		*/
	time_t	z_time;		/* last sccs delta-date			*/
#endif	/* Z_RCS_SCCS */
	} FLIST;

/*
 * Short-hand expressions:
 */
#define	xNAME(x)	flist[x].name
#define	xSTAT(x)	flist[x].s
#define	xLTXT(x)	flist[x].ltxt
#define	xFLAG(x)	flist[x].flag
#define	xDORD(x)	flist[x].dord

#define	cNAME		xNAME(curfile)
#define	cSTAT		xSTAT(curfile)
#define	cLTXT		xLTXT(curfile)
#define	cFLAG		xFLAG(curfile)

#define	GROUPED(n)	(xFLAG(n) || ((n) == curfile))

			/* markers for column-beginnings */
#define	CCOL_PROT	0
#define	CCOL_UID	1
#define	CCOL_GID	2
#define	CCOL_DATE	3
#define	CCOL_CMD	4
#define	CCOL_NAME	5
			/* total number of columns */
#define	CCOL_MAX	6

/*
 * Global data (cf: dedring.c)
 */
MAIN	char	old_wd[BUFSIZ],	/* original working-directory */
		new_wd[BUFSIZ],	/* current working directory */
		*toscan,	/* selects files in 'dedscan()'		*/
		*scan_expr,	/* compiled version of 'toscan'		*/
		bfr_sh[BUFSIZ];	/* last $SHELL-command			*/

MAIN	FLIST	*flist;		/* pointer to display-list */

MAIN	char	**top_argv;	/* 'argv[]' used in re-scanning, etc. */
MAIN	int	top_argc,
		cmdcol[CCOL_MAX],/* column in which to show cursor */
				/* 0=mode, 1=uid/gid, 2=normal */
		mark_W,		/* row of work-area marker */
		clr_sh,		/* true if we clear-screen after SHELL	*/
		Xbase, Ybase,	/* viewport (for scrolling) */
		curfile,	/* current file on which to operate */
		dateopt,	/* date-option (a,c,m = 0,1,2) */
		sortord,	/* sort-order (TRUE=reverse) */
		sortopt,	/* sort-option (a character) */
		tagsort,	/* sort tagged files apart from others */
#ifdef	S_IFLNK
		AT_opt,		/* show symbolic link target */
#endif	/* S_IFLNK */
		A_opt,		/* show "." and ".." */
		G_opt,		/* show uid/gid field */
		I_opt,		/* show link/inode field */
#ifdef	apollo_sr10
		O_opt,		/* show apollo object-types */
#endif	/* apollo_sr10 */
		P_opt,		/* show filemode in octal vs normal */
		S_opt,		/* show filesize in blocks */
		U_opt;		/* show underlying file-info */
#ifdef	Z_RCS_SCCS
MAIN	int	V_opt,		/* show sccs-versions */
		Y_opt,		/* show sccs-locks */
		Z_opt;		/* show sccs-information */
#endif	/* Z_RCS_SCCS */

MAIN unsigned	numfiles;	/* total files in display-list */
