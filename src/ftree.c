#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)ftree.c	1.37 88/05/06 15:39:36";
#endif

/*
 * Author:	T.E.Dickey
 * Created:	02 Sep 1987
 * Modified:
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
 *		ft_linkto
 *		ft_remove
 *		ft_purge
 *		ft_scan
 *		ft_stat
 *		ft_view
 *
 * Configure:	DEBUG	- dump a logfile in readable form at the end
 *			  (i.e., when calling 'ft_read()' or 'ft_write()').
 *		TEST	- make a standalone program (otherwise, part of 'fl')
 */

#ifdef	TEST
#define	MAIN
#endif	TEST
#include	"ded.h"

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/errno.h>
extern	long	time();
extern	int	errno;
extern	char	*stralloc(),
		*strchr();

#ifndef	R_OK		/* should be in <sys/file.h>, but apollo has conflict */
#define	R_OK	4
#define	X_OK	1
#endif	R_OK

#ifdef	apollo
#define	TOP	2
#define	ROOT	"//"
#else
#define	TOP	1
#define	ROOT	"/"
#endif

#define	MAXPATHLEN	BUFSIZ

#define	TOSHOW	(LINES-1)	/* lines to show on a screen */

#define	PRINTF	(void)printf
#define	FPRINTF	(void)fprintf
#define	FORMAT	(void)sprintf

#define	NORMAL	0
#define	MARKED	1
#define	VISITED	2
#define	NOSCCS	4	/* set to disable viewing sccs-directories */
#define	NOVIEW	8	/* set to disable viewing of a tree */
#define	LINKED	16	/* set to show link-to-directory */

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

static	char	FDname[MAXPATHLEN];	/* name of user's database	*/
static	long	FDtime;			/* time: last modified ".ftree"	*/
static	int	FDdiff,			/* number of changes made	*/
		FDlast,			/* last used-entry in ftree[]	*/
		FDsize,			/* current sizeof(ftree[])	*/
		showbase,		/* base of current display	*/
		showlast,		/* last line in current display	*/
		showdiff = -1,		/* controls re-display		*/
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
fd_slow(count)
{
static
time_t	last;
time_t	this	= time((long *)0);
int	y,x;

	if ((count != 0) && (last != this)) {
		getyx(stdscr,y,x);
		move(0,0);
		printw("%4d", count);	/* overwrite "path" */
		move(y,x);
		refresh();
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
		ftree = DOALLOC(FTREE,ftree,FDsize);
		while (size < FDsize) {
			ftree[size].f_root =
			ftree[size].f_mark = 0;
			ftree[size].f_name = stralloc("");
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
	looped = 0;

	if (cmd == '?' || cmd == '/') {
		if (strchr(buffer, *gap))
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
		}
		while (! GOT_REGEX(expr, ftree[new].f_name));
		return(new);
	}
#ifndef	TEST
	BAD_REGEX(expr);
	move(LINES-1,0);
	beep();
	refresh();
	(void)cmdch((int *)0);	/* pause beside error message */
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
	*bfr = '\0';
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
 * Recursively mark for removal entries which belong to a specified node
 */
static
fd_repur(last)
{
register int j;
	for (j = last+1; j <= FDlast; j++) {
	register FTREE *f = &ftree[j];
		if (f->f_root == last) {
			f->f_mark |= MARKED;
			fd_repur(j);
		}
	}
}

/*
 * Returns true iff the node should be displayed
 */
static
fd_show(node)
{
	if (ftree[node].f_mark & NOSCCS)
		return(FALSE);
	while (node = ftree[node].f_root) {
		if (ftree[node].f_mark & NOVIEW)
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
int	last = 0,	/* assume we start from root level */
	order,
	sort,
	this;
char	bfr[MAXPATHLEN];
register int j;
register FTREE *f;

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
		*next = strchr(name, *gap);
		if (next != 0)
			*next = '\0';

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
			if (!showsccs && !strcmp(name, "sccs"))
				ftree[this].f_mark |= NOSCCS;
			ftree[this].f_name = stralloc(name);
		}

		last = this;
		ftree[last].f_mark &= ~MARKED;

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
		*next = strchr(path, *gap);
		if (next) *next = '\0';
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

#ifndef	SYSTEM5
/*
 * Enter a symbolic link into the database.
 */
ft_linkto(path)
char	*path;
{
int	row;
	ft_insert(path);
	row = do_find(path);
	markit(row,LINKED,TRUE);
	/* patch: store pointer to show 'readlink()' */
}
#endif	SYSTEM5

/*
 * Mark nodes below a given path in the database for removal, unless they
 * are added back before the database is written out.  This is used to update
 * the database from 'fl' when (re)reading a directory.  The argument must
 * be a directory name.
 */
ft_remove(path)
char	*path;
{
int	last = do_find(path);
register int j;

	if (last >= 0) {
		for (j = last+1; j <= FDlast; j++) {
		register FTREE *f = &ftree[j];
			if (f->f_root == last)
				f->f_mark |= MARKED;
			else if (f->f_root < last)
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
register int j, k;

	for (j = 1; j <= FDlast; j++) {
		if (ftree[j].f_mark & MARKED) {
			fd_repur(j);
			for (k = j; k < FDlast; k++) {
				ftree[k] = ftree[k+1];
				if (ftree[k].f_root > j)
					ftree[k].f_root--;
			}
			FDlast--;
			FDdiff++;
			j--;
		}
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
ft_read(first)
char	*first;		/* => string defining the initial directory */
{
struct	stat sb;
register int j;
int	fid,
	vecsize,
	size;

	/* read the current database */
	(void)strcat(strcpy(FDname, getenv("HOME")), "/.ftree");
	if ((fid = open(FDname, O_RDONLY)) != 0) {
		if (stat(FDname, &sb) < 0)
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
		if (read(fid, (char *)&vecsize, sizeof(vecsize)) != sizeof(FDlast))
			failed("size \".ftree\"");
		if ((size / sizeof(FTREE)) < vecsize)
			failed("? size error");

		/* (2) vector-contents */
		if (vecsize > FDlast)
			FDlast = vecsize;
		fd_alloc();
		vecsize++;		/* account for 0'th node */
		vecsize *= sizeof(FTREE);
		if (read(fid, (char *)ftree, vecsize) != vecsize)
			failed("read \".ftree\"");

		/* (3) string-heap */
		if ((size -= vecsize) > 0) {
		char	*heap = DOALLOC(char,0,(unsigned)size);
		register char *s = heap;
			if (read(fid, heap, (int)size) != size)
				failed("heap \".ftree\"");
			for (j = 0; j <= FDlast; j++) {
				ftree[j].f_name = s;
				s += strlen(s) + 1;
			}
		}
		(void)close(fid);
#ifdef	DEBUG
		ft_dump("read");
#endif	DEBUG
	}
	FDdiff = 0;

	/* append the current directory to the list */
	ft_insert(first ? first : ".");

	/* inherit sense of 'Z' toggle from database */
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
int	row = 0;
char	*marker,
	bfr[BUFSIZ];

	move(row++,0);
	node = limits(showbase, node);
	printw("path: %.*s", COLS-8, path);
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
			printw("%4d%s", j, marker);
			for (k = fd_level(j); k > 0; k--)
				addstr(fd_line(k));
			if (ftree[j].f_mark & MARKED)	standout();
			printw("%s%s",
				ftree[j].f_name,
				(ftree[j].f_mark &LINKED) ? "@" : gap);
			if (ftree[j].f_mark & MARKED)	standend();
#ifdef	apollo
			if (j == 0)
				printw("%s", zero+1);
#endif	apollo
			if (ftree[j].f_mark & NOVIEW) printw(" (V)");
			if (ftree[j].f_mark & MARKED) printw(" (+)");
			clrtoeol();
		}
		clrtobot();
	}
	k = fd_level(node);
	if (level < k) k = level;
	for (j = showbase, row = 0; j <= showlast; j++) {
		if (fd_show(j))	row++;
		if (j == node)	break;
	}
	move(row, (k * 4) + 6);
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
forward(num)
{
	while (num-- > 0) {
		if (showlast < FDlast) {
			showbase = showlast;
			showdiff = -1;
			(void)limits(showbase,showbase);
		}
	}
	return(showlast);
}

static
backward(num)
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
		}
	}
	return(showbase);
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
			(void)backward(1);
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
			(void)forward(1);
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

static
toggle_sccs()
{
register int j;
	showsccs = !showsccs;
	for (j = 1; j <= FDlast; j++) {
	register FTREE *f = &ftree[j];
		if (!strcmp(f->f_name,"sccs")) {
			f->f_mark ^= NOSCCS;
			showdiff = -1;
		}
	}
}

static
scroll_to(node)
{
register int j = node;
	if (!strcmp(ftree[j].f_name, "sccs") && !showsccs)
		showsccs = TRUE;
	if (ftree[j].f_mark & NOSCCS)
		toggle_sccs();
	while (j = ftree[j].f_root) {	/* ensure this is visible! */
		if (ftree[j].f_mark & NOVIEW)
			markit(j,NOVIEW,FALSE);
		if (ftree[j].f_mark & NOSCCS)
			toggle_sccs();
	}
	(void)limits(showbase,showbase);
	while (node > showlast) (void)forward(1);
	while (node < showbase) (void)backward(1);
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
static
int	row,
	lvl,
	num,
	c;
char	cwdpath[MAXPATHLEN];
register int j;

	/* set initial position */
	abspath(strcpy(cwdpath,path));
	ft_insert(cwdpath);
	row = do_find(cwdpath);
	lvl = fd_level(row);
	scroll_to(row);
	showdiff = -1;

	/* process commands */
	for (;;) {

		c = fd_level(row) + 1;
		if (c < lvl) lvl = c;	/* loosely drag down level */
		row = ft_show(fd_path(cwdpath,row), path, row, lvl);

		switch(c = cmdch(&num)) {
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
				lvl = 999;
		case ARO_DOWN:
		case 'j':	row = downrow(row,num,lvl);	break;
		case ARO_UP:
		case 'k':	row = uprow(row,num,lvl);	break;
		case ARO_RIGHT:
		case 'l':	lvl += num;			break;

		case 'J':	row = downrow(row,num,999);	break;
		case 'K':	row = uprow(row,num,999);	break;

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
				row = forward(num);
			} else
				beep();
			break;
		case 'b':
			if (row > 0) {
				if (row > showbase) {
					row = showbase;
					num--;
				}
				row = backward(num);
			} else
				beep();
			break;

		case '/':
		case '?':
		case 'n':
		case 'N':
			*cwdpath = '\0';
		case '@':
			if (!isalpha(c)) {
				move(0,0);
				printw((c != '@') ? "find: " : "jump: ");
				clrtoeol();
				rawgets(cwdpath,sizeof(cwdpath),FALSE);
			}
			if (c != '@')
				c = fd_find(cwdpath,c,row);
			else {
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

#ifndef	TEST
		/* quit lists in directory-ring */
		case 'Q':
		case 'q':
		/* scroll through the directory-ring */
		case 'F':
		case 'B':
			num = dedring(fd_path(cwdpath, row), c, num);
			if ((row = do_find(strcpy(cwdpath,new_wd))) < 0) {
				/* path was deleted, put it back */
				/* patch: should do ft_stat, recover if err */
				ft_insert(cwdpath);
				row = do_find(cwdpath);
			}
			lvl = fd_level(row);
			scroll_to(row);
			(void)strcpy(path, cwdpath);
			if (!num && (c == 'q' || c == 'Q'))
				return('E');
			break;

		/* Exit from this program (back to 'fl') */
		case 'E':
		case 'e':
			(void)fd_path(cwdpath, row);
#ifndef	SYSTEM5
			if (ftree[row].f_mark & LINKED) {
			char	bfr[BUFSIZ];
			int	len = readlink(cwdpath, bfr, sizeof(bfr));
				if (len <= 0) {
					beep();
					break;
				}
				bfr[len] = EOS;
				abspath(strcpy(cwdpath, bfr));
			}
#endif	SYSTEM5
			if (access(cwdpath, R_OK | X_OK) < 0) {
				beep();
				break;
			}
			(void)strcpy(path, cwdpath);
			/* fall-thru to return */
#endif	TEST
		case 'D':
			return(c);	/* 'e' or 'E' or 'D' -- only! */

		/* Scan/delete nodes */
		case 'R':	if (ftree[row].f_mark & LINKED)
					beep();
				else
					ft_scan(row);
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
					showdiff = -1;
					row = showbase;
					break;
				}
#endif	apollo
				savewin();
				unsavewin(TRUE,0);
				break;

		/* Force dump */
		case 'W':
				ft_write();
				break;

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
ft_dump(msg)
char	*msg;
{
FILE	*fp = fopen("ftree.log", "a+");
register FTREE *f;
register int j, k;
	if (fp) {
		PRINTF("writing log file: %s\n", msg);
		FPRINTF(fp, "FTREE LOG: %s\n", msg);
		FPRINTF(fp, "Total diffs: %d\n", FDdiff);
		FPRINTF(fp, "Total size:  %d\n", FDsize);
		FPRINTF(fp, "Total nodes: %d\n", FDlast);
		for (j = 0; j <= FDlast; j++) {
			f = &ftree[j];
			FPRINTF(fp, "%3d ^%03d", j, f->f_root);
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
		(void)fclose(fp);
	} else
		perror("ftree.log");
}
#endif	DEBUG

/*
 * Scan a given directory, inserting all entries which are themselves valid
 * directories
 */
ft_scan(node)
{
DIR	*dp;
struct	direct	*d;
int	count	= 0;
char	old[MAXPATHLEN],
	bfr[MAXPATHLEN], *s_ = bfr;

	abspath(strcpy(old,"."));
	s_ += strlen(fd_path(bfr,node));

	if (chdir(bfr) < 0)
		perror(bfr);
	else if ((dp = opendir(bfr)) != 0) {
		ft_remove(bfr);
		if (strcmp(bfr,zero))	*s_++ = '/';
		while (d = readdir(dp)) {
			fd_slow(count++);
			FORMAT(s_, "%s", d->d_name);
			if (dotname(s_))		continue;
			ft_stat(bfr, s_);
		}
		ft_purge();
		(void)closedir(dp);
	}
	(void)chdir(old);
}

/*
 * Test a given name to see if it is either a directory name, or a symbolic
 * link.  In either (successful) case, add the name to the database.
 */
ft_stat(name, leaf)
char	*name, *leaf;
{
struct	stat	sb;
	if (lstat(leaf, &sb) >= 0) {
		if ((int)sb.st_ino > 0) {
#ifndef	SYSTEM5
			if (isLINK(sb.st_mode)) {
				if (stat(name, &sb) >= 0)
					if (isDIR(sb.st_mode))
						ft_linkto(name);
			} else
#endif	SYSTEM5
			if (isDIR(sb.st_mode))
				ft_insert(name);
		}
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
		if ((fid = open(FDname, O_WRONLY|O_CREAT|O_TRUNC, 0644)) >= 0) {
#ifdef	DEBUG
			PRINTF("writing file \"%s\" (%d)\n", FDname, FDlast);
#endif	DEBUG
			(void)write(fid, (char *)&FDlast, sizeof(FDlast));
			(void)write(fid, ftree, (int)((FDlast+1) * sizeof(FTREE)));
			for (j = 0; j <= FDlast; j++)
				(void)write(fid, ftree[j].f_name, strlen(ftree[j].f_name)+1);
			(void)close(fid);
		} else if (errno != EPERM && errno != EACCES)
			failed(FDname);
	}
	(void)time(&FDtime);
	showdiff = FDdiff = 0;
	savesccs = showsccs;
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

	ft_read(".");
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

failed(s)
char	*s;
{
	perror(s);
	exit(1);
}
#endif	TEST
