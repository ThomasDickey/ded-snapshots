#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)ftree.c	1.5 87/09/15 11:59:21";
#endif

/*
 * Author:	T.E.Dickey
 * Created:	02 Sep 1987
 * Modified:
 *
 * Function:	This module performs functions supporting a file-tree display.
 *		We show the names of directories in tree-form.
 *
 * Configure:	DEBUG	- dump a logfile in readable form at the end
 *		TEST	- make a standalone program (otherwise, part of 'fl')
 */

#include	<ptypes.h>

#include	<ctype.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/dir.h>
#include	<sys/errno.h>
#include	<sys/stat.h>
extern	int	errno;
extern	char	*getcwd(),
		*getenv(),
		*strcat(),
		*strchr(),
		*strcpy();

#include	"screen.h"
extern	char	*denode(),
		*doalloc();

#define	MAXPATHLEN	BUFSIZ

#define	TOSHOW	(LINES-1)	/* lines to show on a screen */

#define	PRINTF	(void)printf
#define	FPRINTF	(void)fprintf

#define	NORMAL	0
#define	MARKED	1
#define	VISITED	2
#define	NOSCCS	4	/* set to disable viewing sccs-directories */
#define	NOVIEW	8	/* set to disable viewing of a tree */

/************************************************************************
 *	Public data							*
 ************************************************************************/

/************************************************************************
 *	Private data							*
 ************************************************************************/

typedef	struct	{
	int	f_root;			/* array-index of entry's parent */
	int	f_mark;			/* removal/visited flags	*/
	char	f_name[DIRSIZ];		/* name of directory		*/
	} FTREE;

static	char	FDnode[MAXPATHLEN],	/* nodename, if any (stripped)	*/
		FDname[MAXPATHLEN];	/* name of user's database	*/
static	int	FDdiff,			/* number of changes made	*/
		FDlast,			/* last used-entry in ftree[]	*/
		FDsize,			/* current sizeof(ftree[])	*/
		showbase,		/* base of current display	*/
		showlast,		/* last line in current display	*/
		showdiff = -1,		/* controls re-display		*/
		savesccs,		/* original state of 'showsccs'	*/
		showsccs = TRUE;	/* control display of 'sccs'	*/
static	FTREE	*ftree;			/* array of database entries	*/

/************************************************************************
 *	Database Manipulation						*
 ************************************************************************/

/*
 * Ensure that the database has allocated enough space for the current entry.
 */
static
fd_alloc()
{
	if (FDlast >= FDsize) {
	register int	size = FDsize;
		FDsize += FDlast + 2;
		ftree = (FTREE *)doalloc((char *)ftree, FDsize * sizeof(FTREE));
		while (size < FDsize) {
		char	*s = (char *)&ftree[size];
		int	len = sizeof(FTREE);
			while (len--) *s++ = '\0';
			size++;
		}
	}
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
		(void)strcat(strcat(strcpy(bfr, "/"), ftree[node].f_name), tmp);
	}
	while (node = ftree[node].f_root);
	return(bfr);
}

/*
 * Prepare a pathname (i.e., make it standardized, absolute)
 */
static
char *
fd_prep(path)
char	*path;
{
register char *s, *d;

	path = denode(path, FDnode, (int *)0);
	if (*path == '/')
		;
	else if (*path)
		failed("relative path?");

	/* trim out repeated '/' marks */
	for (s = d = path; *s; s++) {
		if (*s == '/')
			while (s[1] == '/')	s++;
		*d++ = *s;
	}
	*d = '\0';
	return(path);
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
register int j;
register FTREE *f;

	path = fd_prep(path);

	/* put this into the database, if it is not already */
	while (*path == '/') {
	char	*name = ++path,
		*nextpath = strchr(name, '/');
		if (nextpath != 0)
			*nextpath = '\0';

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
			(void)strcpy (ftree[this].f_name, name);
		}

		last = this;
		ftree[last].f_mark &= ~MARKED;

		if (nextpath != 0) {
			*nextpath = '/'; /* restore the one we knocked out */
			path = nextpath;
		} else
			break;
	}
}

/*
 * Locate the node corresponding to a particular path.
 */
ft_find(path)
char	*path;
{
register int j, this, last = 0;

	path = fd_prep(path);
	while (*path == '/') {
	char	*name = ++path,
		*next = strchr(path, '/');
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
			*next = '/';
			path = next;
		}
		if (this) {
			last = this;
		} else {
			failed(path);
			/*NOTREACHED*/
		}
	}
	return(last);
}

/*
 * Mark nodes below a given path in the database for removal, unless they
 * are added back before the database is written out.  The pathname argument
 * is in absolute form.
 */
ft_remove(path)
char	*path;
{
int	last = ft_find(path);
	if (last) ftree[last].f_mark |= MARKED;
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
 */
ft_init()
{
struct	stat sb;
register int j;
int	fid, len = -1;
unsigned size;
char	cwdpath[MAXPATHLEN];

	/* read the current database */
	(void)strcat(strcpy(FDname, getenv("HOME")), "/.ftree");
	if ((fid = open(FDname, O_RDONLY)) >= 0) {
		if (stat(FDname, &sb) < 0)
			failed("stat \".ftree\"");
		size = sb.st_size;
		if (size % sizeof(FTREE))
			failed("? size error");
		else {
			FDlast = (size / sizeof(FTREE)) - 1;
			fd_alloc();
			if (read(fid, (char *)ftree, size) != size)
				failed("read \".ftree\"");
		}
		(void)close(fid);
	}
	FDdiff = 0;

	/* append the current directory to the list */
	(void)strcpy(cwdpath, getcwd(FDnode, sizeof(FDnode)-2));
	ft_insert(denode(cwdpath, FDnode, &len));

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
ft_show(path, node, level)
char	*path;
{
register int j, k;
int	row = 0;

	move(row++,0);
	node = limits(showbase, node);
	printw("path: %.*s", COLS-8, path);
	clrtoeol();
	if (showdiff != FDdiff) {
		for (j = showbase; j <= showlast; j++) {
			if (!fd_show(j)) continue;
			move(row++,0);
			printw("%4d. ", j);
			for (k = fd_level(j); k > 0; k--)
				printw(fd_line(k));
			printw("%.*s/%c",
				DIRSIZ, ftree[j].f_name,
				ftree[j].f_mark ? '*' : ' ');
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
	if (on)	ftree[node].f_mark |= flag;
	else	ftree[node].f_mark &= ~flag;
	FDdiff++;
}

/*
 * Interactively display the directory tree and modify it.  The return value
 * is used by the coroutine as the next command.
 */
ft_view()
{
static
int	row,
	lvl,
	count	= 0,
	c;
char	cwdpath[MAXPATHLEN];
register int j;

	/* set initial position */
	(void)limits(showbase,showbase);
	row = ft_find(getcwd(cwdpath,sizeof(cwdpath)-2));
	lvl = fd_level(row);
	while (row > showlast) (void)forward(1);

	/* process commands */
	for (;;) {
	int	num = count ? count : 1;

		c = fd_level(row) + 1;
		if (c < lvl) lvl = c;	/* loosely drag down level */
		row = ft_show(fd_path(cwdpath,row), row, lvl);

		switch(c = getch()) {
		/* Ordinary cursor movement */
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
		case 'j':	row = downrow(row,num,lvl);	break;
		case 'k':	row = uprow(row,num,lvl);	break;
		case 'l':	lvl += num;			break;

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

		/* Exit from this program (back to 'fl') */
#ifndef	TEST
		case 'E':
		case 'e':
			if (chdir(fd_path(cwdpath,row)) < 0) {
				beep();
				break;
			}
#endif	TEST
		case 'D':
		case 'q':
			return(c);

		/* Scan/delete nodes */
		case 'i':	ft_scan(row);			break;
		case 'd':	markit(row,MARKED,TRUE);	break;
		case 'u':	markit(row,MARKED,FALSE);	break;
		case 'p':	ft_purge();			break;

		/* Screen refresh */
		case 'w':	showdiff = -1;
				(void)touchwin(curscr);
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
			showsccs = !showsccs;
			for (j = 0; j <= FDlast; j++) {
			register FTREE *f = &ftree[j];
				if (!strcmp(f->f_name,"sccs")) {
					f->f_mark ^= NOSCCS;
					showdiff = -1;
				}
			}
			(void)limits(showbase,showbase);
			while (row > showlast) (void)forward(1);
			while (row < showbase) (void)backward(1);
			break;

		default:
			if (isdigit(c))
				count = (count * 10) + (c - '0');
			else
				beep();
		}
		if (!isdigit(c))	count = 0;
	}
}

#ifdef	DEBUG
ft_dump()
{
FILE	*fp = fopen("ftree.log", "w");
register FTREE *f;
register int j, k;
	if (fp) {
		PRINTF("writing log file\n");
		FPRINTF(fp, "Total nodes: %d\n", FDlast);
		for (j = 0; j <= FDlast; j++) {
			f = &ftree[j];
			FPRINTF(fp, "%3d ^%03d", j, f->f_root);
			for (k = fd_level(j); k > 0; k--)
				FPRINTF(fp, "|---");
			FPRINTF(fp, "%.*s/", DIRSIZ, f->f_name);
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
struct	direct	d;
struct	stat	sb;
int	fid;
char	old[MAXPATHLEN],
	bfr[MAXPATHLEN], *s = bfr;

	(void)getcwd(old, sizeof(old)-2);
	s += strlen(fd_path(bfr,node));

	if (chdir(bfr) < 0)
		perror(bfr);
	else if ((fid = open(bfr, O_RDONLY)) >= 0) {
		while (read(fid, &d, sizeof(d)) == sizeof(d)) {
			sprintf(s, "/%.*s", DIRSIZ, d.d_name);
			if (dotname(s+1))		continue;
			if (stat(s+1, &sb) < 0)		continue;
			if ((int)sb.st_ino <= 0)	continue;
#define	isDIR(m)	((S_IFMT & m) == S_IFDIR)
			if (!isDIR(sb.st_mode))		continue;
			ft_insert(bfr);
		}
		(void)close(fid);
	}
	chdir(old);
}

/*
 * If any changes to the file-tree database have occurred, update the copy
 * in the user's home directory.  If we cannot write back to the user's
 * directory, no matter, since we mustn't use root privilege for this!
 */
ft_done()
{
	if (FDdiff || (savesccs != showsccs)) {
	int	fid;
#ifdef	DEBUG
		ft_dump();
#endif	DEBUG
		if ((fid = open(FDname, O_WRONLY|O_CREAT|O_TRUNC, 0644)) >= 0) {
#ifdef	DEBUG
			PRINTF("writing file \"%s\" (%d)\n", FDname, FDlast);
#endif	DEBUG
			(void)write(fid, ftree, (FDlast+1) * sizeof(FTREE));
			(void)close(fid);
		} else if (errno != EPERM)
			failed(FDname);
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
char	*s;

	ft_init();
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
	(void) initscr();
	setterm (1,-1,-1,-1);	/* raw; noecho; nonl; nocbreak; */
	(void)ft_view();
	move(LINES-1,0);
	refresh();
	PRINTF("\n");
	endwin();
	ft_done();
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
