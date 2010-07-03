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
static const char ded_h[] = "$Id: ded.h,v 12.68 2010/07/03 17:19:32 tom Exp $";
#endif
#endif	/* MAIN */

#ifndef S_IEXEC
#define S_IEXEC 0100	/* BeOS lacks this */
#endif

#include	<stdarg.h>

#define	FREE(p)		dofree(p)

#define	OFF_T	long		/* lint libraries should have 'off_t' */

/*
 * Conditional-compilation variables
 */
#if defined(RCS_PATH) || defined(SCCS_PATH) || defined(CVS_PATH) || defined(SVN_PATH)
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

#if defined(HAVE_NEW_TOKEN_QUOTE)
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
	FLIST	*z_next;
	char	*z_name;	/* name (within working-directory)	*/
	char	*z_ltxt;	/* what link resolves to		*/
	Stat_t	s;		/* stat()-block				*/
	short	z_namlen;	/* length of 'z_name' field		*/
	short	z_dord;		/* directory-order, for "d" sort	*/
	char	z_flag;		/* tag-flag				*/
#ifdef	Z_RCS_SCCS
	char	*z_vers;	/* last sccs-release, version		*/
	char	*z_lock;	/* current locker (user-name)		*/
	time_t	z_time;		/* last sccs delta-date			*/
#endif	/* Z_RCS_SCCS */
#ifdef MIXEDCASE_FILENAMES
#define z_real_name z_name
#else
#define z_real_name z_mono_name
	char	*z_mono_name;	/* z_name, monocased for sorting	*/
#endif
	};

/*
 * Short-hand expressions:
 */
#define for_each_file(gbl,x) for (x = 0; x < gbl->numfiles; x++)

#define	gENTRY(x)	gbl->flist[x]	/* passed-thru as argument */
#define	gNAME(x)	gENTRY(x).z_name
#define	gSTAT(x)	gENTRY(x).s
#define	gLTXT(x)	gENTRY(x).z_ltxt
#define	gFLAG(x)	gENTRY(x).z_flag
#define	gDORD(x)	gENTRY(x).z_dord

#define	gVERS(x)	gENTRY(x).z_vers
#define	gLOCK(x)	gENTRY(x).z_lock
#define	gTIME(x)	gENTRY(x).z_time

#define	cENTRY		gENTRY(gbl->curfile)
#define	cNAME		cENTRY.z_name
#define	cSTAT		cENTRY.s
#define	cLTXT		cENTRY.z_ltxt
#define	cFLAG		cENTRY.z_flag
#define	cDORD		cENTRY.z_dord

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
	long	tag_bytes;	/* ...corresponding total of bytes */
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
MAIN	int	edit_dates;	/* true if we may edit dates */


/* *** "boxchars.c" *** */
#define	BAR_WIDTH 4
extern	chtype	bar_space[];
extern	chtype	bar_hline[];
extern	chtype	bar_ruler[];

extern	void	boxchars(
		int	flag);

/* *** "ded.c" *** */
extern	int	debug;
extern	int	no_worry;
extern	int	in_screen;

extern	void	to_exit(
		int	last);

extern	int	realstat(
		RING *	gbl,
		int	inx,
		Stat_t *	sb);

extern	void	failed(
		const char *	msg);

extern	int	user_says(
		RING *	gbl,
		int	ok);

extern	int	findFILE(
		RING *	gbl,
		char *	name);

extern	void	showSCCS(
		RING *	gbl);

extern	void	retouch(
		RING *	gbl,
		int	row);

extern	void	resleep(
		RING *	gbl,
		int	count,
		void (*func)	(RING* g));

extern	char	*fixname(
		RING *	gbl,
		unsigned	j);

extern	void	fixtime(
		RING *	gbl,
		unsigned	j);

extern	void	usage(void);

/* *** "dedblip.c" *** */
extern	void	set_dedblip (
		RING *	gbl);

extern	void	put_dedblip (
		int	code);

/* *** "dedcolor.c" *** */
#if defined(HAVE_HAS_COLORS)
extern	int	invert_colors;
extern	void	init_dedcolor(void);
extern	void	dedcolor(
		FLIST *	entry);
#endif

/* *** "deddoit.c" *** */
extern	void	deddoit(
		RING *	gbl,
		int	key,
		int	sense);

/* *** "deddump.c" *** */
extern	void	deddump(
		RING *	gbl);

/* *** "dedfind.c" *** */
extern	void	dedfind(
		RING *	gbl,
		int	key);

/* *** "dedfree.c" *** */
extern	FLIST	*dedfree(
		FLIST *	fp,
		unsigned	num);

/* *** "dedline.c" *** */
extern	void	editprot(
		RING *	gbl);

extern	int	edittext(
		RING *	gbl,
		int	endc,
		int	col,
		int	len,
		char *	bfr);

extern	void	edit_uid(
		RING *	gbl);

extern	void	edit_gid(
		RING *	gbl);

extern	void	editdate(
		RING *	gbl,
		int	current,
		int	recur);

extern	void	editname(
		RING *	gbl);

extern	void	editlink(
		RING *	gbl,
		int	cmd);

/* *** "dedmake.c" *** */
extern	void	dedmake(
		RING *	gbl,
		int	firstc);

/* *** "dedmsgs.c" *** */
extern	void	clearmsg(void);

extern	void	dedmsg(
		RING *	gbl,
		char *	msg);

extern	void	warn(
		RING *	gbl,
		char *	msg);

extern	void	waitmsg(
		char *	msg);

extern	void	wait_warn(
		char *	msg);

/* *** "dedname.c" *** */
extern	int	dedname(
		RING *	gbl,
		int	x,
		char *	newname);

/* *** "dedread.c" *** */
extern	int	dedread(
		RING *	gbl,
		char **	pattern_,
		int	change_needed);

extern	void	init_scan(
		RING *	gbl);

extern	int	ok_scan(
		RING *	gbl,
		char *	name);

/* *** "dedring.c" *** */
extern	RING	*ring_alloc(void);

extern	void	ring_args(
		RING *	gbl,
		int	argc,
		char **	argv);

extern	RING *	ring_get(
		char *	path);

extern	RING *	dedring(
		RING *	gbl,
		char *	path,
		int	cmd,
		int	count,
		int	set_pattern,
		char *	pattern);

extern	RING *	ring_pointer(
		RING *	gbl,
		int	count);

extern	char *	ring_path(
		RING *	gbl,
		int	count);

extern	void	ring_rename(
		RING *	gbl,
		char *	oldname,
		char *	newname);

extern	void	ring_tags(void);

/* *** "dedscan.c" *** */
extern	int	dedscan(
		RING *	gbl);

extern	void	statSCCS(
		RING *	gbl,
		char *	name,
		FLIST *	f_);

extern	void	statLINE(
		RING *	gbl,
		unsigned	j);

extern	void	statMAKE(
		RING *	gbl,
		int	mode);

extern	int	path_RESOLVE(
		RING *	gbl,
		char *	path);

/* *** "dedshow.c" *** */
extern	void	dedshow2 (
		char *	arg);

extern	void	dedshow(
		RING *	gbl,
		char *	tag,
		char *	arg);

/* *** "dedsigs.c" *** */
extern	int	dedsigs(
		int	flag);

/* *** "dedsize.c" *** */
extern	void	dedsize(
		RING *	gbl);

/* *** "dedsort.c" *** */
extern	int	dedsort_cmp(
		RING *	gbl,
		const FLIST *	p1,
		const FLIST *	p2);

extern	void	dedsort(
		RING *	gbl);

/* *** "dedtags.c" *** */
extern	void	init_tags(
		RING *	gbl);

extern	void	tag_entry(
		RING *	gbl,
		unsigned	inx,
		unsigned	count);

extern	void	untag_entry(
		RING *	gbl,
		unsigned	inx,
		unsigned	count);

extern	void	count_tags(
		RING *	gbl);

/* *** "dedtype.c" *** */
extern	int	in_dedtype;
extern	void	dedtype(
		RING *	gbl,
		char *	name,
		int	inlist,
		int	binary,
		int	stripped,
		int	isdir);

/* *** "deduniq.c" *** */
extern	void	deduniq(
		RING *	gbl,
		int	level);

/* *** "dedview.c" *** */
extern	int	file2row(
		unsigned	n);

extern	int	move2row(
		unsigned	n,
		int	col);

extern	void	clear_work(void);

extern	void	to_work(
		RING *	gbl,
		int	clear_it);

extern	int	to_file(
		RING *	gbl);

extern	void	scroll_to_file(
		RING *	gbl,
		unsigned	inx);

extern	void	markset(
		RING *	gbl,
		unsigned	num);

extern	void	upLINE(
		RING *	gbl,
		unsigned	n);

extern	void	downLINE(
		RING *	gbl,
		unsigned	n);

extern	int	showDOWN(
		RING *	gbl);

extern	void	showWHAT(
		RING *	gbl);

extern	void	showLINE(
		RING *	gbl,
		unsigned	j);

extern	void	showMARK(
		int	col);

extern	void	showFILES(
		RING *	gbl,
		int	reset_cols);

extern	void	openVIEW(
		RING *	gbl);

extern	void	redoVIEW(
		RING *	gbl,
		int	freed);

extern	void	scrollVIEW(
		RING *	gbl,
		int	count);

extern	RING *	splitVIEW(
		RING *	gbl);

extern	void	quitVIEW(
		RING *	gbl);

extern	void	top2VIEW(
		RING *	gbl);

extern	void	showC(
		RING *	gbl);

extern	RING *	tab2VIEW(
		RING *	gbl);

extern	void	markC(
		RING *	gbl,
		int	on);

extern	unsigned baseVIEW(void);

extern	int	lastVIEW(void);

extern	RING *	row2VIEW(
		RING *	gbl,
		int	row);

/* *** "dedwait.c" *** */
extern	void	dedwait(
		RING *	gbl,
		int	cursed);

/* *** "ded2s.c" *** */
extern	void	ded2s(
		RING *	gbl,
		int	inx,
		char *	bfr,
		int	len);

extern	int	ded2string(
		RING *	gbl,
		char *	bfr,
		int	len,
		char *	name,
		int	flag);

extern	int	ded_access(
		Stat_t *	sb,
		int	mask);

#define ded_blocks(sb) fileblocks(sb)

extern	char	*type_uid2s(
		Stat_t *	s);

extern	int	has_extended_acl(
		RING *	gbl,
		int	x);

/* *** "dlog.c" *** */
extern	void	dlog_read(
		char *	name);

extern	char	*dlog_open(
		char *	name,
		int	argc,
		char **	argv);

extern	void	dlog_reopen(void);

extern	void	dlog_close(void);

extern	void	dlog_exit(
		int	code);

extern	int	dlog_char(
		RING *	gbl,
		int *	count_,
		int	begin);

extern	void	dlog_prompt(
		RING *	gbl,
		char *	prompt,
		int	row);

#ifdef	SIGWINCH
extern	void	dlog_resize(void);
#endif

extern	char *	dlog_string(
		RING *	gbl,
		char *	prompt,
		int	row,
		DYN **	result,
		DYN **	inflag,
		HIST **	history,
		int	fast_q,
		int	wrap_len);

extern	void	dlog_elapsed(void);

extern	void	dlog_flush(void);

extern	void	dlog_name(
		char *	name);

extern	void	dlog_comment(
			char * fmt,
			...
		);

/* *** "ftree.c" *** */
extern	void	ft_insert(
		char *	path);

extern	void	ft_remove(
		char *	path,
		int	links,
		int	dots);

extern	void	ft_purge(
		RING *	gbl);

extern	void	ft_rename(
		char *	oldname,
		char *	newname);

extern	void	ft_read(
		char *	first,
		char *	home_dir);

#ifdef	SIGWINCH
extern	int	ft_resize(void);
#endif
extern	RING *	ft_view(
		RING *	gbl,
		char *	path,
		int *	cmd);

extern	int	ft_scan(
		RING *	gbl,
		int	node,
		int	levels,
		int	base);

extern	void	ft_set_levels (
		int	row,
		int	levels);

extern	int	ft_stat(
		char *	name,
		char *	leaf);

extern	void	ft_write(void);

/* *** "history.c" *** */
extern	HIST	*cmd_history;

extern	void	put_history(
		HIST **	table,
		char *	text);

extern	char *	get_history(
		HIST *	table,
		int	age);

extern	void	show_history(
		RING *	gbl,
		int	depth);

/* *** "restat.c" *** */
extern	void	restat(
		RING *	gbl,
		int	group);

extern	void	restat_l(
		RING *	gbl);

extern	void	restat_W(
		RING *	gbl);

/* *** "inline.c" *** */
extern	int	dyn_trim1(
		DYN *	p);

extern	void	hide_inline(
		int	flag);

extern	int	edit_inline(
		int	flag);

extern	int	up_inline(void);
extern	int	down_inline(void);

extern	int	get_inline(
		RING *	gbl,
		int	c,
		int	cmd);

#define	ReplayStart(chr)	(void)get_inline(gbl,chr,chr)
#define	ReplayFinish()		(void)get_inline(gbl,EOS,C_DONE)
#define	ReplayInit(chr)		      get_inline(gbl,chr,C_INIT)
#define	ReplayTopC(chr)		(void)get_inline(gbl,chr,C_TOPC)
#define	ReplayEndC()		      get_inline(gbl,EOS,C_ENDC)
#define	ReplayFind(chr)		(void)get_inline(gbl,chr,C_FIND)
#define	ReplayTrim()		(void)get_inline(gbl,EOS,C_TRIM)
#define	ReplayQuit()		(void)get_inline(gbl,EOS,C_QUIT)
#define	ReplayChar()		      get_inline(gbl,EOS,C_NEXT)

extern	DYN **	inline_text(void);
extern	HIST **	inline_hist(void);
#ifdef	DEBUG
extern	int	inline_hidden(void);
#endif

/* *** "showpath.c" *** */
extern	void	showpath(
		char *	path,
		int	level,
		int	base,
		int	margin);

/* *** "sortset.c" *** */
extern	char	sortc[];

extern	int	sortset(
		RING *	gbl,
		int	ord,
		int	opt);

extern	int	sortget(
		RING *	gbl,
		int	c);

#endif	/* DED_H */
