/* @(#)ded.h	1.4 88/03/24 13:16:07 */

/*
 * Created:	09 Nov 1987
 * Function:	Common definitions for 'ded' (directory editor)
 */
#include	<curses.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
extern	char	*getcwd(),
		*getenv(),
		*strcat(),
		*strcpy();

extern	char	*doalloc(),	/* (re)allocate memory		*/
		*gid2s(),	/* translate gid to string	*/
		*uid2s();	/* translate uid to string	*/

#ifndef	S_IFSOCK
#define	SYSTEM5
#endif	S_IFSOCK

#ifdef	SYSTEM5
#define	DIR	FILE
#define	opendir(n)	fopen(n,"r")
#define	readdir(fp)	(fread(dbfr, sizeof(dbfr), 1, fp) ? &dbfr : (struct direct *)0)
#define	closedir(fp)	fclose(fp)
static	struct	direct	dbfr;
#endif	SYSTEM5

/*
 * Map differences between BSD4.2 and SYSTEM5
 */
#ifdef	SYSTEM5
extern	void	free(), qsort();
#define	lstat	stat
#define	utimes	utime
typedef struct screen { char	dummy; };
#else	SYSTEM5
extern		free(), qsort();
typedef char	chtype;		/* sys5-curses data-type */
#define	strchr	index
#define	strrchr	rindex
#endif	SYSTEM5

/*
 * Conditional-compilation variables
 */
#define	Z_SCCS			/* compile-in '-z' sccs support */

/*
 * Miscellaneous definitions
 */
#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif	TRUE

#define	EOS	'\0'
#define	ENV(n)	n
#define	CTL(c)	('c' & 037)

#define	ARO_UP		CTL(u)
#define	ARO_DOWN	CTL(d)
#define	ARO_LEFT	CTL(l)
#define	ARO_RIGHT	CTL(r)

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
#ifdef	Z_SCCS
	unsigned
	char	z_rels, z_vers;	/* last sccs-release, version		*/
	time_t	z_time;		/* last sccs delta-date			*/
#endif	Z_SCCS
	} FLIST;

#define	cNAME	flist[curfile].name
#define	cSTAT	flist[curfile].s
#define	cFLAG	flist[curfile].flag

MAIN	char	old_wd[BUFSIZ],	/* original working-directory */
		new_wd[BUFSIZ],	/* current working directory */
		bfr_sh[BUFSIZ];	/* last $SHELL-command			*/

MAIN	FLIST	*flist;		/* pointer to display-list */

MAIN	int	cmdcol[4],	/* column in which to show cursor */
				/* 0=mode, 1=uid/gid, 2=normal */
		mark_W,		/* row of work-area marker */
		clr_sh,		/* true if we clear-screen after SHELL	*/
		curfile,	/* current file on which to operate */
		dateopt,	/* date-option (a,c,m = 0,1,2) */
		sortord,	/* sort-order (TRUE=reverse) */
		sortopt,	/* sort-option (a character) */
		G_opt,		/* show uid/gid field */
		I_opt,		/* show link/inode field */
		P_opt,		/* show filemode in octal vs normal */
		S_opt,		/* show filesize in blocks */
		U_opt;		/* show underlying file-info */
#ifdef	Z_SCCS
MAIN	int	V_opt,		/* show sccs-versions */
		Z_opt;		/* show sccs-information */
#endif	Z_SCCS

MAIN unsigned	numfiles;	/* total files in display-list */
