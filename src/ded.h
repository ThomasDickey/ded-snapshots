#ifndef	DED_H
#define	DED_H

/*
 * Created:	09 Nov 1987
 * Function:	Common definitions for 'ded' (directory editor)
 */

#define		CUR_PTYPES	/* use "td_curse.h" */
#define		CHR_PTYPES	/* use <ctype.h> */
#define		ERR_PTYPES	/* use <errno.h> */
#define		OPN_PTYPES	/* use <fcntl.h> */
#define		SIG_PTYPES	/* use <signal.h> */
#define		STR_PTYPES	/* use <string.h> */
#define		TIM_PTYPES	/* use <time.h> */
#include	<ptypes.h>
#include	<dyn_str.h>	/* dynamic strings */
#include	<td_regex.h>	/* regular expressions */

#ifdef	MAIN
#if	!defined(NO_IDENT)
static const char ded_h[] = "$Id: ded.h,v 12.59 2000/10/19 09:33:00 tom Exp $";
#endif
#endif	/* MAIN */

#ifndef S_IEXEC
#define S_IEXEC 0100	/* BeOS lacks this */
#endif

#if HAVE_STDARG_H && PROTOTYPES
#include	<stdarg.h>
#else
#include	<varargs.h>
#endif

#define	private	static
#define	public

#define	FREE(p)		dofree(p)

#define	OFF_T	long		/* lint libraries should have 'off_t' */

/*
 * Conditional-compilation variables
 */
#if defined(RCS_PATH) || defined(SCCS_PATH)
#  define Z_RCS_SCCS		/* compile-in common sccs/rcs support */
#endif

#ifdef SCCS_PATH
#  define	Z_SCCS		/* compile-in '-z' sccs support */
#endif

#ifdef RCS_PATH
#  define	Z_RCS		/* compile-in '-z' rcs support */
#endif

/*
 * Miscellaneous definitions
 */
#define	NO_HISTORY (HIST **)0

#if HAVE_NEW_TOKEN_QUOTE
#  define ENV(n) dftenv(n,#n)
#else
#  define ENV(n) dftenv(n,"n")
#endif

#define	UIDLEN	9		/* length of uid/gid field */

#ifdef S_IFBLK
#define	isDEV(mode)	(	(mode & S_IFMT) == S_IFBLK\
			||	(mode & S_IFMT) == S_IFCHR)
#else
#define	isDEV(mode)	(	(mode & S_IFMT) == S_IFCHR)
#endif

#ifndef	MAIN
#  define MAIN extern
#endif	/* MAIN */

/*
 * We store an array of FLIST structures to describe all files which can
 * be displayed in a given viewport.  This is denoted the 'display list'.
 */
#define	FLIST	struct	_flist
	FLIST	{
	FLIST	*next;
	char	*name;		/* name (within working-directory)	*/
	char	*ltxt;		/* what link resolves to		*/
	Stat_t	s;		/* stat()-block				*/
	short	namlen;		/* length of 'name' field		*/
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
#define for_each_file(gbl,x) for (x = 0; x < gbl->numfiles; x++)

#define	gENTRY(x)	gbl->flist[x]	/* passed-thru as argument */
#define	gNAME(x)	gENTRY(x).name
#define	gSTAT(x)	gENTRY(x).s
#define	gLTXT(x)	gENTRY(x).ltxt
#define	gFLAG(x)	gENTRY(x).flag
#define	gDORD(x)	gENTRY(x).dord

#define	gVERS(x)	gENTRY(x).z_vers
#define	gLOCK(x)	gENTRY(x).z_lock
#define	gTIME(x)	gENTRY(x).z_time

#define	cENTRY		gENTRY(gbl->curfile)
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

#define	PORT_MAX	2	/* number of viewports */

/*
 * The RING structure saves global data which lets us restore the state
 * of a file-list (see "ded.h"):
 */
#define	RING	struct	_ring
	RING {
	RING	*_link;
	char	new_wd[MAXPATHLEN];
	char	*toscan;	/* directory-scan expression	*/
	REGEX_T	scan_expr;	/* compiled version of 'toscan'	*/
	int	used_expr;	/* true iff we've compiled 'toscan' */
	DYN	*cmd_sh;	/* command-string, for %/! operations */
	FLIST	*flist;		/* list of filenames & Stat_t-blocks */

	char	**top_argv;
	int	top_argc;

	int	cmdcol[CCOL_MAX],/* column in which to show cursor */
				/* 0=mode, 1=uid/gid, 2=normal */
		base_of[PORT_MAX],/* nominal base-file of viewport */
		item_of[PORT_MAX],/* nominal current-file of viewport */
		clr_sh,		/* true if we clear-screen after SHELL	*/
		Xbase, Ybase,	/* viewport (for scrolling) */
		mrkfile,	/* current file marked with 'markC()' */
		dateopt,	/* date-option (a,c,m = 0,1,2) */
		sortord,	/* sort-order (TRUE=reverse) */
		sortopt,	/* sort-option (a character) */
		tagsort,	/* sort tagged files apart from others */
		tag_opt,	/* show totals for tagged files */
		AT_opt,		/* show symbolic link target */
		A_opt,		/* show "." and ".." */
		G_opt,		/* show uid/gid field */
		I_opt,		/* show link/inode field */
		P_opt,		/* show filemode in octal vs normal */
		S_opt,		/* show filesize in blocks */
		T_opt,		/* show long date+time */
		U_opt;		/* show underlying file-info */
#ifdef	Z_RCS_SCCS
	int	V_opt,		/* show sccs-versions */
		O_opt,		/* show sccs-lock owners */
		Z_opt;		/* show sccs-information */
#endif
	unsigned numfiles;	/* total files in display-list */
	unsigned curfile;	/* current file on which to operate */
	int	tag_count;	/* current # of tagged files */
	off_t	tag_bytes;	/* ...corresponding total of bytes */
	long	tag_blocks;	/* ...corresponding total of blocks */
	};

/*
 * Each type of command that we can type text into has a history table
 */
typedef	struct	_hist	{
	struct	_hist	*next;
	char		*text;
	} HIST;

/*
 * Definitions for inline edit/history:
 */
#define	C_NEXT	 0
#define	C_INIT	-1
#define	C_FIND	-2
#define	C_TOPC	-3
#define	C_ENDC	-4
#define	C_TRIM	-5
#define	C_QUIT	-6
#define	C_DONE	-7

/*
 * Global data (cf: dedring.c)
 */
MAIN	char	old_wd[MAXPATHLEN]; /* original working-directory */
MAIN	int	mark_W;		/* row of work-area marker */
MAIN	int	tree_visible;	/* denotes filelist vs directory-tree */
MAIN	int	gets_active;	/* true while in 'dlog_string()' */
MAIN	HIST	*cmd_history;	/* command-history shared by all filelists */
MAIN	int	first_scan;	/* true while processing command-line scan */


/* *** "boxchars.c" *** */
#define	BAR_WIDTH 4
extern	chtype	bar_space[];
extern	chtype	bar_hline[];
extern	chtype	bar_ruler[];

extern	void	boxchars(
		_ar1(int,	flag));

/* *** "ded.c" *** */
extern	int	debug;
extern	int	no_worry;
extern	int	in_screen;

extern	void	to_exit(
		_ar1(int,	last));

extern	int	realstat(
		_arx(RING *,	gbl)
		_arx(int,	inx)
		_ar1(Stat_t *,	sb));

extern	void	failed(
		_ar1(char *,	msg));

extern	int	user_says(
		_arx(RING *,	gbl)
		_ar1(int,	ok));

extern	int	findFILE(
		_arx(RING *,	gbl)
		_ar1(char *,	name));

extern	void	showSCCS(
		_ar1(RING *,	gbl));

extern	void	retouch(
		_arx(RING *,	gbl)
		_ar1(int,	row));

extern	void	resleep(
		_arx(RING *,	gbl)
		_arx(int,	count)
		_fn1(void,	func,	(_AR1(RING*,g))));

extern	char	*fixname(
		_arx(RING *,	gbl)
		_ar1(unsigned,	j));

extern	void	fixtime(
		_arx(RING *,	gbl)
		_ar1(unsigned,	j));

extern	void	usage(_ar0);

/* *** "dedblip.c" *** */
extern	void	set_dedblip (
		_ar1(RING *,	gbl));

extern	void	put_dedblip (
		_ar1(int,	code));

/* *** "dedcolor.c" *** */
#if HAVE_HAS_COLORS
extern	int	invert_colors;
extern	void	init_dedcolor(_ar0);
extern	void	dedcolor(
		_ar1(FLIST *,	entry));
#endif

/* *** "deddoit.c" *** */
extern	void	deddoit(
		_arx(RING *,	gbl)
		_arx(int,	key)
		_ar1(int,	sense));

/* *** "deddump.c" *** */
extern	void	deddump(
		_ar1(RING *,	gbl));

/* *** "dedfind.c" *** */
extern	void	dedfind(
		_arx(RING *,	gbl)
		_ar1(int,	key));

/* *** "dedfree.c" *** */
extern	FLIST	*dedfree(
		_arx(FLIST *,	fp)
		_ar1(unsigned,	num));

/* *** "dedline.c" *** */
extern	void	editprot(
		_ar1(RING *,	gbl));

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

/* *** "dedmake.c" *** */
extern	void	dedmake(
		_arx(RING *,	gbl)
		_ar1(int,	firstc));

/* *** "dedmsgs.c" *** */
extern	void	clearmsg(_ar0);

extern	void	dedmsg(
		_arx(RING *,	gbl)
		_ar1(char *,	msg));

extern	void	warn(
		_arx(RING *,	gbl)
		_ar1(char *,	msg));

extern	void	waitmsg(
		_ar1(char *,	msg));

extern	void	wait_warn(
		_ar1(char *,	msg));

/* *** "dedname.c" *** */
extern	int	dedname(
		_arx(RING *,	gbl)
		_arx(int,	x)
		_ar1(char *,	newname));

/* *** "dedread.c" *** */
extern	int	dedread(
		_arx(RING *,	gbl)
		_arx(char **,	pattern_)
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

extern	RING *	dedring(
		_arx(RING *,	gbl)
		_arx(char *,	path)
		_arx(int,	cmd)
		_arx(int,	count)
		_arx(int,	set_pattern)
		_ar1(char *,	pattern));

extern	RING *	ring_pointer(
		_arx(RING *,	gbl)
		_ar1(int,	count));

extern	char *	ring_path(
		_arx(RING *,	gbl)
		_ar1(int,	count));

extern	void	ring_rename(
		_arx(RING *,	gbl)
		_arx(char *,	oldname)
		_ar1(char *,	newname));

extern	void	ring_tags(_ar0);

/* *** "dedscan.c" *** */
extern	int	dedscan(
		_ar1(RING *,	gbl));

extern	void	statSCCS(
		_arx(RING *,	gbl)
		_arx(char *,	name)
		_ar1(FLIST *,	f_));

extern	void	statLINE(
		_arx(RING *,	gbl)
		_ar1(unsigned,	j));

extern	void	statMAKE(
		_arx(RING *,	gbl)
		_ar1(int,	mode));

extern	int	path_RESOLVE(
		_arx(RING *,	gbl)
		_ar1(char *,	path));

/* *** "dedshow.c" *** */
extern	void	dedshow2 (
		_ar1(char *,	arg));

extern	void	dedshow(
		_arx(RING *,	gbl)
		_arx(char *,	tag)
		_ar1(char *,	arg));

/* *** "dedsigs.c" *** */
extern	int	dedsigs(
		_ar1(int,	flag));

/* *** "dedsize.c" *** */
extern	void	dedsize(
		_ar1(RING *,	gbl));

/* *** "dedsort.c" *** */
extern	int	dedsort_cmp(
		_arx(RING *,	gbl)
		_arx(const FLIST *,	p1)
		_ar1(const FLIST *,	p2));

extern	void	dedsort(
		_ar1(RING *,	gbl));

/* *** "dedtags.c" *** */
extern	void	init_tags(
		_ar1(RING *,	gbl));

extern	void	tag_entry(
		_arx(RING *,	gbl)
		_arx(unsigned,	inx)
		_ar1(unsigned,	count));

extern	void	untag_entry(
		_arx(RING *,	gbl)
		_arx(unsigned,	inx)
		_ar1(unsigned,	count));

extern	void	count_tags(
		_ar1(RING *,	gbl));

/* *** "dedtype.c" *** */
extern	int	in_dedtype;
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

/* *** "dedview.c" *** */
extern	int	file2row(
		_ar1(unsigned,	n));

extern	int	move2row(
		_arx(unsigned,	n)
		_ar1(int,	col));

extern	void	clear_work(_ar0);

extern	void	to_work(
		_arx(RING *,	gbl)
		_ar1(int,	clear_it));

extern	int	to_file(
		_ar1(RING *,	gbl));

extern	void	scroll_to_file(
		_arx(RING *,	gbl)
		_ar1(unsigned,	inx));

extern	void	markset(
		_arx(RING *,	gbl)
		_ar1(unsigned,	num));

extern	void	upLINE(
		_arx(RING *,	gbl)
		_ar1(unsigned,	n));

extern	void	downLINE(
		_arx(RING *,	gbl)
		_ar1(unsigned,	n));

extern	int	showDOWN(
		_ar1(RING *,	gbl));

extern	void	showWHAT(
		_ar1(RING *,	gbl));

extern	void	showLINE(
		_arx(RING *,	gbl)
		_ar1(unsigned,	j));

extern	void	showMARK(
		_ar1(int,	col));

extern	void	showFILES(
		_arx(RING *,	gbl)
		_ar1(int,	reset_cols));

extern	void	openVIEW(
		_ar1(RING *,	gbl));

extern	void	redoVIEW(
		_arx(RING *,	gbl)
		_ar1(int,	freed));

extern	void	scrollVIEW(
		_arx(RING *,	gbl)
		_ar1(int,	count));

extern	RING *	splitVIEW(
		_ar1(RING *,	gbl));

extern	void	quitVIEW(
		_ar1(RING *,	gbl));

extern	void	top2VIEW(
		_ar1(RING *,	gbl));

extern	void	showC(
		_ar1(RING *,	gbl));

extern	RING *	tab2VIEW(
		_ar1(RING *,	gbl));

extern	void	markC(
		_arx(RING *,	gbl)
		_ar1(int,	on));

extern	unsigned baseVIEW(_ar0);

extern	int	lastVIEW(_ar0);

extern	RING *	row2VIEW(
		_arx(RING *,	gbl)
		_ar1(int,	row));

/* *** "dedwait.c" *** */
extern	void	dedwait(
		_arx(RING *,	gbl)
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

extern	int	ded_access(
		_arx(Stat_t *,	sb)
		_ar1(int,	mask));

#define ded_blocks(sb) fileblocks(sb)

extern	char	*type_uid2s(
		_ar1(Stat_t *,	s));

extern	int	has_extended_acl(
		_arx(RING *,	gbl)
		_ar1(int,	x));

/* *** "dlog.c" *** */
extern	void	dlog_read(
		_ar1(char *,	name));

extern	char	*dlog_open(
		_arx(char *,	name)
		_arx(int,	argc)
		_ar1(char **,	argv));

extern	void	dlog_reopen(_ar0);

extern	void	dlog_close(_ar0);

extern	void	dlog_exit(
		_ar1(int,	code));

extern	int	dlog_char(
		_arx(RING *,	gbl)
		_arx(int *,	count_)
		_ar1(int,	begin));

extern	void	dlog_prompt(
		_arx(RING *,	gbl)
		_arx(char *,	prompt)
		_ar1(int,	row));

#ifdef	SIGWINCH
extern	void	dlog_resize(_ar0);
#endif

extern	char *	dlog_string(
		_arx(RING *,	gbl)
		_arx(char *,	prompt)
		_arx(int,	row)
		_arx(DYN **,	result)
		_arx(DYN **,	inflag)
		_arx(HIST **,	history)
		_arx(int,	fast_q)
		_ar1(int,	wrap_len));

extern	void	dlog_elapsed(_ar0);

extern	void	dlog_flush(_ar0);

extern	void	dlog_name(
		_ar1(char *,	name));

extern	void	dlog_comment(
#if PROTOTYPES
# if HAVE_STDARG_H
			char * fmt,
			...
# endif
#endif
		);

/* *** "ftree.c" *** */
extern	void	ft_insert(
		_ar1(char *,	path));

extern	void	ft_remove(
		_arx(char *,	path)
		_arx(int,	links)
		_ar1(int,	dots));

extern	void	ft_purge(
		_ar1(RING *,	gbl));

extern	void	ft_rename(
		_arx(char *,	oldname)
		_ar1(char *,	newname));

extern	void	ft_read(
		_arx(char *,	first)
		_ar1(char *,	home_dir));

#ifdef	SIGWINCH
extern	int	ft_resize(_ar0);
#endif
extern	RING *	ft_view(
		_arx(RING *,	gbl)
		_arx(char *,	path)
		_ar1(int *,	cmd));

extern	int	ft_scan(
		_arx(RING *,	gbl)
		_arx(int,	node)
		_arx(int,	levels)
		_ar1(int,	base));

extern	void	ft_set_levels (
		_arx(int,	row)
		_ar1(int,	levels));

extern	int	ft_stat(
		_arx(char *,	name)
		_ar1(char *,	leaf));

extern	void	ft_write(_ar0);

/* *** "history.c" *** */
extern	HIST	*cmd_history;

extern	void	put_history(
		_arx(HIST **,	table)
		_ar1(char *,	text));

extern	char *	get_history(
		_arx(HIST *,	table)
		_ar1(int,	age));

extern	void	show_history(
		_arx(RING *,	gbl)
		_ar1(int,	depth));

/* *** "restat.c" *** */
extern	void	restat(
		_arx(RING *,	gbl)
		_ar1(int,	group));

extern	void	restat_l(
		_ar1(RING *,	gbl));

extern	void	restat_W(
		_ar1(RING *,	gbl));

/* *** "inline.c" *** */
extern	int	dyn_trim1(
		_ar1(DYN *,	p));

extern	void	hide_inline(
		_ar1(int,	flag));

extern	int	edit_inline(
		_ar1(int,	flag));

extern	int	up_inline(_ar0);
extern	int	down_inline(_ar0);

extern	int	get_inline(
		_arx(RING *,	gbl)
		_arx(int,	c)
		_ar1(int,	cmd));

#define	ReplayStart(chr)	(void)get_inline(gbl,chr,chr)
#define	ReplayFinish()		(void)get_inline(gbl,EOS,C_DONE)
#define	ReplayInit(chr)		      get_inline(gbl,chr,C_INIT)
#define	ReplayTopC(chr)		(void)get_inline(gbl,chr,C_TOPC)
#define	ReplayEndC()		      get_inline(gbl,EOS,C_ENDC)
#define	ReplayFind(chr)		(void)get_inline(gbl,chr,C_FIND)
#define	ReplayTrim()		(void)get_inline(gbl,EOS,C_TRIM)
#define	ReplayQuit()		(void)get_inline(gbl,EOS,C_QUIT)
#define	ReplayChar()		      get_inline(gbl,EOS,C_NEXT)

extern	DYN **	inline_text(_ar0);
extern	HIST **	inline_hist(_ar0);
#ifdef	DEBUG
extern	int	inline_hidden(_ar0);
#endif

/* *** "showpath.c" *** */
extern	void	showpath(
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

#endif	/* DED_H */
