#ifndef	lint
static	char	Id[] = "$Id: ftree.c,v 8.1 1990/08/27 08:38:21 dickey Exp $";
#endif	lint

/*
 * Author:	T.E.Dickey
 * Created:	02 Sep 1987
 * $Log: ftree.c,v $
 * Revision 8.1  1990/08/27 08:38:21  dickey
 * modified 'ft_read()' and 'ft_write()' to try to recover from
 * missing/corrupt ".ftree" file.
 *
 *		Revision 8.0  90/05/23  10:51:18  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.2  90/05/23  10:51:18  dickey
 *		set initial "=>" marker in 'ft_view()' so that caller can
 *		give an arbitrary 'path' value (e.g., when error-recovering
 *		from 'E' command).  Also, pass CTL(E) command out of 'ft_view()'
 *		like 'E' command.
 *		
 *		Revision 7.1  90/05/23  09:29:30  dickey
 *		modified interface to 'dedring()'
 *		
 *		Revision 7.0  90/03/06  08:25:57  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/03/06  08:25:57  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.3  90/03/06  08:25:57  dickey
 *		lint
 *		
 *		Revision 5.2  90/02/01  13:25:15  dickey
 *		use 'showpath()' to handle long pathname-display
 *		
 *		Revision 5.1  89/11/13  14:19:15  dickey
 *		added some error recovery in 'ft_read()' against corrupted
 *		".ftree" file (i.e., missing string-heap).
 *		
 *		Revision 5.0  89/10/16  09:19:21  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.4  89/10/16  09:19:21  dickey
 *		re-simplified 'ft_stat()' (don't need lstat)
 *		
 *		Revision 4.3  89/10/16  09:17:39  dickey
 *		altered 'ft_stat()' so we don't look at stat.st_ino (don't
 *		assume inode is positive!), and to use lstat/stat combination
 *		
 *		Revision 4.2  89/09/06  15:45:30  dickey
 *		use ACC_PTYPES rather than inline def's
 *		
 *		Revision 4.1  89/08/25  07:44:50  dickey
 *		use 'wrepaint()' rather than savewin/unsavewin
 *		
 *		Revision 4.0  89/06/05  15:44:44  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/06/05  15:44:44  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.1  89/06/05  15:44:44  dickey
 *		revised/simplified code which inserts logical links by making
 *		this part of 'ft_insert()'
 *		
 *		Revision 2.0  89/03/14  13:25:04  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.66  89/03/14  13:25:04  dickey
 *		sccs2rcs keywords
 *		
 *		14 Mar 1989, interface to 'dlog'.
 *		07 Mar 1989, forgot that 'strchr()' will also search for a null.
 *		23 Jan 1989, to support 'A' toggle and '~' home-command
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
 *		TEST	- make a standalone program (otherwise, part of 'ded')
 */

#ifdef	TEST
#define	MAIN
#endif	TEST

#define	ACC_PTYPES
#define	DIR_PTYPES
#include	"ded.h"

#include	<fcntl.h>
#include	<sys/errno.h>
extern	time_t	time();
extern	int	errno;
extern	char	*pathcat(),
		*rcs_dir(),
		*sccs_dir(),
		*txtalloc();

#define	dedmsg	waitmsg	/* ...so we don't call 'showC' from this module */

#ifdef	apollo
#define	TOP	2
#define	ROOT	"//"
#else
#define	TOP	1
#define	ROOT	"/"
#endif

#define	PATH_ROW	0	/* line to show "path:" on */
#define	FLAG_ROW	1	/* line to show "flags:" on */
#define	LOSHOW	(2)		/* first line to show directory name on */
#define	TOSHOW	(LINES-LOSHOW)	/* lines to show on a screen */

#define	NORMAL	0
#define	MARKED	1
#define	VISITED	2
#define	NOSCCS	4	/* set to disable viewing sccs-directories */
#define	NOVIEW	8	/* set to disable viewing of a tree */
#define	LINKED	16	/* set to show link-to-directory */

#define	MAXLVL	999

#define	PRE(j,c)	(ftree[j].f_name[0] == c)
#define	ALL_SHOW(j)	(all_show || !(PRE(j,'.') || PRE(j,'$')))
#define	zMARK(j)	(ftree[j].f_mark & MARKED)
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

static	char	FDname[MAXPATHLEN];	/* name of user's database	*/
static	time_t	FDtime;			/* time: last modified ".ftree"	*/
static	unsigned FDsize;		/* current sizeof(ftree[])	*/
static	int	FDdiff,			/* number of changes made	*/
		FDlast,			/* last used-entry in ftree[]	*/
		cant_W,			/* TRUE if last ft_write failed	*/
		showbase,		/* base of current display	*/
		showlast,		/* last line in current display	*/
		showdiff = -1,		/* controls re-display		*/
		all_show = TRUE,	/* TRUE to suppress '.' files	*/
		out_of_sight = TRUE,	/* TRUE to suppress search	*/
		savesccs,		/* original state of 'showsccs'	*/
		showsccs = TRUE;	/* control display of 'sccs'	*/
static	char	zero[] = ROOT,
		*gap = zero + (TOP-1);
static	FTREE	*ftree;			/* array of database entries	*/

/************************************************************************
 *	Database Manipulation						*
 ************************************************************************/

/*
 * Show count while doing things which may be time-consuming.
 */
static
fd_slow(count, pathname)
char	*pathname;
{
static
time_t	last;
time_t	this	= time((time_t *)0);
int	y,x;

	if ((count == 0) || (last != this)) {
		getyx(stdscr,y,x);
		move(PATH_ROW,0);
		PRINTW("%4d: ", count);
		showpath(pathname, 999, 0);
		clrtoeol();
		refresh();
		move(y,x);
	} else
		last = this;
}

/*
 * Ensure that the database has allocated enough space for the current entry.
 */
static
fd_alloc()
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
 * Perform a search through the database for a selected directory name.
 */
static
fd_find (buffer,cmd,old)
char	*buffer, cmd;
{
static	int	next = 0;		/* last-direction		*/
static	char	pattern[MAXPATHLEN],
		*expr;			/* regex-state/output		*/

register int	step,
	new = old,
	skip,
	looped = 0;

	if (cmd == '?' || cmd == '/') {
		if (strchr(buffer, (size_t)(*gap)))
			return(-1);	/* we don't search full-paths */
		if (*buffer)
			(void)strcpy(pattern,buffer);
		step =
		next = (cmd == '/') ? 1 : -1;
	} else
		step = (cmd == 'n') ? next : -next;

	OLD_REGEX(expr);
	if (NEW_REGEX(expr,pattern)) {
		do {
			if (looped++ && (new == old)) { beep();	return(-1); }
			else if ((new += step) < 0)		new = FDlast;
			else if (new > FDlast)			new = 0;
			skip = (out_of_sight && !fd_show(new));
		}
		while (skip || !GOT_REGEX(expr, ftree[new].f_name));
		return(new);
	}
#ifndef	TEST
	BAD_REGEX(expr);
#endif	TEST
	return (-1);
}

/*
 * Returns the hierarchical level of a given node
 */
static
fd_level(this)
{
register int level = this ? 1 : 0;
	while (this = ftree[this].f_root) level++;
	return(level);
}

static
node2col(node, level)
{
	register int k = fd_level(node);

	if (level < k) k = level;
	return ((k * 4) + 6);
}

static
node2row(node)
{
	register int j, row;

	for (j = showbase, row = LOSHOW-1; j <= showlast; j++) {
		if (fd_show(j))	row++;
		if (j == node)	break;
	}
	return (row);
}

/*
 * Returns a code appropriate for displaying the directory-tree's lines
 */
static
char *
fd_line(height)
{
	if (height != 1) {
		return("|   ");
	}
	return("|---");
}

/*
 * Compute a pathname for a given node
 */
static
char *
fd_path(bfr, node)
char	*bfr;
{
char	tmp[MAXPATHLEN];
	*bfr = EOS;
	do {
		(void)strcpy(tmp, bfr);
		(void)strcat(strcat(strcpy(bfr, gap), ftree[node].f_name), tmp);
	}
	while (node = ftree[node].f_root);
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
static
fd_show(node)
{
	if (ftree[node].f_mark & NOSCCS)
		return(FALSE);
	if (!ALL_SHOW(node))
		return(FALSE);
	while (node = ftree[node].f_root) {
		if (ftree[node].f_mark & NOVIEW)
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
ft_insert(path)
char	*path;
{
	auto	int		last = 0, /* assume we start from root level */
				order,
				sort,
				this;
	auto	char		bfr[MAXPATHLEN];
	auto	struct	stat	sb;
	register int		j;
	register FTREE		*f;

	abspath(path = strcpy(bfr,path));
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
		*next = strchr(name, (size_t)(*gap));

		if (next != 0)
			*next = EOS;

		/* double-check link/directory here */
		if (lstat(bfr, &sb) < 0)
			break;

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
			ftree[this].f_mark = NORMAL;
			ftree[this].f_name = txtalloc(name);
			if (!showsccs && is_sccs(this))
				ftree[this].f_mark |= NOSCCS;
		}

		last = this;
		ftree[last].f_mark &= ~MARKED;

#ifdef	S_IFLNK
		if (isLINK(sb.st_mode)) {
			ftree[last].f_mark |= LINKED;
			/* patch: store pointer to show 'readlink()' */
			break;
		}
		if (isDIR(sb.st_mode)) {
			ftree[last].f_mark &= ~LINKED;
		} else
			break;
#endif	S_IFLNK

		if (next != 0) {
			*next = *gap; /* restore the one we knocked out */
			path  = next;
		} else
			break;
	}
}

/*
 * Locate the node corresponding to a particular path.
 */
static
do_find(path)
char	*path;
{
char	bfr[MAXPATHLEN];
register int j, this, last = 0;

	abspath(path = strcpy(bfr,path));
	if (!strcmp(path,zero))
		return(0);

	path += (TOP-1);
	while (*path == *gap) {
	char	*name = ++path,
		*next = strchr(path, (size_t)(*gap));
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
ft_remove(path,all)
char	*path;
{
int	last = do_find(path);
register int j;

	if (last >= 0) {
		for (j = last+1; j <= FDlast; j++) {
		register FTREE *f = &ftree[j];
			if (f->f_root == last) {
#ifdef	S_IFLNK
				if (all || !(f->f_mark & LINKED))
#endif	S_IFLNK
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
ft_purge()
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

#ifndef	TEST
	if (changed) {			/* re-insert ring */
	extern	char	*dedrung();
	char	tmp[BUFSIZ], *s;
		(void)strcpy(tmp, new_wd);
		for (j = 1; s = dedrung(j); j++) {
			ft_insert(s);
			if (!strcmp(s,tmp))
				break;
		}
	}
#endif	TEST
}

/*
 * Rename a directory or link.
 */
ft_rename(old, new)
char	*old, *new;
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
static
ft_init(msg)
char	*msg;
{
	waitmsg(msg);
	FDtime = time((time_t *)0);
	FDlast = 0;
	fd_alloc();
	return (FALSE);
}

/* read from the .ftree file, testing for consistency in sizes */
static
ok_read(fid,s,ask,msg)
char	*s;
char	*msg;
{
	LEN_READ	got = read(fid,s,ask);
	if (got != ask) {
		char	bfr[BUFSIZ];
		FORMAT(bfr, "%s (got %d, asked %d)", msg, got, ask);
		return (ft_init(msg));
	}
	return (TRUE);
}

static
read_ftree(the_file)
char	*the_file;
{
	struct		stat sb;
	register int	j;
	LEN_READ	vecsize;
	int		fid,
			size;

	/* read the current database */
	if ((fid = open(the_file, O_RDONLY)) != 0) {
		if (stat(the_file, &sb) < 0)
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
				"size \".ftree\""))
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
				(char *)ftree, vecsize,
				"read \".ftree\""))
			return;

		/* (3) string-heap */
		if ((size -= vecsize) > 0) {
		char	*heap = doalloc((char *)0, (unsigned)(size+1));
		register char *s = heap;
			if (!ok_read(fid,
					heap, size,
					"heap \".ftree\""))
				return;
			s[size] = EOS;
			for (j = 0; j <= FDlast; j++) {
				if (*s != EOS && !isprint(*s)) {
					(void)ft_init("? corrupted heap");
					return;
				}
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
#endif	DEBUG
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
ft_read(first,home_dir)
char	*first;		/* => string defining the initial directory */
char	*home_dir;
{
	register int	j;

	read_ftree(pathcat(FDname, home_dir, ".ftree"));
	FDdiff = 0;

	/* append the current directory to the list */
	ft_insert(first ? first : ".");

	/* inherit sense of 'Z' toggle from database */
	showsccs = TRUE;
	for (j = 0; j <= FDlast; j++) {
		if (ftree[j].f_mark & NOSCCS) {
			showsccs = FALSE;
			break;
		}
	}
	savesccs = showsccs;
}

/*
 * Display the file-tree.  To help keep paths from growing arbitrarily, show
 * the common (at least to the current screen) part of the path in the
 * status line, moving to the common nodes only if the user directs so.
 */
static
ft_show(path, home, node, level)
char	*path, *home;
{
	register int j, k;
	auto	int	row;
	auto	char	*marker,
	bfr[BUFSIZ];

	move(PATH_ROW,0);
	row = LOSHOW;
	node = limits(showbase, node);
	k = FDdiff || (savesccs != showsccs);
	PRINTW("path: ");
	showpath(path, level, k ? 5 : 0);
	dlog_comment("path: %s\n", path);
	clrtoeol();
	if (k) {	/* show W-command if we have pending diffs */
		move(PATH_ROW, COLS-5);
		PRINTW(" (W%s)", cant_W ? "?" : "");
	}
	move(FLAG_ROW,0);
	PRINTW("flags:");
	if (!all_show)		PRINTW(" A (hide '.' names)");
	if (out_of_sight) 	PRINTW(" I (inhibit search in V)");
	if (!showsccs) 		PRINTW(" Z (hide SCCS/RCS)");
	clrtoeol();
	if (showdiff != FDdiff) {
		for (j = showbase; j <= showlast; j++) {
			if (!fd_show(j)) continue;
			move(row++,0);
			marker = strcmp(fd_path(bfr, j), home) ? ". " : "=>";
#ifndef	TEST
			if (*marker == '.' && dedrang(bfr))
				marker = "* ";
#endif	TEST
			PRINTW("%4d%s", j, marker);
			for (k = fd_level(j); k > 0; k--)
				addstr(fd_line(k));
			if (ftree[j].f_mark & MARKED)	standout();
			(void)ded2string(bfr, sizeof(bfr), ftree[j].f_name, FALSE);
			PRINTW("%s%s",
				bfr,
				(ftree[j].f_mark &LINKED) ? "@" : gap);
			if (ftree[j].f_mark & MARKED)	standend();
#ifdef	apollo
			if (j == 0)
				PRINTW("%s", zero+1);
#endif	apollo
			if (ftree[j].f_mark & NOVIEW) PRINTW(" (V)");
			if (ftree[j].f_mark & MARKED) PRINTW(" (+)");
			clrtoeol();
		}
		clrtobot();
	}
	move(node2row(node), node2col(node,level));
	refresh();
	return(node);
}

/*
 * Set base/last limits for the current screen
 */
static
limits(base,row)
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

static
fd_fwd(num)
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

static
fd_bak(num)
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
 * Scroll to current ring-entry
 */
static
fd_ring(path, row_, level_)
char	*path;
int	*row_, *level_;
{
	char	cwdpath[BUFSIZ];
	if ((*row_ = do_find(strcpy(cwdpath,new_wd))) < 0) {
		/* path was deleted, put it back */
		/* patch: should do ft_stat, recover if err */
		ft_insert(cwdpath);
		*row_ = do_find(cwdpath);
	}
	*level_ = fd_level(*row_);
	scroll_to(*row_);
	(void)strcpy(path, cwdpath);
	showdiff = -1;		/* always refresh '*', '=>' marks */
}

static
uprow(node,count,level)
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
		while (k < showbase)
			(void)fd_bak(1);
	} else
		beep();
	return(k);
}

static
downrow(node,count,level)
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
		while (k > showlast)
			(void)fd_fwd(1);
	} else
		beep();
	return(k);
}

static
markit(node,flag,on)
{
register FTREE *f = &ftree[node];
int	old = f->f_mark;
	if (on)			f->f_mark |= flag;
	else			f->f_mark &= ~flag;
	if (old != f->f_mark)	FDdiff++;
}

/*
 * Returns TRUE iff the name of the node corresponds to a source-control
 * directory.
 */
static
is_sccs(node)
{
register FTREE *f = &ftree[node];
	if (!strcmp(f->f_name, sccs_dir()))	return (TRUE);
	if (!strcmp(f->f_name, rcs_dir()))	return (TRUE);
	return (FALSE);
}

/*
 * Toggles the state of the sccs-directory visibility, and coerces all nodes
 * to correspond to the new state.
 */
static
toggle_sccs()
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

/*
 * Set up for display of the given node.  If it is not visible, make it so.
 */
static
scroll_to(node)
{
register int j = node;

	if (ftree[j].f_mark & NOSCCS)
		toggle_sccs();
	while (j = ftree[j].f_root) {	/* ensure this is visible! */
		if (ftree[j].f_mark & NOVIEW)
			markit(j,NOVIEW,FALSE);
		if (ftree[j].f_mark & NOSCCS)
			toggle_sccs();
	}
	(void)limits(showbase,showbase);
	while (node > showlast) (void)fd_fwd(1);
	while (node < showbase) (void)fd_bak(1);
}

/*
 * Interactively display the directory tree and modify it:
 *	* The return value is used by the coroutine as the next command.
 *	* The argument is overwritten with the name of the new directory
 *	  for e/E commands.
 */
ft_view(path)
char	*path;
{
	auto	 int	row,
			lvl,
			num,
			c;
	auto	 char	cwdpath[MAXPATHLEN];
	register int	j;

	/* set initial position */
	abspath(strcpy(cwdpath,path));
	ft_insert(cwdpath);
	row = do_find(cwdpath);
	lvl = fd_level(row);
	scroll_to(row);
	showdiff = -1;
#ifndef	TEST
	(void)strcpy(path, dedrung(0));	/* init for "=>" marker */
#endif

	/* process commands */
	for (;;) {

		c = fd_level(row);
		if (c < lvl) lvl = c;	/* loosely drag down level */
		row = ft_show(fd_path(cwdpath,row), path, row, lvl);

		switch(c = dlog_char(&num,1)) {
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
		case '\r':
		case '\n':
				lvl = MAXLVL;
		case ARO_DOWN:
		case 'j':	row = downrow(row,num,lvl);	break;
		case ARO_UP:
		case 'k':	row = uprow(row,num,lvl);	break;
		case ARO_RIGHT:
		case 'l':	lvl += num;			break;

		case 'J':	row = downrow(row,num,MAXLVL);	break;
		case 'K':	row = uprow(row,num,MAXLVL);	break;

		case '^':	if (showbase != row) {
					showbase = row;
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

		case '/':
		case '?':
		case 'n':
		case 'N':
			*cwdpath = EOS;
		case '~':
		case '@':
			if (!isalpha(c)) {
				move(PATH_ROW,0);
				PRINTW((c != '@') ? "find: " : "jump: ");
				clrtoeol();
				if (c == '~')
					(void)strcpy(cwdpath, "~");
				dlog_string(cwdpath,sizeof(cwdpath),FALSE);
				if (!*cwdpath && c != '@') {
					c = -1;
					beep();
				}
			}
			if (c != '@' && c != '~')
				c = fd_find(cwdpath,c,row);
			else {
				abspath(cwdpath);
				if ((c = do_find(cwdpath)) < 0) {
					ft_stat(cwdpath,cwdpath);
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

		case '=':	/* patch: test rename-function */
			if (chdir(fd_path(cwdpath,zROOT(row))) >= 0) {
			char	bfr[BUFSIZ];
				abspath(fd_path(cwdpath,row));
				move(node2row(row),node2col(row,MAXLVL));
				(void)strcpy(bfr, ftree[row].f_name);
				dlog_string(bfr,sizeof(bfr),FALSE);
				abspath(bfr);
				if (strcmp(cwdpath, bfr) ) {
					ft_rename(cwdpath, bfr);
					scroll_to (row = do_find(bfr));
				}
				(void)chdir(new_wd);
			} else
				waitmsg(cwdpath);
			break;
#ifndef	TEST
		/* dump the current screen */
		case CTL(K):
			deddump();
			break;

#define	SKIP_THIS(num)	dedring(fd_path(cwdpath, row), c, num, FALSE, (char *)0)

		/* quit lists in directory-ring */
		case 'Q':
		case 'q':
			while (num-- > 0) {
				j = SKIP_THIS(1);
				if (!j)
					break;
				if (is_sccs(row) && (savesccs != showsccs))
					toggle_sccs();
			}
			fd_ring(path, &row, &lvl);
			if (!j)
				return('E');
			break;
		/* scroll through the directory-ring */
		case 'F':
		case 'B':
			num = SKIP_THIS(num);
			fd_ring(path, &row, &lvl);
			break;

		/* Exit from this program (back to 'ded') */
		case CTL(E):
		case 'E':
		case 'e':
			(void)fd_path(cwdpath, row);
#ifdef	S_IFLNK
			if (ftree[row].f_mark & LINKED) {
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
				(void)chdir(new_wd);
			}
#endif	S_IFLNK
			if (access(cwdpath, R_OK | X_OK) < 0) {
				beep();
				break;
			}
			(void)strcpy(path, cwdpath);
			/* fall-thru to return */
#endif	TEST
		case 'D':
			return(c);	/* 'e', 'E' CTL(E) or 'D' -- only! */

		/* Scan/delete nodes */
		case 'R':	if (ftree[row].f_mark & LINKED)
					beep();
				else
					(void)ft_scan(row, num);
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
					if (ftree[j].f_mark & MARKED)
						num = 0;
					else if (num == 0)
						num = j;
				if (num >= 0) {
					ft_purge();
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
#endif	apollo
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
		case 'A':
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
			U_opt = !U_opt;
			showdiff = -1;
			break;
#endif	apollo

		/* toggle flag which controls whether we show dependents */
		case 'V':
			if (row) {
				for (j = row+1; j <= FDlast; j++) {
					if (ftree[j].f_root == row) {
						markit(row,NOVIEW,!(ftree[row].f_mark & NOVIEW));
						break;
					}
				}
			} else
				beep();
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
static
FILE *
ft_DUMP()
{
	static	char	logname[BUFSIZ];
	return (fopen(strcat(strcpy(logname, gethome()), "/ftree.log"), "a+"));
}

/*
 * Dump a message to a log-file
 */
ft_dump2(fmt, msg)
char	*fmt,*msg;
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
ft_dump(msg)
char	*msg;
{
	extern	 char	*ctime();
	extern	 time_t	time();
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
#endif	DEBUG

/*
 * Scan a given directory, inserting all entries which are themselves valid
 * directories
 */
ft_scan(node, levels)
{
DIR	*dp;
struct	direct	*d;
int	count	= 0;
char	bfr[MAXPATHLEN], *s_ = bfr;

#ifdef	TEST
char	old[MAXPATHLEN];
	abspath(strcpy(old,"."));
#else	TEST
int	interrupted = 0;
	(void)dedsigs(TRUE);		/* reset interrupt-counter */
#endif	TEST

	s_ += strlen(fd_path(bfr,node));

	if (chdir(bfr) < 0)
		waitmsg(bfr);
	else if ((dp = opendir(bfr)) != 0) {
		ft_remove(bfr,TRUE);
		if (strcmp(bfr,zero))	*s_++ = '/';
		while (d = readdir(dp)) {
			(void)strcpy(s_, "*");
			fd_slow(count++, bfr);
			FORMAT(s_, "%s", d->d_name);
			if (dotname(s_))		continue;
			ft_stat(bfr, s_);
#ifndef	TEST
			if (interrupted = dedsigs(TRUE))
				break;	/* exit loop so we can cleanup */
#endif	TEST
		}
		ft_purge();
		(void)closedir(dp);

#ifndef	TEST
		if (interrupted) {
			waitmsg(bfr);	/* show where we stopped */
			return (-1);	/* ...and give up from there */
		}
#endif	TEST

		/* recur to lower levels if asked */
		if (levels > 1) {
			register int j;
			for (j = node+1; j <= FDlast; j++)
				if ((zROOT(j) == node) && !zLINK(j))
					if (ft_scan(j, levels-1) < 0)
						return (-1);
		}
	}
#ifdef	TEST
	(void)chdir(old);
#else	TEST
	(void)chdir(new_wd);
#endif	TEST
	return (0);
}

/*
 * Test a given name to see if it is either a directory name, or a symbolic
 * link.  In either (successful) case, add the name to the database.
 */
ft_stat(name, leaf)
char	*name, *leaf;
{
	auto	struct	stat	sb;

	if (stat(leaf, &sb) >= 0) {
		if (isDIR(sb.st_mode))
			ft_insert(name);
	}
}

/*
 * If any changes to the file-tree database have occurred, update the copy
 * in the user's home directory.  If we cannot write back to the user's
 * directory, no matter, since we mustn't use root privilege for this!
 */
ft_write()
{
	if (FDdiff || (savesccs != showsccs)) {
	int	fid;
	register int j;
#ifdef	DEBUG
		ft_dump("write");
#endif	DEBUG
		cant_W = TRUE;
		if ((fid = open(FDname, O_WRONLY|O_CREAT|O_TRUNC, 0644)) >= 0) {
#ifdef	DEBUG
			PRINTF("writing file \"%s\" (%d)\n", FDname, FDlast);
#endif	DEBUG
#define	WRT(s,n)	(void)write(fid,(char *)s,(LEN_READ)(n))
			WRT(&FDlast, sizeof(FDlast));
			WRT(ftree, ((FDlast+1) * sizeof(FTREE)));
			for (j = 0; j <= FDlast; j++)
				WRT(ftree[j].f_name, strlen(ftree[j].f_name)+1);
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

#ifdef	TEST
/*
 * Main program, for standalone applications
 */
main(argc, argv)
char	*argv[];
{
extern	WINDOW	*initscr();
int	j,
	remove	= FALSE,
	purge	= FALSE;
char	cwdpath[MAXPATHLEN],
	*s;

	ft_read(".", gethome());
	for (j = 1; j < argc; j++) {
		s = argv[j];
		if (*s == '-') {
			while (*++s) {
				switch(*s) {
				case 'r':	remove = TRUE;	break;
				case 'i':	remove = FALSE;	break;
				case 'p':	purge  = TRUE;	break;
				}
			}
		} else {
			if (remove)	ft_remove(s);
			else		ft_insert(s);
		}
	}
	if (purge)
		ft_purge();
	if (!initscr())			failed("initscr");
	rawterm();
	(void)ft_view(".");
	move(LINES-1,0);
	refresh();
	PRINTF("\n");
	endwin();
	ft_write();
	(void)exit(0);
	/*NOTREACHED*/
}
#endif	TEST
