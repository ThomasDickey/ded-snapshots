/* @(#)ded.h	1.19 88/08/01 11:14:10 */

/*
 * Created:	09 Nov 1987
 * Function:	Common definitions for 'ded' (directory editor)
 */
#include	<curses.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	"cmdch.h"
extern	char	*getenv(),
		*strcat(),
		*strcpy();

extern	char	*doalloc(),	/* (re)allocate memory		*/
		*gethome(),	/* find home-directory		*/
		*gid2s(),	/* translate gid to string	*/
		*uid2s();	/* translate uid to string	*/

/*
 * Definitions to make linting easier
 */
#ifdef	lint
#define	DOALLOC(t,p,n)	(t *)0
#else	lint
#define	DOALLOC(t,p,n)	(t *)doalloc((char *)p,(n) * sizeof(t))
#endif	lint
#define	FREE(p)		dofree(p)

#define	PRINTF	(void)printf
#define	FPRINTF	(void)fprintf
#define	FORMAT	(void)sprintf

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	BUFSIZ
#endif	MAXPATHLEN

/*
 * Main difference between SYSTEM5/BSD4.x is the use of sockets & symbolic links
 */
#ifndef	S_IFSOCK
#define	SYSTEM5
#endif	S_IFSOCK

/*
 * SYSTEM5/BSD4.x differences for opening/scanning directories:
 */
#ifdef	SYSTEM5
#define	DIR	FILE
#define	opendir(n)	fopen(n,"r")
#define	readdir(fp)	(fread(dbfr, sizeof(dbfr), 1, fp) ? &dbfr : (struct direct *)0)
#define	closedir(fp)	fclose(fp)
static	struct	direct	dbfr;
#endif	SYSTEM5

/*
 * Map differences between BSD4.2 and SYSTEM5 runtime libraries:
 */
#ifdef	SYSTEM5
extern	void	free(), qsort();
#define	lstat	stat
#define	getwd(p)	getcwd(p,sizeof(p)-2)
extern	char	*getcwd();
typedef struct screen { char	dummy; };
#else	SYSTEM5
extern		free(), qsort();
extern	char	*getwd();
typedef char	chtype;		/* sys5-curses data-type */
#define	strchr	index
#define	strrchr	rindex
#endif	SYSTEM5

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
#endif	SYSTEM5

/*
 * system: Gould
 */
#ifdef	gould
#define	OFF_T	long
#else	gould
#define	OFF_T	off_t
#endif	gould

/*
 * Conditional-compilation variables
 */
#define	Z_SCCS			/* compile-in '-z' sccs support */
#define	Z_RCS			/* compile-in '-z' rcs support */

#if	defined(Z_SCCS) || defined(Z_RCS)
#define	Z_RCS_SCCS		/* compile-in common sccs/rcs support */
#endif

/*
 * Miscellaneous definitions
 */
#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif	TRUE

#define	EOS	'\0'
#define	ENV(n)	n

#define	UIDLEN	9		/* length of uid/gid field */

#define	HOUR		(60*60)	/* unix time for one hour */

#define	isDIR(mode)	((mode & S_IFMT) == S_IFDIR)
#define	isDEV(mode)	(	(mode & S_IFMT) == S_IFBLK\
			||	(mode & S_IFMT) == S_IFCHR)
#define	isFILE(mode)	((mode & S_IFMT) == S_IFREG)
#define	isLINK(mode)	((mode & S_IFMT) == S_IFLNK)

#ifndef	MAIN
#define	MAIN	extern
#endif	MAIN

/*
 * We store an array of FLIST structures to describe all files which can
 * be displayed in a given viewport.  This is denoted the 'display list'.
 */
typedef	struct	{
	char		*name;	/* name (within working-directory)	*/
	char		*ltxt;	/* what link resolves to		*/
	struct	stat	s;	/* stat()-block				*/
	char		flag;	/* tag-flag				*/
#ifdef	Z_RCS_SCCS
	char	*z_vers;	/* last sccs-release, version		*/
	char	*z_lock;	/* current locker (user-name)		*/
	time_t	z_time;		/* last sccs delta-date			*/
#endif	Z_RCS_SCCS
	} FLIST;

#define	cNAME	flist[curfile].name
#define	cSTAT	flist[curfile].s
#define	cFLAG	flist[curfile].flag

MAIN	char	old_wd[BUFSIZ],	/* original working-directory */
		new_wd[BUFSIZ],	/* current working directory */
		bfr_sh[BUFSIZ];	/* last $SHELL-command			*/

MAIN	FLIST	*flist;		/* pointer to display-list */

MAIN	char	**top_argv;	/* 'argv[]' used in re-scanning, etc. */
MAIN	int	top_argc,
		cmdcol[4],	/* column in which to show cursor */
				/* 0=mode, 1=uid/gid, 2=normal */
		mark_W,		/* row of work-area marker */
		clr_sh,		/* true if we clear-screen after SHELL	*/
		Xbase, Ybase,	/* viewport (for scrolling) */
		curfile,	/* current file on which to operate */
		dateopt,	/* date-option (a,c,m = 0,1,2) */
		sortord,	/* sort-order (TRUE=reverse) */
		sortopt,	/* sort-option (a character) */
		tagsort,	/* sort tagged files apart from others */
#ifndef	SYSTEM5
		AT_opt,		/* show symbolic link target */
#endif	SYSTEM5
		G_opt,		/* show uid/gid field */
		I_opt,		/* show link/inode field */
		P_opt,		/* show filemode in octal vs normal */
		S_opt,		/* show filesize in blocks */
		U_opt;		/* show underlying file-info */
#ifdef	Z_RCS_SCCS
MAIN	int	V_opt,		/* show sccs-versions */
		Y_opt,		/* show sccs-locks */
		Z_opt;		/* show sccs-information */
#endif	Z_RCS_SCCS

MAIN unsigned	numfiles;	/* total files in display-list */
