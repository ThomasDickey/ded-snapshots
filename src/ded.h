#ifndef	_ded_h
#define	_ded_h

#ifdef	MAIN
#ifndef	lint
static	char	*ded_h = "$Id: ded.h,v 10.23 1992/04/02 14:10:29 dickey Exp $";
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

#define	private	static
#define	public

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
#define	FLIST	struct	_flist
typedef	FLIST	{
	FLIST	*next;
	char	*name;		/* name (within working-directory)	*/
	char	*ltxt;		/* what link resolves to		*/
	STAT	s;		/* stat()-block				*/
	short	dord;		/* directory-order, for "d" sort	*/
	char	flag;		/* tag-flag				*/
#ifdef	Z_RCS_SCCS
	char	*z_vers;	/* last sccs-release, version		*/
	char	*z_lock;	/* current locker (user-name)		*/
	time_t	z_time;		/* last sccs delta-date			*/
#endif	/* Z_RCS_SCCS */
	};

/*
 * Short-hand expressions:
 */
#define	xENTRY(x)	FOO->flist[x]	/* passed as global */
#define	xNAME(x)	xENTRY(x).name
#define	xSTAT(x)	xENTRY(x).s
#define	xLTXT(x)	xENTRY(x).ltxt
#define	xFLAG(x)	xENTRY(x).flag
#define	xDORD(x)	xENTRY(x).dord

#define	xVERS(x)	xENTRY(x).z_vers
#define	xLOCK(x)	xENTRY(x).z_lock
#define	xTIME(x)	xENTRY(x).z_time

#define	gENTRY(x)	gbl->flist[x]	/* passed-thru as argument */
#define	gNAME(x)	gENTRY(x).name
#define	gSTAT(x)	gENTRY(x).s
#define	gLTXT(x)	gENTRY(x).ltxt
#define	gFLAG(x)	gENTRY(x).flag
#define	gDORD(x)	gENTRY(x).dord

#define	gVERS(x)	gENTRY(x).z_vers
#define	gLOCK(x)	gENTRY(x).z_lock
#define	gTIME(x)	gENTRY(x).z_time

#define	cENTRY		xENTRY(FOO->curfile)
#define	cNAME		cENTRY.name
#define	cSTAT		cENTRY.s
#define	cLTXT		cENTRY.ltxt
#define	cFLAG		cENTRY.flag
#define	cDORD		cENTRY.dord

#define	cVERS		cENTRY.z_vers
#define	cLOCK		cENTRY.z_lock
#define	cTIME		cENTRY.z_time

#define	GROUPED(n)	(gFLAG(n) || ((n) == gbl->curfile))

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
 * The RING structure saves global data which lets us restore the state
 * of a file-list (see "ded.h"):
 */
#define	RING	struct	_ring
typedef	RING {
	RING	*_link;
	DYN	*sort_key;	/* 'new_wd', translated for sorting */
	char	new_wd[BUFSIZ],
		*toscan,	/* directory-scan expression	*/
		*scan_expr;	/* compiled version of 'toscan'	*/
	DYN	*cmd_sh;
	FLIST	*flist;
	char	**top_argv;
	int	top_argc;
	int	cmdcol[CCOL_MAX],/* column in which to show cursor */
				/* 0=mode, 1=uid/gid, 2=normal */
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
#endif
		A_opt,		/* show "." and ".." */
		G_opt,		/* show uid/gid field */
		I_opt,		/* show link/inode field */
#ifdef	apollo_sr10
		O_opt,		/* show apollo object-types */
#endif
		P_opt,		/* show filemode in octal vs normal */
		S_opt,		/* show filesize in blocks */
		T_opt,		/* show long date+time */
		U_opt;		/* show underlying file-info */
#ifdef	Z_RCS_SCCS
	int	V_opt,		/* show sccs-versions */
		Y_opt,		/* show sccs-locks */
		Z_opt;		/* show sccs-information */
#endif
	unsigned numfiles;	/* total files in display-list */
	};

/*
 * Global data (cf: dedring.c)
 */
MAIN	char	old_wd[BUFSIZ];	/* original working-directory */
MAIN	int	mark_W,		/* row of work-area marker */
		Xbase,
		Ybase;
MAIN	RING	*FOO;		/* current list (patch: 'X' command?) */

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
		_ar1(STAT *,	sb));

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

extern	void	showLINE(
		_arx(RING *,	gbl)
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
extern	void	deddoit(
		_arx(RING *,	gbl)
		_arx(int,	key)
		_ar1(int,	sense));

/* *** "deddump.c" *** */
extern	int	deddump(_ar0);

/* *** "dedfind.c" *** */
extern	void	dedfind(
		_arx(RING *,	gbl)
		_ar1(int,	key));

/* *** "dedfree.c" *** */
extern	FLIST	*dedfree(
		_arx(FLIST *,	fp)
		_ar1(unsigned,	num));

/* *** "dedline.c" *** */
extern	int	replay(
		_ar1(int,	cmd));

extern	void	editprot(
		_arx(RING *,	gbl));

extern	int	edittext(
		_arx(RING *,	gbl)
		_arx(int,	endc)
		_arx(int,	col)
		_arx(int,	len)
		_ar1(char *,	bfr));

extern	void	edit_uid(
		_ar1(RING *,	gbl));

extern	void	edit_gid(
		_ar1(RING *,	gbl));

extern	void	editname(
		_ar1(RING *,	gbl));

extern	void	editlink(
		_arx(RING *,	gbl)
		_ar1(int,	cmd));

extern	int	dedline(
		_ar1(int,	flag));

/* *** "dedmake.c" *** */
extern	void	dedmake(
		_arx(RING *,	gbl)
		_ar1(int,	firstc));

/* *** "dedname.c" *** */
extern	int	dedname(
		_arx(RING *,	gbl)
		_arx(int,	x)
		_ar1(char *,	newname));

/* *** "dedread.c" *** */
extern	int	dedread(
		_arx(RING *,	gbl)
		_arx(char *,	*pattern_)
		_ar1(int,	change_needed));

extern	void	init_scan(
		_ar1(RING *,	gbl));

extern	int	ok_scan(
		_arx(RING *,	gbl)
		_ar1(char *,	name));

/* *** "dedring.c" *** */
extern	RING	*ring_alloc(_ar0);

extern	void	ring_args(
		_arx(RING *,	gbl)
		_arx(int,	argc)
		_ar1(char **,	argv));

extern	RING *	ring_get(
		_ar1(char *,	path));

extern	int	dedring(
		_arx(char *,	path)
		_arx(int,	cmd)
		_arx(int,	count)
		_arx(int,	set_pattern)
		_ar1(char *,	pattern));

extern	RING *	ring_pointer(
		_ar1(int,	count));

extern	char *	ring_path(
		_ar1(int,	count));

extern	void	ring_rename(
		_arx(RING *,	gbl)
		_arx(char *,	oldname)
		_ar1(char *,	newname));

/* *** "dedscan.c" *** */
extern	int	dedscan(
		_arx(RING *,	gbl)
		_arx(int,	argc)
		_ar1(char **,	argv));

extern	void	statSCCS(
		_arx(RING *,	gbl)
		_arx(char *,	name)
		_ar1(FLIST *,	f_));

extern	void	statLINE(
		_arx(RING *,	gbl)
		_ar1(int,	j));

extern	void	statMAKE(
		_arx(RING *,	gbl)
		_ar1(int,	mode));

extern	int	path_RESOLVE(
		_arx(RING *,	gbl)
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
		_arx(RING *,	gbl)
		_arx(FLIST *,	p1)
		_ar1(FLIST *,	p2));

extern	void	dedsort(
		_ar1(RING *,	gbl));

/* *** "dedtype.c" *** */
extern	void	dedtype(
		_arx(RING *,	gbl)
		_arx(char *,	name)
		_arx(int,	inlist)
		_arx(int,	binary)
		_arx(int,	stripped)
		_ar1(int,	isdir));

/* *** "deduniq.c" *** */
extern	void	deduniq(
		_arx(RING *,	gbl)
		_ar1(int,	level));

/* *** "dedwait.c" *** */
extern	int	dedwait(
		_ar1(int,	cursed));

/* *** "ded2s.c" *** */
extern	void	ded2s(
		_arx(RING *,	gbl)
		_arx(int,	inx)
		_arx(char *,	bfr)
		_ar1(int,	len));

extern	int	ded2string(
		_arx(RING *,	gbl)
		_arx(char *,	bfr)
		_arx(int,	len)
		_arx(char *,	name)
		_ar1(int,	flag));

extern	char	*type_uid2s(
		_ar1(STAT *,	s));

extern	int	has_extended_acl(
		_arx(RING *,	gbl)
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
extern	void	ft_insert(
		_ar1(char *,	path));

extern	void	ft_remove(
		_arx(char *,	path)
		_ar1(int,	all));

extern	void	ft_purge(
		_ar1(RING *,	gbl));

extern	void	ft_rename(
		_arx(char *,	old)
		_ar1(char *,	new));

extern	void	ft_read(
		_arx(char *,	first)
		_ar1(char *,	home_dir));

extern	int	ft_view(
		_arx(RING *,	gbl)
		_ar1(char *,	path));

extern	int	ft_scan(
		_arx(RING *,	gbl)
		_arx(int,	node)
		_arx(int,	levels)
		_ar1(int,	base));

extern	int	ft_stat(
		_arx(char *,	name)
		_ar1(char *,	leaf));

extern	void	ft_write(_ar0);

/* *** "showpath.c" *** */
extern	int	showpath(
		_arx(char *,	path)
		_arx(int,	level)
		_arx(int,	base)
		_ar1(int,	margin));

/* *** "sortset.c" *** */
extern	char	sortc[];

extern	int	sortset(
		_arx(RING *,	gbl)
		_arx(int,	ord)
		_ar1(int,	opt));

extern	int	sortget(
		_arx(RING *,	gbl)
		_ar1(int,	c));

#endif	/* _ded_h */
