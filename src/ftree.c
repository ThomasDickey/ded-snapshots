#ifndef	NO_IDENT
static	char	Id[] = "$Id: ftree.c,v 12.32 1994/08/12 21:15:18 tom Exp $";
#endif

/*
 * Author:	T.E.Dickey
 * Created:	02 Sep 1987
 * Modified:
 *		23 Jul 1994, implemented repeat-count for 'V'.
 *		17 Jul 1994, implemented left/right scrolling.
 *		16 Jul 1994, redesign display, taking advantage of Sys5-curses
 *		29 Jun 1994, changes for display-resizing.
 *		02 Jun 1994, allow environment variable DED_TREE to set full
 *			     ".ftree" path.
 *		24 May 1994, allow leaves with non-printing characters.
 *		19 Nov 1993, added mouse-support.
 *		18 Nov 1993, modified to make "^" command toggle, and to make
 *			     up/down row commands simulate scrolling.
 *		29 Oct 1993, ifdef-ident, port to HP/UX
 *		28 Sep 1993, gcc warnings
 *		23 Jul 1992, in '~' command, do chdir to resolve symbolic links
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		30 Mar 1992, in 'ft_write()', copy strings to temp-buffer first
 *			     to avoid having to make lots of writes.
 *		17 Feb 1992, fix 'V' toggle, 'R' command when no-children.
 *			     Make '=' command work.
 *		18 Oct 1991, converted to ANSI
 *		16 Jul 1991, broke out guts of 'ft_insert()' to allow special
 *			     call from 'ft_view()', allowing it to recover from
 *			     bizarre case on SunOS where real (mounted)
 *			     directory was masked by a symbolic link.
 *		15 Jul 1991, added guard in case 'ft_insert()' fails to insert
 *			     current-dir
 *		28 Jun 1991, lint (apollo sr10.3)
 *		04 Jun 1991, corrected logic in q/Q/F/B commands which caused
 *			     program to hang when call on 'fd_ring()' failed to
 *			     find a path.
 *		31 May 1991, modified interface to 'showpath()' so that
 *			     'fd_slow()' will highlight subtree scanned by
 *			     'ft_scan()'. increased width of number-tag.
 *		16 May 1991, mods to accommodate apollo sr10.3
 *		18 Apr 1991, added ':' command to simplify jumps to specific
 *			     point
 *		27 Aug 1990, modified 'ft_read()' and 'ft_write()' to try to
 *			     recover from missing/corrupt ".ftree" file.
 *		23 May 1990, set initial "=>" marker in 'ft_view()' so that
 *			     caller can give an arbitrary 'path' value (e.g.,
 *			     when error-recovering from 'E' command).  Also,
 *			     pass CTL(E) command out of 'ft_view()' like 'E'
 *			     command.  Modified interface to 'dedring()'
 *		06 Mar 1990, lint
 *		01 Feb 1990, use 'showpath()' to handle long pathname-display
 *		13 Nov 1989, added some error recovery in 'ft_read()' against
 *			     corrupted ".ftree" file (i.e., missing string-heap)
 *		16 Oct 1989, re-simplified 'ft_stat()' (don't need lstat)
 *		16 Oct 1989, altered 'ft_stat()' so we don't look at
 *			     stat.st_ino (don't assume inode is positive!), and
 *			     to use lstat/stat combination
 *		06 Sep 1989, use ACC_PTYPES rather than inline def's
 *		25 Aug 1989, use 'wrepaint()' rather than savewin/unsavewin
 *		05 Jun 1989, revised/simplified code which inserts logical links
 *			     by making this part of 'ft_insert()'
 *		14 Mar 1989, interface to 'dlog'.
 *		07 Mar 1989, forgot that 'strchr()' will also search for a null.
 *		23 Jan 1989, to support '&' toggle and '~' home-command
 *		20 Jan 1989, to support "-t" option of DED.
 *		09 Sep 1988, adjusted '=' to permit "~" expressions.
 *		07 Sep 1988, fixes to q/Q.
 *		02 Sep 1988, use 'rcs_dir()' and 'sccs_dir()'
 *		03 Aug 1988, use 'dedsigs()' in 'R' command to permit interrupt.
 *		27 Jul 1988, reversed initial sense of 'I'.  Corrected display
 *			     of 'W' in case file could not be written, etc.
 *		25 Jul 1988, use repeat-count on 'R' to recur that many levels.
 *		05 Jul 1988, recoded 'ft_rename()' so children are moved too.
 *		29 Jun 1988, added temporary '=' command for testing rename.
 *		27 Jun 1988, recoded 'ft_purge()' so it isn't recursive.
 *		07 Jun 1988, added CTL(K) command.
 *		06 Jun 1988, use 'gethome()' for ".ftree" location.
 *		01 Jun 1988, added SCCS_DIR environment variable.
 *		16 May 1988, added 'U' command.
 *		13 May 1988, use 'txtalloc()' in 'ft_read()' -- should be
 *			     eventually cheaper: less memory.  Treat RCS
 *			     directories same as sccs-directories whith 'Z'.
 *			     Added 'I' command.
 *		11 May 1988, added 'ft_rename' entrypoint.
 *		09 May 1988, do chdir before interpreting 'readlink()'.
 *		06 May 1988, added 'W' command.  Also, in cursor movement, make
 *			     newline force cursor rightwards.  Portable regex.
 *		05 May 1988, added 'Q' command.
 *		03 May 1988, broke out 'ft_stat()'. to use in '@' command.
 *			     Added 'J', 'K' level-independent up/down.
 *		02 May 1988, added '^' command to 'ft_view()'.  Adjusted 'q'
 *			     so that it quits lists via 'dedring()'.
 *		28 Apr 1988, integrated with 'ded' via 'dedring()' module.
 *		26 Apr 1988, adjusted 'p' command so we position before changes.
 *		24 Mar 1988, moved under 'ded' directory to begin changes for
 *			     bsd4.2
 *		09 Dec 1987, began recoding to provide for long (BSD-style)
 *			     names (still does not omit duplication of strings)
 *		02 Dec 1987, port to Apollo (leading "//" on pathnames)
 *
 * Function:	This module performs functions supporting a file-tree display.
 *		We show the names of directories in tree-form.
 *
 * Entrypoints:
 *		ft_read
 *		ft_write
 *		ft_insert
 *		ft_remove
 *		ft_purge
 *		ft_rename
 *		ft_scan
 *		ft_stat
 *		ft_view
 *
 * Configure:	DEBUG	- dump a logfile in readable form at the end
 *			  (i.e., when calling 'ft_read()' or 'ft_write()').
 */

#define	ACC_PTYPES
#define	DIR_PTYPES
#include	"ded.h"
#include	"rcsdefs.h"
#include	"sccsdefs.h"

#include	<fcntl.h>

#define	Null	(char *)0	/* some NULL's are simply 0 */

#define	min(a,b) ((a) < (b) ? (a) : (b)) /* patch */
#define	max(a,b) ((a) > (b) ? (a) : (b)) /* patch */

#ifdef	apollo
#define	TOP	2
#define	ROOT	"//"
#else
#define	TOP	1
#define	ROOT	"/"
#endif

#define LEN_MARK	2	/* length of mark at beginning of line */
#define	PATH_ROW	0	/* line to show "path:" on */
#define	FLAG_ROW	1	/* line to show "flags:" on */
#define	CMDS_ROW	2	/* line to prompt for commands on */
#define	LOSHOW	(4)		/* first line to show directory name on */
#define	TOSHOW	(LINES-LOSHOW)	/* lines to show on a screen */

/* these bit-flags are stored in the .ftree file */
#define	NORMAL	0
#define	MARKED	1
#define	VISITED	2
#define	NOSCCS	4	/* set to disable viewing sccs-directories */
#define	NOVIEW	8	/* set to disable viewing of a tree */
#define	LINKED	16	/* set to show link-to-directory */

#define	MAXLVL	999

#define	PRE(j,c)	(ftree[j].f_name[0] == c)
#define	ALL_SHOW(j)	(all_show || !(PRE(j,'.') || PRE(j,'$')))
#define	zSCCS(j)	(ftree[j].f_mark & NOSCCS)
#define	zMARK(j)	(ftree[j].f_mark & MARKED)
#define	zHIDE(j)	(ftree[j].f_mark & NOVIEW)
#define	zLINK(j)	(ftree[j].f_mark & LINKED)
#define	zROOT(j)	(ftree[j].f_root)

/************************************************************************
 *	Public data							*
 ************************************************************************/

/************************************************************************
 *	Private data							*
 ************************************************************************/

typedef	struct	{
	int	f_root;			/* array-index of entry's parent */
	int	f_mark;			/* removal/visited flags	*/
	char	*f_name;		/* name of directory		*/
	} FTREE;

#define	def_doalloc	FTREE_alloc
	/*ARGSUSED*/
	def_DOALLOC(FTREE)

private	int	is_sccs _one(int,node);
private	int	fd_show _one(int,node);
private	int	limits(
			_arx(int,	base)
			_ar1(int,	row));

static	char	FDname[MAXPATHLEN];	/* name of user's database	*/
static	time_t	FDtime;			/* time: last modified ".ftree"	*/
static	unsigned FDsize;		/* current sizeof(ftree[])	*/
static	int	FDdiff,			/* number of changes made	*/
		FDlast,			/* last used-entry in ftree[]	*/
		cant_W,			/* TRUE if last ft_write failed	*/
		showbase,		/* base of current display	*/
		shifted,		/* amount of right-shifting	*/
		showlast,		/* last line in current display	*/
		showdiff = -1,		/* controls re-display		*/
		all_show = TRUE,	/* TRUE to suppress '.' files	*/
		out_of_sight = TRUE,	/* TRUE to suppress search	*/
		savesccs,		/* original state of 'showsccs'	*/
		showsccs = TRUE;	/* control display of 'sccs'	*/
static	char	zero[] = ROOT,
		*caller_top,		/* caller's current directory	*/
		*viewer_top,		/* viewer's current directory	*/
		*gap = zero + (TOP-1);
static	FTREE	*ftree;			/* array of database entries	*/

/************************************************************************
 *	Database Manipulation						*
 ************************************************************************/

	/*ARGSUSED*/
private	void	my_dedmsg(
	_ARX(RING *,	gbl)
	_AR1(char *,	msg)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	msg)
{
	waitmsg(msg);
#define	dedmsg	my_dedmsg /* ...so we don't call 'showC' from this module */
}

private	int	ok_rename(
	_ARX(char *,	oldname)
	_AR1(char *,	newname)
		)
	_DCL(char *,	oldname)
	_DCL(char *,	newname)
{
	if (strcmp(oldname, newname)) {
		dlog_comment("rename \"%s\" (name=%s)\n", newname, oldname);
		if (rename(oldname, newname) >= 0)
			return TRUE;
		wait_warn(newname);
	}
	return FALSE;
}

/*
 * Show count while doing things which may be time-consuming.
 */
private	void	fd_slow(
	_ARX(int,	count)
	_ARX(int,	base)
	_AR1(char *,	pathname)
		)
	_DCL(int,	count)
	_DCL(int,	base)
	_DCL(char *,	pathname)
{
	static
		time_t	last;
		time_t	this	= time((time_t *)0);
	int	y,x;

	if ((count == 0) || (last != this)) {
		getyx(stdscr,y,x);
		move(PATH_ROW,0);
		PRINTW("%4d: ", count);
		showpath(pathname, 999, base, 0);
		clrtoeol();
		refresh();
		move(y,x);
	} else
		last = this;
}

/*
 * Ensure that the database has allocated enough space for the current entry.
 */
private	void	fd_alloc(_AR0)
{
	if (FDlast >= FDsize) {
	register int	size = FDsize;
		FDsize += FDlast + 2;
		ftree = DOALLOC(ftree,FTREE,FDsize);
		while (size < FDsize) {
			ftree[size].f_root =
			ftree[size].f_mark = 0;
			ftree[size].f_name = txtalloc("");
			size++;
		}
	}
}

/*
 * Add a path to the database.  As we add them, we insert in alphabetical
 * order to make it simple to display the tree.
 */
private	void	fd_add_path(
	_ARX(char *,	path)
	_AR1(char *,	validated)
		)
	_DCL(char *,	path)
	_DCL(char *,	validated)
{
	auto	int	last = 0, /* assume we start from root level */
			order,
			sort,
			this,
			check;
	auto	char	bfr[MAXPATHLEN];
	auto	Stat_t	sb;
	register int	j;
	register FTREE	*f;

	path = strcpy(bfr,path);
	if (!strcmp(path, zero)) {
		if (FDlast == 0) {	/* cwd is probably "/" */
			FDdiff++;
			FDlast++;
			fd_alloc();
		}
		return;
	}

	/* put this into the database, if it is not already */
	path += (TOP-1);
	while (*path == *gap) {
	char	*name = ++path,
		*next = strchr(name, (*gap));

		if (next != 0)
			*next = EOS;

		/* double-check link/directory here */
		if ((check = (is_subpath(validated, bfr) < 0) ) != 0) {
			if (lstat(bfr, &sb) < 0)
				/* patch: remove this & all children */
				break;
		}

		this = sort = 0;
		for (j = last+1; j <= FDlast; j++) {
			f = &ftree[j];
			if (f->f_root == last) {
				order = strcmp(f->f_name, name);
				if (order == 0) {
					this = j;
					break;
				} else if (order > 0) {
					sort = j;
					break;
				}
			} else if (f->f_root < last) {
				sort = j;
				break;
			}
		}

		if (!this) {		/* add a new entry */
			FDdiff++;
			FDlast++;
			fd_alloc();	/* make room for entry */
			if (sort) {	/* insert into the list */
				this = sort;
				for (j = FDlast; j > this; j--) {
					ftree[j] = ftree[j-1];
					if (ftree[j].f_root >= this)
						ftree[j].f_root++;
				}
			} else		/* append on the end */
				this = FDlast;

			ftree[this].f_root = last;
			ftree[this].f_mark &= ~(MARKED | LINKED | NOVIEW);
			ftree[this].f_name = txtalloc(name);
			if (!showsccs && is_sccs(this))
				ftree[this].f_mark |= NOSCCS;
		}

		last = this;
		ftree[last].f_mark &= ~(MARKED | LINKED);

#ifdef	S_IFLNK
		if (check) {
			if (isLINK(sb.st_mode)) {
				ftree[last].f_mark |= LINKED;
				/* patch: remove all children */
				break;
			} else if (!isDIR(sb.st_mode)) {
				break;
			}
		}
#endif	/* S_IFLNK */

		if (next != 0) {
			*next = *gap; /* restore the one we knocked out */
			path  = next;
		} else
			break;
	}
}

/*
 * Perform a search through the database for a selected directory name.
 */
private	int	fd_find (
	_ARX(char *,	buffer)
	_ARX(int,	cmd)
	_AR1(int,	old)
		)
	_DCL(char *,	buffer)
	_DCL(int,	cmd)
	_DCL(int,	old)
{
	static	RING *	gbl;		/* dummy for REGEX stuff	*/
	static	int	next = 0;	/* last-direction		*/
	static	char	pattern[MAXPATHLEN];
	static	REGEX_T	expr;		/* regex-state/output		*/
	static	int	ok_expr;

	register int	snxt,
			new = old,
			skip,
			looped = 0;

	if (cmd == '?' || cmd == '/') {
		if (strchr(buffer, (*gap))) {
			waitmsg("\"/\" not allowed in search-path");
			return(-1);	/* we don't search full-paths */
		}
		if (*buffer)
			(void)strcpy(pattern,buffer);
		snxt =
		next = (cmd == '/') ? 1 : -1;
	} else
		snxt = (cmd == 'n') ? next : -next;

	if (!*pattern && strchr("?/nN", cmd)) {
		waitmsg("No previous regular expression");
		return(-1);
	}

	if (ok_expr)
		OLD_REGEX(expr);
	if ((ok_expr = NEW_REGEX(expr,pattern)) != 0) {
		do {
			if (looped++ && (new == old)) { beep();	return(-1); }
			else if ((new += snxt) < 0)		new = FDlast;
			else if (new > FDlast)			new = 0;
			skip = (out_of_sight && !fd_show(new));
		} while (skip || !GOT_REGEX(expr, ftree[new].f_name));
		return(new);
	}
	BAD_REGEX(expr);
	return (-1);
}

/*
 * Returns the hierarchical level of a given node
 */
private	int	fd_level (
	_AR1(int,	this))
	_DCL(int,	this)
{
	register int level = this ? 1 : 0;

	while ((this = ftree[this].f_root) != 0)
		level++;
	return(level);
}

private	int	node2col(
	_ARX(int,	node)
	_AR1(int,	level)
		)
	_DCL(int,	node)
	_DCL(int,	level)
{
	register int k = fd_level(node);

	if (level < k) k = level;
	k = (((k-shifted) * BAR_WIDTH) + LEN_MARK);
	return min(k, COLS-1);
}

private	int	node2row (
	_AR1(int,	node))
	_DCL(int,	node)
{
	register int j, row;

	for (j = showbase, row = LOSHOW-1; j <= showlast; j++) {
		if (fd_show(j))	row++;
		if (j == node)	break;
	}
	return (row);
}

#ifndef	NO_XTERM_MOUSE
private	int	row2node (
	_AR1(int,	row))
	_DCL(int,	row)
{
	register int node = showbase;
	register int j;

	if (node < showlast) {
		for (j = LOSHOW; j < row; j++) {
			node++;
			while (!fd_show(node)) {
				if (node+1 >= showlast) {
					/* back off! */
					while (node > showbase) {
						if (fd_show(node))
							break;
						node--;
					}
					break;
				}
				node++;
			}
		}
	}
	return node;
}
#endif

/*
 * Compute a pathname for a given node
 */
private	char *	fd_path(
	_ARX(char *,	bfr)
	_AR1(int,	node)
		)
	_DCL(char *,	bfr)
	_DCL(int,	node)
{
	char	tmp[MAXPATHLEN];
		*bfr = EOS;

	do {
		(void)strcpy(tmp, bfr);
		(void)strcat(strcat(strcpy(bfr, gap), ftree[node].f_name), tmp);
	}
	while ((node = ftree[node].f_root) != 0)
		;
#if	(TOP > 1)
	(void)strcpy(tmp, bfr);
	(void)strcpy(bfr, zero+1);
	(void)strcat(bfr, tmp);
#endif
	return(bfr);
}

/*
 * Returns true iff the node should be displayed
 */
private	int	fd_show (
	_AR1(int,	node))
	_DCL(int,	node)
{
	if (zSCCS(node))
		return(FALSE);
	if (!ALL_SHOW(node))
		return(FALSE);
	while ((node = ftree[node].f_root) != 0) {
		if (zHIDE(node))
			return(FALSE);
		if (!ALL_SHOW(node))
			return(FALSE);
	}
	return(TRUE);
}

/************************************************************************
 *	Module Entrypoints						*
 ************************************************************************/

/*
 * Add a path to the database.  As we add them, we insert in alphabetical
 * order to make it simple to display the tree.
 */
public	void	ft_insert (
	_AR1(char *,	path))
	_DCL(char *,	path)
{
	auto	char	bfr[MAXPATHLEN];

	abspath(path = strcpy(bfr,path));
	fd_add_path(bfr, zero);
}

/*
 * Locate the node corresponding to a particular path.
 */
private	int	do_find (
	_AR1(char *,	path))
	_DCL(char *,	path)
{
	char	bfr[MAXPATHLEN];
	register int j, this, last = 0;

	abspath(path = strcpy(bfr,path));
	if (!strcmp(path,zero))
		return(0);

	path += (TOP-1);
	while (*path == *gap) {
	char	*name = ++path,
		*next = strchr(path, (*gap));
		if (next) *next = EOS;
		this = 0;
		for (j = last+1; j <= FDlast; j++) {
			if (ftree[j].f_root == last) {
				if (!strcmp(ftree[j].f_name, name)) {
					this = j;
					break;
				}
			}
		}
		if (next) {
			*next = *gap;
			path = next;
		}
		if (this) {
			last = this;
		} else {
			last = -1;
			break;
		}
	}
	return(last);
}

/*
 * Mark nodes below a given path in the database for removal, unless they
 * are added back before the database is written out.  This is used to update
 * the database from 'ded' when (re)reading a directory.  The argument must
 * be a directory name.
 *
 * The 'all' argument is used so that we needn't resolve symbolic links to
 * retain them in a directory-scan.  If 'all' is true, then we will purge
 * symbolic links as well.
 */
public	void	ft_remove(
	_ARX(char *,	path)
	_AR1(int,	all)
		)
	_DCL(char *,	path)
	_DCL(int,	all)
{
	int	last = do_find(path);
	register int j;

	if (last >= 0) {
		for (j = last+1; j <= FDlast; j++) {
		register FTREE *f = &ftree[j];
			if (f->f_root == last) {
#ifdef	S_IFLNK
				if (all || !(f->f_mark & LINKED))
#endif	/* S_IFLNK */
					f->f_mark |= MARKED;
			} else if (f->f_root < last)
				break;
		}
	}
}

/*
 * Scan the database for items which have been marked for removal.  When each
 * is found, it, and all of its dependents are purged from the database (i.e.,
 * the database is packed, leaving no trace of the removed entry).
 */
public	void	ft_purge (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	register int j, k, adj;
	int	changed	= 0;

	for (j = 1; j <= FDlast; j++) {	/* scan for things to purge */
		if (!zMARK(j)) 	continue;
		for (k = j; (k <= FDlast) && (zMARK(k) || zROOT(k) >= j); k++);
		adj = k - j;
		while (k <= FDlast) {
			if (zROOT(k) >= j)	zROOT(k) -= adj;
			ftree[k-adj] = ftree[k];
			k++;
		}
		FDlast -= adj;
		changed++;
		j--;			/* re-start scan */
	}
	FDdiff += changed;

	if (changed) {			/* re-insert ring */
		char	tmp[BUFSIZ], *s;

		(void)strcpy(tmp, gbl->new_wd);
		for (j = 1; (s = ring_path(gbl, j)) != NULL; j++) {
			ft_insert(s);
			if (!strcmp(s,tmp))
				break;
		}
	}
}

/*
 * Rename a directory or link.
 */
public	void	ft_rename(
	_ARX(char *,	old)
	_AR1(char *,	new)
		)
	_DCL(char *,	old)
	_DCL(char *,	new)
{
	register int	j, k;
	int	oldloc, newloc, base, len1, len2, chop, count;

	if (do_find(new) >= 0) {
		beep();
		return;
	}
	ft_insert(new);
	newloc = do_find(new);
	oldloc = do_find(old);
	if (oldloc < 0)
		return;

	if (oldloc > newloc) {
		chop =
		base = newloc + 1;
		len1 = oldloc - base;
		len2 = len1 + 1;
		for (j = oldloc+1; j <= FDlast; j++) {
			if (zROOT(j) >= oldloc)
				len2++;
			else
				break;
		}
	} else {		/* oldloc < newloc */
		base = oldloc;
		chop = newloc;
		len2 = newloc + 1 - oldloc;
		len1 = 1;
		for (j = oldloc+1; j < FDlast; j++) {
			if (zROOT(j) >= oldloc) {
				chop--;
				len1++;
			} else
				break;
		}
	}

	/* interchange segments to make children follow renaming */
	count = j = 0;
	while (count < len2) {
		int	first = j;
		FTREE	tmp;
		tmp = ftree[base+j];
		for (;;) {
			k = j + len1;
			if (k >= len2) k -= len2;
			count++;
			if (k == first) {
				ftree[base + (j++)] = tmp;
				break;
			}
			ftree[base + j] = ftree[base + k];
			j = k;
		}
	}

	/* ...and readjust the root-pointers */
	for (j = base; j <= FDlast; j++) {
		k = zROOT(j);
		if (k >= base && k < base + len1)
			zROOT(j) += (len2 - len1);
		else if (k >= base + len1 && k < base + len2)
			zROOT(j) -= len1;
	}

	/* eliminate old entry */
	for (j = chop; j < FDlast; j++) {
		ftree[j] = ftree[j+1];
		if (zROOT(j) >= chop)	zROOT(j) -= 1;
	}
	FDlast--;
}


/* recover from corrupt .ftree file by initializing to empty-state */
private	int	ft_init (
	_AR1(char *,	msg))
	_DCL(char *,	msg)
{
	waitmsg(msg);
	FDtime = time((time_t *)0);
	FDlast = 0;
	fd_alloc();
	return (FALSE);
}

/* read from the .ftree file, testing for consistency in sizes */
private	int	ok_read(
	_ARX(int,	fid)
	_ARX(char *,	s)
	_ARX(LEN_READ,	ask)
	_AR1(char *,	msg)
		)
	_DCL(int,	fid)
	_DCL(char *,	s)
	_DCL(LEN_READ,	ask)
	_DCL(char *,	msg)
{
	LEN_READ	got = read(fid,s,ask);
	if (got != ask) {
		char	bfr[BUFSIZ];
		dlog_comment("%s (got %d, asked %d)", msg, got, ask);
		FORMAT(bfr, "%s \"%s\"", msg, FDname);
		return (ft_init(msg));
	}
	return (TRUE);
}

private	void	read_ftree (
	_AR1(char *,	the_file))
	_DCL(char *,	the_file)
{
	Stat_t		sb;
	register int	j;
	LEN_READ	vecsize;
	int		fid,
			size;

	/* read the current database */
	if ((fid = open(the_file, O_RDONLY, 0)) != 0) {
		if (stat_file(the_file, &sb) < 0)
			return;
		if (sb.st_mtime <= FDtime) {
			(void)close(fid);
			return;
		} else if (FDtime) {
			showdiff = -1;
		}
		FDtime = sb.st_mtime;
		size   = sb.st_size  - sizeof(FDlast);

		/* (1) vector-size */
		if (!ok_read(fid,
				(char *)&vecsize, sizeof(vecsize),
				"size"))
			return;
		if ((size / sizeof(FTREE)) < vecsize) {
			(void)ft_init("? size error");
			return;
		}

		/* (2) vector-contents */
		if (vecsize > FDlast)
			FDlast = vecsize;
		fd_alloc();
		vecsize++;		/* account for 0'th node */
		vecsize *= sizeof(FTREE);
		if (!ok_read(fid,
				(char *)ftree, (LEN_READ)vecsize,
				"read"))
			return;

		/* (3) string-heap */
		if ((size -= vecsize) > 0) {
		char	*heap = doalloc(Null, (unsigned)(size+1));
		register char *s = heap;
			if (!ok_read(fid,
					heap, (LEN_READ)size,
					"heap"))
				return;
			s[size] = EOS;
			for (j = 0; j <= FDlast; j++) {
				ftree[j].f_name = txtalloc(s);
				s += strlen(s) + 1;
			}
			dofree(heap);
		} else {
			FDlast = 0;	/* try to recover */
			ftree[0].f_name = txtalloc("");
		}
		(void)close(fid);
#ifdef	DEBUG
		ft_dump("read");
#endif
	}
}

/*
 * Initialize this module, by reading the file-tree database from the user's
 * home directory.
 *
 * The database file is stored:
 *	1) the number of entries in the vector
 *	2) the vector (with name-pointers adjusted to integer indices)
 *	3) the string-heap
 */
public	void	ft_read(
	_ARX(char *,	first)	/* => string defining the initial directory */
	_AR1(char *,	tree_name)
		)
	_DCL(char *,	first)
	_DCL(char *,	tree_name)
{
	register int	j;

	read_ftree(strcpy(FDname, tree_name));
	FDdiff = 0;

	/* append the current directory to the list */
	ft_insert(first ? first : ".");

	/* inherit sense of 'Z' toggle from database */
	showsccs = TRUE;
	for (j = 0; j <= FDlast; j++) {
		if (zSCCS(j)) {
			showsccs = FALSE;
			break;
		}
	}
	savesccs = showsccs;
}

/*
 * For a given node, compute the strings that are appended in 'ft_show()'
 */
private	void	fd_annotate(
	_ARX(int,	node)
	_AR1(char *,	buffer)
		)
	_DCL(int,	node)
	_DCL(char *,	buffer)
{
	(void)strcpy(buffer, zLINK(node) ? "@" : gap);
#ifdef	apollo
	if (node == 0)
		(void)strcat(buffer, zero+1);
#endif
	if (zHIDE(node))
		(void)strcat(buffer, " ...");
	if (zMARK(node))
		(void)strcat(buffer, " (Purge)");
}

/*
 * Display the file-tree.  To help keep paths from growing arbitrarily, show
 * the common (at least to the current screen) part of the path in the
 * status line, moving to the common nodes only if the user directs so.
 */
private	int	ft_show(
	_ARX(RING *,	gbl)
	_ARX(char *,	path)
	_ARX(char *,	home)
	_ARX(int,	node)
	_AR1(int,	level)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(char *,	home)
	_DCL(int,	node)
	_DCL(int,	level)
{
	static	char	*fmt = "%.*s";
	register int j, k;
	auto	int	row,
			count,
			limit;
	auto	char	*marker,
			bfr[BUFSIZ],
			end[BUFSIZ];

	move(PATH_ROW,0);
	row = LOSHOW;
	node = limits(showbase, node);
	k = FDdiff || (savesccs != showsccs);
	PRINTW("path: ");
	showpath(path, level, -1, k ? 5 : 0);
	clrtoeol();
	if (k) {	/* show W-command if we have pending diffs */
		move(PATH_ROW, COLS-5);
		PRINTW(" (W%s)", cant_W ? "?" : "");
	}

	/* This normally fits into 80 columns! */
	move(FLAG_ROW,0);
	PRINTW("flags:");
	if (!all_show)		PRINTW(" & ('.' names)");
	if (out_of_sight) 	PRINTW(" I (inhibit search)");
	if (!showsccs) 		PRINTW(" Z (SCCS/RCS)");
	clrtoeol();
	FORMAT(bfr, "  node: %d of %d ", node+1, FDlast+1);
	if (strlen(bfr) < COLS) {
		move(FLAG_ROW, (int)(COLS - strlen(bfr)));
		addstr(bfr);
	}

	/* clear the command-line when we aren't using it */
	move(CMDS_ROW,0);
	clrtoeol();

	/* make a line between the command-section and the tree-display */
	move(LOSHOW-1,0);
	for (j = 0; j < COLS; j++) {
		addch(bar_hline[1]);
	}

	/* Adjust left/right shift to keep the cursor visible.  If we're at
	 * rightmost position, try to make the filename visible.  If the node
	 * name is itself too long, limit it to allow the navigation bars to
	 * show.  In all cases, we show the 2-column prefix that indicates to
	 * which filelist the node corresponds, if any.
	 */
	(void)ded2string(gbl, bfr, sizeof(bfr), ftree[node].f_name, FALSE);
	fd_annotate(node, end);
	k = strlen(bfr) + strlen(end);
	j = k + (level * BAR_WIDTH) + LEN_MARK;
	if (j >= COLS) {	/* not all of the line will be visible */
		int	value;

		limit  = (COLS - 1 - LEN_MARK);

		if (level+1 >= fd_level(node)) { /* make the name visible */
			k     = min(k, limit);
			value = fd_level(node) - ((limit - k) / BAR_WIDTH);
		} else {			/* center the cursor */
			value = level - (limit + BAR_WIDTH - 1)
					/ (2 * BAR_WIDTH);
		}
		value = max(0, value);

		if (value != shifted) {
			showdiff = -1;
			shifted  = value;
		}
	} else if (shifted) {
		showdiff = -1;
		shifted  = 0;
	}

	/* update the tree display if we've moved */
	if (showdiff != FDdiff) {

		for (j = showbase; j <= showlast; j++) {
			if (!fd_show(j))
				continue;
			move(row++,0);

			marker = strcmp(fd_path(bfr, j), home) ? "  " : "=>";
			if (*marker == ' ' && (ring_get(bfr) != 0))
				marker = "* ";
			PRINTW("%s", marker);

			limit = COLS - 1 - LEN_MARK;
			count = fd_level(j) - shifted;
			if (count > 0) {
				for (k = count; k > 0; k--) {
					int	len   = min(BAR_WIDTH, limit);
					chtype	*fill = (k != 1)
							? bar_space
							: bar_hline;
#if SYS5_CURSES
					addchnstr(fill, len);
#else
					PRINTW(fmt, len, fill);
#endif
					move(row-1, LEN_MARK + (count+1-k) * BAR_WIDTH);
					limit -= len;
				}
				count = 0;
			} else {
				count *= (-BAR_WIDTH);
			}

			if (limit > 0) {

				(void)ded2string(gbl, bfr, sizeof(bfr), ftree[j].f_name, FALSE);
				fd_annotate(j, end);

				if (strlen(bfr) > count) {
					if (zMARK(j))
						standout();
					PRINTW(fmt, limit, bfr + count);
					if (zMARK(j))
						standend();
					limit -= (strlen(bfr) - count);
					count = 0;
				} else {
					count -= strlen(bfr);
				}

				if (limit > 0
				 && strlen(end) > count) {
					PRINTW(fmt, limit, end + count);
				}
			}
			clrtoeol();
		}
		clrtobot();
	}
	move(node2row(node), node2col(node,level));
	return(node);
}

/*
 * Set base/last limits for the current screen
 */
private	int	limits(
	_ARX(int,	base)
	_AR1(int,	row)
		)
	_DCL(int,	base)
	_DCL(int,	row)
{
	register int j;
	int	len = 0;

	showbase = fd_show(base) ? base : -1;
	showlast = -1;

	/* determine screen extent, starting at nominal basepoint */
	for (j = base; j <= FDlast; j++) {
		if (fd_show(j)) {
			if (showbase < 0) showbase = j;
			showlast = j;
			if (++len == TOSHOW) break;
		}
	}

	/* keep at least one line on screen (e.g., if a line was deleted) */
	if (showlast < 0) {
		for (j = base-1; j >= 0; j--) {
			if (fd_show(j)) {
				showbase = showlast = j;
				break;
			}
		}
	}

	if (row > showlast)	row = showlast;
	if (row < showbase)	row = showbase;
	return(row);
}

private	int	fd_fwd (
	_AR1(int,	num))
	_DCL(int,	num)
{
	while (num-- > 0) {
		if (showlast < FDlast) {
			showbase = showlast;
			showdiff = -1;
			(void)limits(showbase,showbase);
		} else
			break;	/* no sense in going further */
	}
	return(showlast);
}

private	int	fd_bak (
	_AR1(int,	num))
	_DCL(int,	num)
{
	while (num-- > 0) {
	register int j, len = 0, base = -1;
		for (j = showbase; j >= 0; j--) {
			if (fd_show(j)) {
				base = j;
				if (++len == TOSHOW) break;
			}
		}
		if (base >= 0) {
			showbase = base;
			showdiff = -1;
		} else
			break;	/* no sense in going further */
	}
	return(showbase);
}

/*
 * Toggles the state of the sccs-directory visibility, and coerces all nodes
 * to correspond to the new state.
 */
private	void	toggle_sccs(_AR0)
{
	register int j;

	showsccs = !showsccs;
	for (j = 1; j <= FDlast; j++) {
	register FTREE *f = &ftree[j];
		if (is_sccs(j)) {
			if (f->f_mark & NOSCCS) {
				if (showsccs) {
					f->f_mark ^= NOSCCS;
					showdiff = -1;
				}
			} else if (!showsccs) {
				f->f_mark ^= NOSCCS;
				showdiff = -1;
			}
		}
	}
}

private	void	markit(
	_ARX(int,	node)
	_ARX(int,	flag)
	_AR1(int,	on)
		)
	_DCL(int,	node)
	_DCL(int,	flag)
	_DCL(int,	on)
{
	register FTREE *f = &ftree[node];
	register int	old = f->f_mark;

	if (on)			f->f_mark |= flag;
	else			f->f_mark &= ~flag;
	if ((old & ~MARKED) != (f->f_mark & ~MARKED))
		FDdiff++;
}

/*
 * Set up for display of the given node.  If it is not visible, make it so.
 */
private	void	scroll_to (
	_AR1(int,	node))
	_DCL(int,	node)
{
	register int j;
	if (node < 0)
		node = 0;
	j = node;

	if (zSCCS(j))
		toggle_sccs();
	while ((j = ftree[j].f_root) != 0) {	/* ensure this is visible! */
		if (zHIDE(j))
			markit(j,NOVIEW,FALSE);
		if (zSCCS(j))
			toggle_sccs();
	}
	(void)limits(showbase,showbase);
	while (node > showlast) (void)fd_fwd(1);
	while (node < showbase) (void)fd_bak(1);
}

/*
 * Scroll to current ring-entry
 */
private	int	fd_ring(
	_ARX(RING *,	gbl)
	_ARX(char *,	path)
	_ARX(int *,	row_)
	_AR1(int *,	level_)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(int *,	row_)
	_DCL(int *,	level_)
{
	int	newrow;
	char	cwdpath[BUFSIZ];

	if ((newrow = do_find(strcpy(cwdpath, gbl->new_wd))) < 0) {
		/* path was deleted, put it back if it is really there */
		(void) ft_stat(cwdpath, cwdpath);
		newrow = do_find(cwdpath);
	}

	if (newrow >= 0) {
		*level_ = fd_level(newrow);
		scroll_to(newrow);
		(void)strcpy(path, cwdpath);
		showdiff = -1;		/* always refresh '*', '=>' marks */
		*row_ = newrow;
		return TRUE;
	}
	return FALSE;
}

private	int	uprow(
	_ARX(int,	node)
	_ARX(int,	count)
	_AR1(int,	level)
		)
	_DCL(int,	node)
	_DCL(int,	count)
	_DCL(int,	level)
{
	register int j, k = node;

	level++;
	for (j = node-1; j >= 0; j--) {
		if (fd_show(j) && fd_level(j) <= level) {
			k = j;
			if (--count <= 0)
				break;
		}
	}
	if (k != node) {
		while (showbase > k
		   &&  showbase > 0) {
			showbase--;
			showdiff = -1;
		}
	} else
		beep();
	return(k);
}

private	int	downrow(
	_ARX(int,	node)
	_ARX(int,	count)
	_AR1(int,	level)
		)
	_DCL(int,	node)
	_DCL(int,	count)
	_DCL(int,	level)
{
	register int j, k = node;

	level++;
	for (j = node+1; j <= FDlast; j++) {
		if (fd_show(j) && fd_level(j) <= level) {
			k = j;
			if (--count <= 0)
				break;
		}
	}
	if (k != node) {
		while (showlast < k
		    && showlast < FDlast) {
			showbase++;
			showdiff = -1;
			(void)limits(showbase,showbase);
		}
	} else
		beep();
	return(k);
}

/*
 * Returns TRUE iff the name of the node corresponds to a source-control
 * directory.
 */
private	int	is_sccs (
	_AR1(int,	node))
	_DCL(int,	node)
{
	register FTREE *f = &ftree[node];

	if (!strcmp(f->f_name, sccs_dir(Null,Null)))	return (TRUE);
	if (!strcmp(f->f_name, rcs_dir()))		return (TRUE);
#ifdef CVS_PATH
	if (!strcmp(f->f_name, "CVS"))			return (TRUE);
#endif
	return (FALSE);
}

/*
 * Update the cursor position within the tree, given the latest row/column
 * inputs, and repaint the display as needed.
 */
private	int	ft_update (
		_ARX(RING *,	gbl)
		_ARX(int,	row)
		_AR1(int *,	level)
			)
		_DCL(RING *,	gbl)
		_DCL(int,	row)
		_DCL(int *,	level)
{
	auto	int	c = fd_level(row);

	if (c < *level)
		*level = c;	/* loosely drag down level */
	return ft_show(gbl, fd_path(viewer_top,row), caller_top, row, *level);
}

/*
 * The window-resizing code is a little crude, since it must have access
 * to the row/lvl variables of 'ft_view()'.
 */
#ifdef	SIGWINCH
static	RING	*resize_gbl;
static	int	*resize_row;
static	int	*resize_lvl;

public	int	ft_resize(_AR0)
{
	if (tree_visible) {
		showdiff = -1;
		scroll_to(*resize_row);
		(void)ft_update(resize_gbl, *resize_row, resize_lvl);
		return TRUE;
	}
	return FALSE;
}
#endif

/*
 * Interactively display the directory tree and modify it:
 *	* The return value is used by the coroutine as the next command.
 *	* The argument is overwritten with the name of the new directory
 *	  for e/E commands.
 */
public	RING *	ft_view(
	_ARX(RING *,	gbl)
	_ARX(char *,	path) /* caller's current directory */
	_AR1(int *,	cmdp)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(int *,	cmdp)
{
	static	 DYN *	my_text;
	static	 HIST 	*JumpHistory, *FindHistory, *NameHistory;
	auto	 RING *	tmp;
	auto	 int	row,
			lvl,
			num,
			c;
	auto	 char	cwdpath[MAXPATHLEN];
	auto	 char	ignore;
	register int	j;
	register char	*s;

#ifdef	SIGWINCH	/* make the row/column visible to signal handler */
	caller_top = path;
	resize_gbl = gbl;
	resize_row = &row;
	resize_lvl = &lvl;
#endif
	viewer_top = cwdpath;
	*cmdp = 'E';	/* the most common return-value */

	/* Set initial position. This has to be done by assuming the 'path'
	 * argument is a true result from 'getwd' since the mount-table may
	 * be screwed up and a symbolic link may be hiding this path. */
	fd_add_path(strcpy(cwdpath, path), path);
	if ((row = do_find(cwdpath)) < 0) {
		waitmsg(cwdpath);
		return gbl;
	}
	lvl = fd_level(row);
	scroll_to(row);
	showdiff = -1;

	/* process commands */
	for (;;) {

		row = ft_update(gbl, row, &lvl);
		switch(c = dlog_char(gbl, &num, 1)) {
		/* Ordinary cursor movement */
		case ARO_LEFT:
		case '\b':
		case 'h':	if (lvl > 0) {
					lvl -= num;
					if (lvl < 0) lvl = 0;
				}
				else
					beep();
				break;
		case '\n':
				lvl = MAXLVL;
		case ARO_DOWN:
		case 'j':	row = downrow(row,num,lvl);	break;
		case ARO_UP:
		case 'k':	row = uprow(row,num,lvl);	break;
		case ARO_RIGHT:
		case 'l':	lvl += num;			break;

#ifndef	NO_XTERM_MOUSE
	case ARO_MOUSE:
			if (xt_mouse.released) {
				if (xt_mouse.button == 1
				 && xt_mouse.row >= LOSHOW-1) {
					j = BAR_WIDTH;
					row = row2node(xt_mouse.row);
					lvl = (xt_mouse.col - LEN_MARK + (j/2)) / j;
					if (lvl < 0)
						lvl = 0;
					if (xt_mouse.dbl_clik) {
						(void)ungetc('E',stdin);
					}
				} else {
					beep();
				}
			}
			break;
#endif
		case 'J':	row = downrow(row,num,MAXLVL);	break;
		case 'K':	row = uprow(row,num,MAXLVL);	break;

		case '^':	if (showbase != row) {
					showbase = row;
					showdiff = -1;
				} else if ((row - TOSHOW) > 0) {
					(void)uprow(row, TOSHOW - 1, MAXLVL);
					showdiff = -1;
				}
				break;

		case 'H':	row = showbase;			break;
		case 'L':	row = showlast;			break;

		/* middle-of-screen (complicated by Z,V modes) */
		case 'M':
			for (j = showbase, row = 0; j <= showlast; j++)
				if (fd_show(j))
					row++;
			row /= 2;
			for (j = showbase; ; j++) {
				if (fd_show(j))
					if (--row <= 0)
						break;
			}
			row = j;
			break;

		/* scrolling */
		case 'f':
			if (row < FDlast) {
				if (row < showlast) {
					row = showlast;
					num--;
				}
				row = fd_fwd(num);
			} else
				beep();
			break;
		case 'b':
			if (row > 0) {
				if (row > showbase) {
					row = showbase;
					num--;
				}
				row = fd_bak(num);
			} else
				beep();
			break;

		case ':':	/* jump to absolute line */
			move(CMDS_ROW,0);
			PRINTW("line: ");
			clrtoeol();

			dyn_init(&my_text,1);
			if (!(s = dlog_string(
					gbl,
					Null,
					&my_text,
					(DYN **)0,
					NO_HISTORY,
					EOS,
					MAXPATHLEN)))
				break;

			if (!strclean(s))
				break;
			if (!strcmp(s, "$"))
				c = FDlast+1;
			else if (sscanf(s, "%d%c", &c, &ignore) != 1)
				c = -1;

			if (c >= 0 && c <= FDlast+1) {
				if (c > 0)
					c--;
				row = c;
				lvl = fd_level(row);
				scroll_to(row);
			} else {
				dedmsg(gbl, "illegal line number");
			}
			break;

		case '/':
		case '?':
		case 'n':
		case 'N':
			*cwdpath = EOS;
		case '~':
		case '@':
			j = (c == '@' || c == '~');
			s = "";
			if (!isalpha(c)) {
				move(CMDS_ROW,0);
				PRINTW(j ? "jump: " : "find: ");
				clrtoeol();

				my_text = dyn_copy(my_text,
					(c == '~') ? "~" : cwdpath);
				if (!(s = dlog_string(
						gbl,
						Null,
						&my_text,
						(DYN **)0,
						j ? &JumpHistory : &FindHistory,
						EOS,
						MAXPATHLEN)))
					s = "";

				if (!*s && c != '@') {
					c = -1;
					beep();
				}
			}

			if (!j)
				c = fd_find(s,c,row);
			else if (!*s)
				c = 0;
			else {
				abspath(strcpy(cwdpath, s));
				if (chdir(cwdpath) < 0)
					c = -1;
				else if ((c = do_find(getwd(cwdpath))) < 0) {
					(void) ft_stat(cwdpath,cwdpath);
					c = do_find(cwdpath);
				}
				if (c < 0)	beep();
			}

			if (c >= 0) {
				row = c;
				lvl = fd_level(row);
				scroll_to(row);
			}
			break;

		case '=':	/* rename-function */
			if (chdir(fd_path(cwdpath,zROOT(row))) >= 0) {
				char	bfr[BUFSIZ];

				abspath(fd_path(cwdpath,row));
				move(node2row(row),node2col(row,MAXLVL));

				my_text = dyn_copy(my_text, ftree[row].f_name);
				if (!(s = dlog_string(
						gbl,
						Null,
						&my_text,
						(DYN **)0,
						&NameHistory,
						'=',
						MAXPATHLEN)))
					break;

				abspath(strcpy(bfr,s));

				if (ok_rename(cwdpath, bfr) ) {
					ring_rename(gbl, cwdpath, bfr);
					(void)strcpy(path, gbl->new_wd);
					ft_rename(cwdpath, bfr);
					scroll_to (row = do_find(bfr));
				}
				(void)chdir(gbl->new_wd);
			} else
				waitmsg(cwdpath);
			break;
		/* dump the current screen */
		case CTL('K'):
			deddump(gbl);
			break;

#define	SKIP_THIS(num)	dedring(gbl, fd_path(cwdpath, row), c, num, FALSE, Null)
#define	QUIT_THIS(num)	dedring(gbl, fd_path(cwdpath, row),'q',num, FALSE, Null)

		/* quit lists in directory-ring */
		case 'Q':
		case 'q':
			j = 1;
			while (num-- > 0) {
				tmp = SKIP_THIS(1);
				if ((j = (tmp != 0)) != 0)
					gbl = tmp;
				else
					break;
				if (is_sccs(row) && (savesccs != showsccs))
					toggle_sccs();
			}
			while (!fd_ring(gbl, path, &row, &lvl)) {
				if ((tmp = QUIT_THIS(1)) != 0)
					gbl = tmp;
				else
					return gbl;
			}
			if (!j)
				return gbl;
			break;
		/* scroll through the directory-ring */
		case 'F':
		case 'B':
			if ((tmp = SKIP_THIS(num)) != 0)
				gbl = tmp;
			while (!fd_ring(gbl, path, &row, &lvl)) {
				if ((tmp = QUIT_THIS(1)) != 0)
					gbl = tmp;
				else
					return gbl;
			}
			break;

		/* Exit from this program (back to 'ded') */
		case CTL('E'):
		case 'E':
		case 'e':
			(void)fd_path(cwdpath, row);
#ifdef	S_IFLNK
			if (zLINK(row)) {
				char	bfr[BUFSIZ];
				int	len;

				(void)chdir(fd_path(bfr, ftree[row].f_root));
				len = readlink(cwdpath, bfr, sizeof(bfr));
				if (len <= 0) {
					beep();
					break;
				}
				bfr[len] = EOS;
				abspath(strcpy(cwdpath, bfr));
				(void)chdir(gbl->new_wd);
			}
#endif	/* S_IFLNK */
			if (access(cwdpath, R_OK | X_OK) < 0) {
				beep();
				break;
			}
			(void)strcpy(path, cwdpath);
			/* fall-thru to return */
		case 'D':
			*cmdp = c;	/* 'e', 'E' CTL(E) or 'D' -- only! */
			return gbl;

		/* Scan/delete nodes */
		case 'R':	if (zLINK(row))
					beep();
				else
					(void)ft_scan(gbl, row, num, fd_level(row));
				break;
		case '+':	while (num-- > 0) {
					markit(row,MARKED,TRUE);
					row = downrow(row,1,lvl);
				}
				break;
		case '-':	while (num-- > 0) {
					markit(row,MARKED,FALSE);
					row = downrow(row,1,lvl);
				}
				break;
		case '_':	for (j = 0; j <= FDlast; j++)
					markit(j,MARKED,FALSE);
				break;
		case 'p':	num = -1;
				for (j = FDlast; j > 0; j--)
					if (zMARK(j))
						num = 0;
					else if (num == 0)
						num = j;
				if (num >= 0) {
					ft_purge(gbl);
					while (num > 0) {
						if (!fd_show(num))
							num--;
						else
							break;
					}
					row = num;
					lvl = fd_level(row);
					scroll_to(row);
				}
				break;

		/* Screen refresh */
		case 'w':
#ifdef	apollo
				if (resizewin()) {
					dlog_comment("resizewin(%d,%d)\n",
						LINES, COLS);
					showdiff = -1;
					row = showbase;
					break;
				}
#endif	/* apollo */
				wrepaint(stdscr,0);
				break;

		/* Force dump */
		case 'W':
				ft_write();
				break;

		/*
		 * toggle flag which controls whether names beginning with
		 * '.' are shown
		 */
		case '&':
			all_show = !all_show;
			showdiff = -1;
			break;

		/*
		 * toggle flag which controls whether invisible directories
		 * can be found by a search.
		 */
		case 'I':
			out_of_sight = !out_of_sight;
			showdiff = -1;
			break;

#ifdef	apollo
		/* toggle flag showing Aegis/Unix names */
		case 'U':
			gbl->U_opt = !gbl->U_opt;
			showdiff = -1;
			break;
#endif	/* apollo */

		/* toggle flag which controls whether we show dependents */
		case 'V':
			ft_set_levels(row, num);
			break;

		/* toggle flag which controls whether we show 'sccs' */
		case 'Z':
			toggle_sccs();
			scroll_to(row);
			break;

		default:
			beep();
		}
	}
}

#ifdef	DEBUG
private	FILE *	ft_DUMP(_AR0)
{
	static	char	logname[BUFSIZ];
	return (fopen(strcat(strcpy(logname, gethome()), "/ftree.log"), "a+"));
}

/*
 * Dump a message to a log-file
 */
private	ft_dump2(
	_ARX(char *,	fmt)
	_AR1(char *,	msg)
		)
	_DCL(char *,	fmt)
	_DCL(char *,	msg)
{
	auto	FILE	*fp = ft_DUMP();
	if (fp) {
		FPRINTF(fp, "FTREE MSG: ");
		FPRINTF(fp, fmt, msg);
		FPRINTF(fp, "\n");
		FCLOSE(fp);
	}
}

/*
 * Dump the current tree to a log-file
 */
private	ft_dump (
	_AR1(char *,	msg))
	_DCL(char *,	msg)
{
	auto	 time_t	now	= time((time_t *)0);
	auto	 FILE	*fp;
	register FTREE *f;
	register int j, k;

	if (fp = ft_DUMP()) {
		PRINTF("writing log file \"%s\"\r\n", msg);
		FPRINTF(fp, "FTREE LOG: \"%s\" %s", msg, ctime(&now));
		FPRINTF(fp, "Total diffs: %d\n", FDdiff);
		FPRINTF(fp, "Total size:  %d\n", FDsize);
		FPRINTF(fp, "Total nodes: %d\n", FDlast);
		for (j = 0; j <= FDlast; j++) {
			f = &ftree[j];
			FPRINTF(fp, "%3d ^%03d", j, f->f_root);
			/* verify legality of the root-links */
			for (k = j; k; k = f->f_root, f = &ftree[k]) {
				if (f->f_root < 0 || f->f_root > FDlast) {
					k = -1;
					break;
				}
				if (k == f->f_root) {
					k = -2;
					break;
				}
			}
			if (k < 0) {
				FPRINTF(fp, "(%s)\n", k == -2 ? "err" : "loop");
				continue;
			}
			f = &ftree[j];
			for (k = fd_level(j); k > 0; k--)
				FPRINTF(fp, "|---");
			FPRINTF(fp, "%s/", f->f_name);
			if (!fd_show(j))
				FPRINTF(fp," ?");
			if (k = f->f_mark) {
				FPRINTF(fp," ");
				if (k & MARKED)	FPRINTF(fp,"M");
				if (k & NOSCCS)	FPRINTF(fp,"S");
				if (k & NOVIEW)	FPRINTF(fp,"V");
			}
			FPRINTF(fp, "\n");
		}
		PRINTF("**done***\r\n");
		FCLOSE(fp);
	} else
		perror("ftree.log");
}
#endif	/* DEBUG */

/*
 * Scan a given directory, inserting all entries which are themselves valid
 * directories
 */
public	int	ft_scan(
	_ARX(RING *,	gbl)
	_ARX(int,	node)
	_ARX(int,	levels)
	_AR1(int,	base)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	node)
	_DCL(int,	levels)
	_DCL(int,	base)
{
	auto	int	found = FALSE;
	auto	DIR	*dp;
	auto	DirentT *d;
	auto	int	count	= 0;
	char	bfr[MAXPATHLEN], *s_ = bfr;

	auto	int	interrupted = 0;
	(void)dedsigs(TRUE);		/* reset interrupt-counter */

	s_ += strlen(fd_path(bfr,node));

	if (chdir(bfr) < 0)
		waitmsg(bfr);
	else if ((dp = opendir(bfr)) != 0) {
		ft_remove(bfr,TRUE);
		if (strcmp(bfr,zero))	*s_++ = '/';
		while ((d = readdir(dp)) != NULL) {
			(void)strcpy(s_, "*");
			fd_slow(count++, base, bfr);
			FORMAT(s_, "%s", d->d_name);
			if (dotname(s_))		continue;
			if (ft_stat(bfr, s_))
				found = TRUE;
			if ((interrupted = dedsigs(TRUE)) != 0)
				break;	/* exit loop so we can cleanup */
		}
		ft_purge(gbl);
		(void)closedir(dp);

		if (interrupted) {
			waitmsg(bfr);	/* show where we stopped */
			return (-1);	/* ...and give up from there */
		}

		/* recur to lower levels if asked */
		if (levels > 1) {
			register int j;
			for (j = node+1; j <= FDlast; j++) {
				if (zROOT(j) == node) {
					found = TRUE;
					if (zLINK(j))
						continue;
					if (ft_scan(gbl, j, levels-1, base) < 0)
						return (-1);
				}
			}
		}
		if (!found)
			markit(node,NOVIEW,FALSE);
	}
	(void)chdir(gbl->new_wd);
	return (0);
}

/*
 * Set/toggle the number of levels shown below the present node.
 */
public	void	ft_set_levels (
	_ARX(int,	row)
	_AR1(int,	levels)
		)
	_DCL(int,	row)
	_DCL(int,	levels)
{
	int	found	= FALSE;
	int	toggle	= (levels == 1);
	register int	j;

	for (j = row+1; j <= FDlast; j++) {
		if (ftree[j].f_root == row) {
			found = TRUE;
			if (toggle) {
				markit(row, NOVIEW, !zHIDE(row));
				break;
			} else if (levels > 2) {
				markit(row, NOVIEW, FALSE);
				ft_set_levels(j, levels-1);
			} else {
				markit(row, NOVIEW, FALSE);
				markit(j, NOVIEW, (j+1 <= FDlast)
					&& ftree[j+1].f_root == j);
			}
		} else if (!toggle && (fd_level(j) <= fd_level(row))) {
			break;
		}
	}
	if (!found) {	/* just in case we're cleaning up after purge */
		markit(row, NOVIEW, FALSE);
	}
}

/*
 * Test a given name to see if it is either a directory name, or a symbolic
 * link.  In either (successful) case, add the name to the database.
 */
public	int	ft_stat(
	_ARX(char *,	name)
	_AR1(char *,	leaf)
		)
	_DCL(char *,	name)
	_DCL(char *,	leaf)
{
	auto	Stat_t	sb;

	if (stat_dir(leaf, &sb) >= 0) {
		ft_insert(name);
		return TRUE;
	}
	return FALSE;
}

/*
 * If any changes to the file-tree database have occurred, update the copy
 * in the user's home directory.  If we cannot write back to the user's
 * directory, no matter, since we mustn't use root privilege for this!
 */
public	void	ft_write(_AR0)
{
	if (FDdiff || (savesccs != showsccs)) {
	int	fid;
	register int j;
	register unsigned k;
#ifdef	DEBUG
		ft_dump("write");
#endif
		cant_W = TRUE;
		if ((fid = open(FDname, O_WRONLY|O_CREAT|O_TRUNC, 0644)) >= 0) {
			char *heap;
#ifdef	DEBUG
			PRINTF("writing file \"%s\" (%d)\n", FDname, FDlast);
#endif
#define	WRT(s,n)	(void)write(fid,(char *)s,(LEN_READ)(n))
			WRT(&FDlast, sizeof(FDlast));
			WRT(ftree, ((FDlast+1) * sizeof(FTREE)));

			for (j = k = 0; j <= FDlast; j++)
				k += strlen(ftree[j].f_name)+1;
			heap = doalloc(Null, k);
			for (j = k = 0; j <= FDlast; j++)
				k += strlen(strcpy(heap+k, ftree[j].f_name)) + 1;
			(void)write(fid, heap, (LEN_READ)k);
			free(heap);

			(void)close(fid);
			cant_W   = FALSE;
			showdiff = FDdiff = 0;
			savesccs = showsccs;
			(void)time(&FDtime);
		} else if (errno != EPERM
			&& errno != ENOENT
			&& errno != EACCES)
			wait_warn(FDname);
	}
}
