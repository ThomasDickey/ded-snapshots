#ifndef	_ded_h
#define	_ded_h

#ifdef	MAIN
#ifndef	lint
static	char	*ded_h = "$Id: ded.h,v 10.3 1992/03/12 12:13:38 dickey Exp $";
#endif
#endif	/* MAIN */

/*
 * Created:	09 Nov 1987
 * Function:	Common definitions for 'ded' (directory editor)
 */

#define		CUR_PTYPES	/* use "curses" */
#define		STR_PTYPES	/* use "strrchr" */
#include	<ptypes.h>
#include	<dyn_string.h>
#include	<ctype.h>
#include	<errno.h>
#include	<cmdch.h>
extern	char	*sys_errlist[];

/*
 * Definitions to make linting easier
 */
#define	FREE(p)		dofree(p)
#define	_ONE(t,a)	(_AR1(t,a)) _DCL(t,a)

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
#else	/* SYSTEM5 */
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
#ifdef	__STDCPP__
#define	ENV(n)	dftenv(n,#n)
#else
#define	ENV(n)	dftenv(n,"n")
#endif

#define	UIDLEN	9		/* length of uid/gid field */

#define	HOUR		(60*60)	/* unix time for one hour */

#define	isDEV(mode)	(	(mode & S_IFMT) == S_IFBLK\
			||	(mode & S_IFMT) == S_IFCHR)

#ifndef	MAIN
#define	MAIN	extern
#endif	/* MAIN */

/*
 * We store an array of FLIST structures to describe all files which can
 * be displayed in a given viewport.  This is denoted the 'display list'.
 */
typedef	struct	_flist	{
	struct	_flist	*next;
	char		*name;	/* name (within working-directory)	*/
	char		*ltxt;	/* what link resolves to		*/
	STAT		s;	/* stat()-block				*/
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
		*scan_expr;	/* compiled version of 'toscan'		*/
MAIN	DYN	*cmd_sh;	/* last $SHELL-command			*/

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
		tag_opt,	/* show totals for tagged files */
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
		T_opt;		/* show long date+time */
		U_opt;		/* show underlying file-info */
#ifdef	Z_RCS_SCCS
MAIN	int	V_opt,		/* show sccs-versions */
		Y_opt,		/* show sccs-locks */
		Z_opt;		/* show sccs-information */
#endif	/* Z_RCS_SCCS */

MAIN unsigned	numfiles;	/* total files in display-list */

/* *** "ded.c" *** */
extern	int	debug;
extern	int	no_worry;

extern	int	file2row(
		_ar1(int,	n));

extern	int	move2row(
		_arx(int,	n)
		_ar1(int,	col));

extern	int	to_exit(
		_ar1(int,	last));

extern	int	clear_work(_ar0);

extern	int	to_work(
		_ar1(int,	clear_it));

extern	int	to_file(_ar0);

extern	int	scroll_to_file(
		_ar1(int,	inx));

extern	int	markset(
		_arx(int,	num)
		_ar1(int,	reset_work));

extern	int	realstat(
		_arx(int,	inx)
		_ar1(struct stat *,sb));

extern	void	clearmsg(_ar0);

extern	char	*err_msg(
		_ar1(char *,	msg));

extern	int	dedmsg(
		_ar1(char *,	msg));

extern	int	warn(
		_ar1(char *,	msg));

extern	int	waitmsg(
		_ar1(char *,	msg));

extern	int	wait_warn(
		_ar1(char *,	msg));

extern	int	failed(
		_ar1(char *,	msg));

extern	int	user_says(
		_ar1(int,	ok));

extern	int	findFILE(
		_ar1(char *,	name));

extern	int	upLINE(
		_ar1(int,	n));

extern	int	downLINE(
		_ar1(int,	n));

extern	int	showDOWN(_ar0);

extern	int	forward(
		_ar1(int,	n));

extern	int	backward(
		_ar1(int,	n));

extern	int	showWHAT(_ar0);

extern	int	showLINE(
		_ar1(int,	j));

extern	int	showVIEW(_ar0);

extern	int	showMARK(
		_ar1(int,	col));

extern	int	showFILES(
		_arx(int,	reset_cols)
		_ar1(int,	reset_work));

extern	int	showSCCS(_ar0);

extern	int	showC(_ar0);

extern	int	markC(
		_ar1(int,	on));

extern	int	retouch(
		_ar1(int,	row));

extern	int	rescan(
		_arx(int,	fwd)
		_ar1(char *,	backto));

extern	int	restat(
		_ar1(int,	group));

extern	int	restat_l(_ar0);

extern	int	restat_W(_ar0);

extern	int	resleep(
		_arx(int,	count)
		_fn1(int,	func));

extern	char	*fixname(
		_ar1(int,	j));

extern	int	fixtime(
		_ar1(int,	j));

extern	int	usage(_ar0);

/* *** "deddoit.c" *** */
extern	int	deddoit(
		_arx(int,	key)
		_ar1(int,	sense));

/* *** "deddump.c" *** */
extern	int	deddump(_ar0);

/* *** "dedfind.c" *** */
extern	int	dedfind(
		_ar1(int,	key));

/* *** "dedfree.c" *** */
extern	FLIST	*dedfree(
		_arx(FLIST *,	fp)
		_ar1(unsigned,	num));

/* *** "dedline.c" *** */
extern	int	replay(
		_ar1(int,	cmd));

extern	int	editprot(_ar0);

extern	int	edittext(
		_arx(int,	endc)
		_arx(int,	col)
		_arx(int,	len)
		_ar1(char *,	bfr));

extern	int	edit_uid(_ar0);

extern	int	edit_gid(_ar0);

extern	int	editname(_ar0);

extern	int	editlink(
		_ar1(int,	cmd));

extern	int	dedline(
		_ar1(int,	flag));

/* *** "dedmake.c" *** */
extern	int	dedmake(
		_ar1(int,	firstc));

/* *** "dedname.c" *** */
extern	int	dedname(
		_arx(int,	x)
		_ar1(char *,	newname));

/* *** "dedread.c" *** */
extern	int	dedread(
		_arx(char *,	*pattern_)
		_ar1(int,	change_needed));

extern	int	init_scan(_ar0);

extern	int	ok_scan(
		_ar1(char *,	name));

/* *** "dedring.c" *** */
extern	int	dedring(
		_arx(char *,	path)
		_arx(int,	cmd)
		_arx(int,	count)
		_arx(int,	set_pattern)
		_ar1(char *,	pattern));

extern	int	dedrang(
		_ar1(char *,	path));

extern	char	*dedrung(
		_ar1(int,	count));

extern	void	dedrering(
		_arx(char *,	oldname)
		_ar1(char *,	newname));

/* *** "dedscan.c" *** */
extern	int	dedscan(
		_arx(int,	argc)
		_ar1(char **,	argv));

extern	int	statSCCS(
		_arx(char *,	name)
		_ar1(FLIST *,	f_));

extern	int	statLINE(
		_ar1(int,	j));

extern	int	statMAKE(
		_ar1(int,	mode));

extern	int	path_RESOLVE(
		_ar1(char *,	path));

/* *** "dedshow.c" *** */
extern	int	dedshow(
		_arx(char *,	tag)
		_ar1(char *,	arg));

/* *** "dedsigs.c" *** */
extern	int	dedsigs(
		_ar1(int,	flag));

/* *** "dedsort.c" *** */
extern	int	dedsort_cmp(
		_arx(FLIST *,	p1)
		_ar1(FLIST *,	p2));

extern	int	dedsort(_ar0);

/* *** "dedtype.c" *** */
extern	int	dedtype(
		_arx(char *,	name)
		_arx(int,	inlist)
		_arx(int,	binary)
		_arx(int,	stripped)
		_ar1(int,	isdir));

/* *** "deduniq.c" *** */
extern	int	deduniq(
		_ar1(int,	level));

/* *** "dedwait.c" *** */
extern	int	dedwait(
		_ar1(int,	cursed));

/* *** "ded2s.c" *** */
extern	int	ded2s(
		_arx(int,	inx)
		_arx(char *,	bfr)
		_ar1(int,	len));

extern	int	ded2string(
		_arx(char *,	bfr)
		_arx(int,	len)
		_arx(char *,	name)
		_ar1(int,	flag));

extern	char	*type_uid2s(
		_ar1(struct stat *,s));

extern	int	has_extended_acl(
		_ar1(int,	x));

/* *** "dlog.c" *** */
extern	int	dlog_read(
		_ar1(char *,	name));

extern	char	*dlog_open(
		_arx(char *,	name)
		_arx(int,	argc)
		_ar1(char **,	argv));

extern	int	dlog_reopen(_ar0);

extern	int	dlog_close(_ar0);

extern	int	dlog_exit(
		_ar1(int,	code));

extern	int	dlog_char(
		_arx(int,	*count_)
		_ar1(int,	begin));

extern	int	dlog_string(
		_arx(char *,	s)
		_arx(int,	len)
		_ar1(int,	wrap));

extern	int	dlog_elapsed(_ar0);

extern	int	dlog_flush(_ar0);

extern	int	dlog_name(
		_ar1(char *,	name));

extern	int	dlog_comment(
#ifdef	__STDC__
		...
#endif
		);

/* *** "ftree.c" *** */
extern	int	ft_insert(
		_ar1(char *,	path));

extern	int	ft_remove(
		_arx(char *,	path)
		_ar1(int,	all));

extern	int	ft_purge(_ar0);

extern	int	ft_rename(
		_arx(char *,	old)
		_ar1(char *,	new));

extern	int	ft_read(
		_arx(char *,	first)
		_ar1(char *,	home_dir));

extern	int	ft_view(
		_ar1(char *,	path));

extern	int	ft_scan(
		_arx(int,	node)
		_arx(int,	levels)
		_ar1(int,	base));

extern	int	ft_stat(
		_arx(char *,	name)
		_ar1(char *,	leaf));

extern	int	ft_write(_ar0);

/* *** "showpath.c" *** */
extern	int	showpath(
		_arx(char *,	path)
		_arx(int,	level)
		_arx(int,	base)
		_ar1(int,	margin));

/* *** "sortset.c" *** */
extern	char	sortc[];

extern	int	sortset(
		_arx(int,	ord)
		_ar1(int,	opt));

extern	int	sortget(
		_ar1(int,	c));

#endif	/* _ded_h */
