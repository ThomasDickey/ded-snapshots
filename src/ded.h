/*
 * Common definitions for 'ded' (directory editor)
 */
#include	<curses.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
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

/*
 * Map differences between BSD4.2 and SYSTEM5
 */
#ifdef	SYSTEM5
#define	lstat	stat
#define	utimes	utime
typedef struct screen { char	dummy; };
#else	SYSTEM5
typedef char	chtype;		/* sys5-curses data-type */
#endif	SYSTEM5

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

#define	isDIR(mode)	((mode & S_IFMT) == S_IFDIR)
#define	isDEV(mode)	(	(mode & S_IFMT) == S_IFBLK\
			||	(mode & S_IFMT) == S_IFCHR)
#define	isFILE(mode)	((mode & S_IFMT) == S_IFREG)
#define	isLINK(mode)	((mode & S_IFMT) == S_IFLNK)

#ifndef	MAIN
#define	MAIN	extern
#endif	MAIN

typedef	struct	{
	char		*name;	/* name (within working-directory) */
	char		*ltxt;	/* what link resolves to */
	struct	stat	s;	/* stat()-block */
	char		flag;	/* tag-flag */
	} FLIST;

#define	cNAME	flist[curfile].name
#define	cSTAT	flist[curfile].s
#define	cFLAG	flist[curfile].flag

MAIN	char	old_wd[BUFSIZ],	/* original working-directory */
		new_wd[BUFSIZ];	/* current working directory */

MAIN	FLIST	*flist;		/* pointer to display-list */

MAIN	int	cmdcol[4],	/* column in which to show cursor */
				/* 0=mode, 1=uid/gid, 2=normal */
		mark_W,		/* row of work-area marker */
		count,		/* repeat-count for current command */
		curfile,	/* current file on which to operate */
		dateopt,	/* date-option (a,c,m = 0,1,2) */
		sortord,	/* sort-order (TRUE=reverse) */
		sortopt,	/* sort-option (a character) */
		G_opt,		/* show uid/gid field */
		I_opt,		/* show link/inode field */
		P_opt,		/* show filemode in octal vs normal */
		S_opt;		/* show filesize in blocks */

MAIN unsigned	numfiles;	/* total files in display-list */
