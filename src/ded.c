#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)ded.c	1.30 88/06/06 12:24:40";
#endif	NO_SCCS_ID

/*
 * Title:	ded.c (directory-editor)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		06 Jun 1988, if 'R' finds nothing, do 'F' to recover before 'q'.
 *		01 Jun 1988, added 'Y' toggle, y-sort.
 *		25 May 1988, fix 'edittext()' for left/right scroll position.
 *			     also, +/- update via 'showDOWN()'.
 *		23 May 1988, use 'setmtime()', corrected masking in 'replay()'.
 *		13 May 1988, provide walkback on Apollo after 'failed()'.
 *		11 May 1988, use 'rename()' if it is available.
 *		03 May 1988, added 'P', 's', 't' subcommands to editprot.
 *		02 May 1988, forgot to allow arrow keys in insert-mode of
 *			     'edittext()'.
 *		28 Apr 1988, added 'new_args()' to integrate with ftree module.
 *			     Modified 'edittext()' to correspond with rawgets.
 *			     Added "=" renaming command.
 *		21 Apr 1988, use 'savewin()' module for retouch.  Apollo SR9.7
 *			     changes 'fixtime()'.  Added call on 'resizewin'.
 *		25 Mar 1988, corrected 'retouch()' so that last line is cleared,
 *			     and so we don't have to change terminal-mode.
 *			     Added ':' command, and began interface to 'ftree'.
 *		24 Mar 1988, to provide interface to 'ftree.c' module.
 *
 * Function:	Interactively display/modify unix directory structures
 *		and files.
 */

#define	MAIN
#include	"ded.h"
#include	<sys/signal.h>
extern	char	*strchr();

#ifndef	EDITOR
#define	EDITOR	"/usr/ucb/vi"
#endif	EDITOR

#ifndef	BROWSE
#define	BROWSE	"/usr/ucb/view"
#endif	BROWSE

#ifndef	PAGER
#define	PAGER	"/usr/ucb/more"
#endif	PAGER

#define	P_cmd	'p'
#define	file2row(n)	((n) - Ybase + Yhead + 1)

#ifdef	lint
#undef	putchar
#endif	lint

/*
 * Per-viewport main-module state:
 */
static	int	Yhead = 0;		/* first line of viewport */
static	int	Xbase, Ybase;		/* viewport (for scrolling) */
static	int	Xscroll;		/* amount by which to left/right */
static	int	Ylast;			/* last visible file on screen */
static	int	tag_count;		/* number of tagged files */

/*
 * Other, private main-module state:
 */
static	int	in_screen;		/* TRUE if we have successfully init'ed */
static	char	whoami[BUFSIZ],		/* my execution-path */
		howami[BUFSIZ];		/* my help-file */

static	char	sortc[] = ".cgilnprstuwGTUyvzZ";/* valid sort-keys */
					/* (may correspond with cmds) */

static	int	re_edit;		/* flag for 'edittext()' */
static	char	lastedit[BUFSIZ];	/* command-stream for 'edittext()' */

/************************************************************************
 *	local procedures						*
 ************************************************************************/
static
sortset(ord,opt)
{
#ifndef	Z_RCS_SCCS
	if (strchr("vyzZ", opt) != 0)
		opt = '?';
#endif	Z_RCS_SCCS
	if (strchr(sortc, opt) != 0) {
		dateopt = opt == 'c'  ? 1 : (opt == 'r' ? 0 : 2);
		sortopt = opt;
		sortord = (ord == 'r');
		return(TRUE);
	}
	return(FALSE);
}

static
viewset()
{
	Ylast = mark_W + Ybase - (Yhead + 2);
	if (Ylast >= numfiles)	Ylast = numfiles-1;
}

/************************************************************************
 *	public procedures						*
 ************************************************************************/

/*
 * Exit from window mode
 */
to_exit(last)
{
	if (in_screen) {
		if (last) {
			move(LINES-1,0);
			clrtoeol();
			refresh();
		}
		resetty();
		endwin();
	}
}

/*
 * Clear the work-area, and move the cursor there.
 */
clear_work()
{
	move(mark_W + 1, 0);
	clrtobot();
	move(mark_W + 1, 0);
	refresh();
}

/*
 * Clear the work area, move the cursor there after setting marker
 */
to_work()
{
	markC(TRUE);
	clear_work();
}

/*
 * Scroll, as needed, to put current-file in the window
 */
to_file()
{
int	code	= ((curfile < Ybase)
		|| (curfile > Ylast));
	while (curfile > Ylast)	forward(1);
	while (curfile < Ybase)	backward(1);
	return(code);
}

/*
 * Move the workspace marker
 */
markset(num)
{
	mark_W = num;
	if (mark_W < 2)		mark_W = 2;
	if (mark_W > LINES-2)	mark_W = LINES-2;
	viewset();			/* update things dependent */
	(void)to_file();
	showFILES();
}

/*
 * Determine if the given entry is a file, directory or none of these.
 */
realstat(inx)
{
register j = flist[inx].s.st_mode;

#ifndef	SYSTEM5
	if (isLINK(j)) {
	struct	stat	sb;
		j = (stat(flist[inx].name, &sb) >= 0) ? sb.st_mode : 0;
	}
#endif	SYSTEM5
	if (isFILE(j))	return(0);
	if (isDIR(j))	return(1);
	return (-1);
}

/*
 * Show a "blip" while (re)stating files, etc.
 */
blip(c)
{
	if (putchar(c) != EOF)
		(void)fflush(stdout);
}

/*
 * Print an error/warning message
 */
dedmsg(msg)
char	*msg;
{
	move(LINES-1,0);
	printw("** %s", msg);
	clrtoeol();
	showC();
}

warn(msg)
char	*msg;
{
extern	int	errno;
extern	char	*sys_errlist[];
	move(LINES-1,0);
	printw("** %s: %s", msg, sys_errlist[errno]);
	clrtoeol();
	showC();
}

/*
 * Wait for the user to hit a key before the next screen is shown.  This is
 * used when we have put a message up and may be going back to the
 * directory tree display.
 */
waitmsg()
{
	move(LINES-1,0);
	refresh();
	(void)cmdch((int *)0);	/* pause beside error message */
}

/*
 * Fatal-error exit from this process
 */
failed(msg)
char	*msg;
{
	to_exit(msg != 0);
	if (msg)
		PRINTF("-------- \n?? %-79s\n-------- \n", msg);
#ifdef	apollo
	if (msg) {
		(void)kill(getpid(), SIGKILL);
		for (;;);	/* when terminated, will be able to use 'tb' */
	}
#else	apollo
	exit(1);
#endif	apollo
}

/*
 * Move the cursor up/down the specified number of lines, scrolling
 * to a new screen if necessary.
 */
upLINE(n)
{
	curfile -= n;
	if (curfile < 0)		curfile = 0;
	if (curfile < Ybase) {
		while (curfile < Ybase)	backward(1);
		showFILES();
	} else
		showC();
}

downLINE(n)
{
	curfile += n;
	if (curfile >= numfiles)	curfile = numfiles-1;
	if (curfile > Ylast) {
		while (curfile > Ylast)	forward(1);
		showFILES();
	} else
		showC();
}

showDOWN()
{
	showLINE(curfile);
	if (curfile < numfiles-1)
		downLINE(1);
	else {
		showC();
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Recompute viewport line-limits for forward/backward scrolling
 */
forward(n)
{
	while (n-- > 0) {
		if (Ylast < (numfiles-1)) {
			Ybase = Ylast + 1;
			viewset();
		} else
			break;
	}
}

backward(n)
{
	while (n-- > 0) {
		if (Ybase > 0) {
			Ybase -= (mark_W - Yhead - 1);
			if (Ybase < 0)	Ybase = 0;
			viewset();
		} else
			break;
	}
}

/*
 * Display the given line.  If it is tagged, highlight the name.
 */
showLINE(j)
{
int	k = file2row(j),
	col, len;
char	bfr[BUFSIZ];

	if (j >= Ybase && j <= Ylast) {
		move(k,0);
		ded2s(j, bfr, sizeof(bfr));
		if (Xbase < strlen(bfr)) {
			printw("%.*s", COLS-1, &bfr[Xbase]);
			if (flist[j].flag) {
				col = cmdcol[3] - Xbase;
				len = (COLS-1) - col;
				if (len > 0) {
					move(k, col);
					standout();
					printw("%.*s", len, &bfr[cmdcol[3]]);
					standend();
				}
			}
		}
		clrtoeol();
	}
}

/*
 * Display all files in the current viewport
 */
showFILES()
{
register int j;

	viewset();		/* set 'Ylast' as function of mark_W */

	for (j = Ybase; j <= Ylast; j++)
		showLINE(j);
	for (j = file2row(Ylast+1); j < mark_W; j++) {
		move(j,0);
		clrtoeol();
	}
	move(mark_W,0);
	for (j = 0; j < COLS - 1; j += 10)
		printw("%.*s", COLS - j - 1, "----:----+");
	move(mark_W+1,0);
	clrtobot();
	showC();
}

#ifdef	Z_RCS_SCCS
showSCCS()
{
register int j;
	if (!Z_opt) {		/* catch up */
		to_work();
		Z_opt = -1;
		for (j = 0; j < numfiles; j++)
			if (!flist[j].z_time) {
				statSCCS(flist[j].name, &flist[j]);
				blip('*');
			}
	}
}
#endif	Z_RCS_SCCS

/*
 * Set the cursor to the current file, noting this in the viewport header.
 */
showC()
{
int	x = cmdcol[2] - Xbase;
static	char	datechr[] = "acm";

	if (x < 0)		x = 0;
	if (x > COLS-1)		x = COLS-1;
	move(Yhead,0);
	if (tag_count)	standout();
	printw("%2d of %2d [%ctime] %", curfile+1, numfiles, datechr[dateopt]);
	printw("%.*s", COLS-((stdscr->_curx)+2), new_wd);
	if (tag_count)	standend();
	clrtoeol();
	markC(FALSE);
	move(file2row(curfile), x);
	refresh();
}

/*
 * Flag the current-file in the display (i.e., when leaving the current
 * line for the work-area).
 */
markC(on)
{
int	col = cmdcol[2] - Xbase;

	if (col >= 0) {
		move(file2row(curfile), col);
		addch(on ? '*' : ' ');
	}
}

/*
 * Repaint the screen
 */
retouch(row)
{
int	y,x;
#ifdef	apollo
	if (resizewin()) {
		markset(mark_W);
		showFILES();
		return;
	}
#endif	apollo
	getyx(stdscr,y,x);
	move(mark_W+1,0);
	clrtobot();
	move(y,x);
	savewin();
	unsavewin(TRUE,row);
}

static
restat(group)		/* re-'stat()' the current line, and optionally group */
{
	if (group) {
	register int j;
		for (j = 0; j < numfiles; j++) {
			if (j != curfile) {
				if (flist[j].flag) {
					statLINE(j);
					showLINE(j);
				}
			}
		}
	}
	statLINE(curfile);
	showLINE(curfile);
	showC();
}

/*
 * Use the 'dedring()' module to switch to a different file-list
 */
static
new_args(path, cmd, count)
char	*path;
{
register int j;
int	ok;

	clear_work();
	if (ok = dedring(path, cmd, count)) {
		(void)to_file();
		for (j = tag_count = 0; j < numfiles; j++)
			if (flist[j].flag)
				tag_count++;
		showFILES();
	} else
		beep();
	(void)chdir(new_wd);
	return (ok);
}

/*
 * Invoke a new file-list from the directory-tree display, cleaning up if
 * it fails.
 */
static
new_tree(path, cmd, count)
char	*path;
{
	if (new_args(path, cmd, count))
		return (TRUE);
	(void)strcpy(path, new_wd);
	waitmsg();
	return (FALSE);
}

/*
 * Convert a name to a form which shell commands can use.  For most
 * names, this is simply a copy of the original name.  However, on
 * Apollo, we may have names with '$' and other special characters.
 */
char *
fixname(j)
{
static	char	nbfr[BUFSIZ];
	(void)name2s(nbfr, flist[j].name, sizeof(nbfr), TRUE);
	return (nbfr);
}

/*
 * Adjust mtime-field so that chmod, chown do not alter it.
 * This fixes Apollo kludges!
 */
static
fixtime(j)
{
	if (setmtime(flist[j].name, flist[j].s.st_mtime) < 0)	warn("utime");
}

/*
 * Store/retrieve field-editing commands.  The first character of the buffer
 * 'lastedit[]' is reserved to tell us what the command was.
 */
static
replay(cmd)
{
int	c;
static	int	in_edit;

	if (cmd) {
		in_edit = 1;
		lastedit[0] = cmd;
	} else {
		if (re_edit) {
			c = lastedit[in_edit++];
		}
		if (!re_edit) {
			c = cmdch((int *)0);
			if (c == '\r') c = '\n';
			lastedit[in_edit++] = c;
			lastedit[in_edit]   = EOS;
		}
	}
	return (c & 0xff);
}

/*
 * edit protection-code for current & tagged files
 */
#define	CHMOD(n)	(flist[n].s.st_mode & 07777)

static
editprot()
{
int	y	= file2row(curfile),
	x	= 0,
	c,
	opt	= P_opt,
	changed	= FALSE,
	done	= FALSE;

	if (Xbase > 0) {
		Xbase = 0;
		showFILES();
	}

	(void)replay(P_cmd);

	while (!done) {
	int	rwx,
		cols[3];

		showLINE(curfile);

		rwx	= (P_opt ? 1 : 3),
		cols[0] = cmdcol[0];
		cols[1] = cols[0] + rwx;
		cols[2] = cols[1] + rwx;

		move(y, cols[x]);
		if (!re_edit) refresh();
		switch (c = replay(0)) {
		case P_cmd:
			c = CHMOD(curfile);
			for (x = 0; x < numfiles; x++) {
				if (flist[x].flag || x == curfile) {
					statLINE(x);
					changed++;
					if (c != CHMOD(x)) {
						if (chmod(flist[x].name, c) < 0) {
							warn(flist[x].name);
							break;
						}
						fixtime(x);
					}
				}
			}
			done = TRUE;
			break;
		case 'q':
			lastedit[0] = EOS;
			done = TRUE;
			break;
		case ARO_RIGHT:
		case ' ':
			if (x < 2)	x++;
			else		beep();
			break;
		case ARO_LEFT:
		case '\b':
			if (x > 0)	x--;
			else		beep();
			break;
		default:
			if (c >= '0' && c <= '7') {
			int	shift = 6 - (x * rwx);
				cSTAT.st_mode &= ~(7      << shift);
				cSTAT.st_mode |= ((c-'0') << shift);
				if (x < 2)
					x++;
			} else if (c == 'P') {
				P_opt = !P_opt;
			} else if (c == 's') {
				if (x == 0)
					cSTAT.st_mode ^= S_ISUID;
				else if (x == 1)
					cSTAT.st_mode ^= S_ISGID;
				else
					beep();
			} else if (c == 't') {
				if (x == 2)
					cSTAT.st_mode ^= S_ISVTX;
				else
					beep();
			} else
				beep();
		}
	}
	if (opt != P_opt) {
		P_opt = opt;
		showLINE(curfile);
	}
	restat(changed);
}

/*
 * Edit a text-field on the current display line.  Use the arrow keys for
 * moving within the line, and for setting/resetting insert mode.  Use
 * backspace to delete characters.
 */
static
edittext(endc, col, len, bfr)
char	*bfr;
{
int	y	= file2row(curfile),
	x	= 0,
	c,
	ec	= erasechar(),
	kc	= killchar(),
	insert	= FALSE,
	delete;
register char *s;

	if ((col -= Xbase) < 1) {	/* convert to absolute column */
		col += Xbase;
		Xbase = 0;
		showFILES();
	}
	(void)replay(endc);

	for (;;) {
		move(y,col-1);
		printw("%c", insert ? ' ' : '^');	/* a la rawgets */
		if (!insert)	standout();
		for (s = bfr; *s; s++)
			addch(isprint(*s) ? *s : '?');
		while ((s++ - bfr) < len)
			addch(' ');
		if (!insert)	standend();
		move(y,x+col);
		if (!re_edit) refresh();

		delete = -1;
		c = replay(0);

		if (c == '\n') {
			return (TRUE);
		} else if (c == '\t') {
			insert = ! insert;
		} else if (insert) {
			if (isascii(c) && isprint(c)) {
			int	d,j = 0;
				do {
					d = c;
					c = bfr[x+j];
				} while (bfr[x+(j++)] = d);
				bfr[len] = EOS;
				if (x < len)	x++;
			} else if (c == ec) {
				delete = x-1;
			} else if (c == kc) {
				delete = x;
			} else if (c == ARO_LEFT) {
				if (x > 0)	x--;
			} else if (c == ARO_RIGHT) {
				if (x < strlen(bfr))	x++;
			} else if (c == CTL(b)) {
				x = 0;
			} else if (c == CTL(f)) {
				x = strlen(bfr);
			} else
				beep();
		} else {
			if (c == 'q') {
				lastedit[0] = EOS;
				return (FALSE);
			} else if (c == endc) {
				return (TRUE);
			} else if (c == '\b' || c == ARO_LEFT) {
				if (x > 0)	x--;
			} else if (c == '\f' || c == ARO_RIGHT) {
				if (x < strlen(bfr))	x++;
			} else if (c == CTL(b)) {
				x = 0;
			} else if (c == CTL(f)) {
				x = strlen(bfr);
			} else
				beep();
		}
		if (delete >= 0) {
			x = delete;
			while (bfr[delete] = bfr[delete+1]) delete++;
		}
	}
}

/*
 * Change file's owner.
 */
static
edit_uid()
{
register int j;
int	uid,
	changed	= FALSE;
char	bfr[UIDLEN+1];

	if (G_opt) {
		G_opt = FALSE;
		showFILES();
	}
	if (edittext('u', cmdcol[1], UIDLEN, strcpy(bfr, uid2s(cSTAT.st_uid)))
	&&  (uid = s2uid(bfr)) >= 0) {
		for (j = 0; j < numfiles; j++) {
			if (flist[j].s.st_uid == uid)	continue;
			if (flist[j].flag || (j == curfile)) {
				if (chown(flist[j].name,
					uid, flist[j].s.st_gid) < 0) {
					warn(flist[j].name);
					return;
				}
				fixtime(j);
				flist[j].s.st_uid = uid;
				changed++;
			}
		}
	}
	restat(changed);
}

/*
 * Change file's group.
 */
static
edit_gid()
{
register int j;
int	gid,
	changed	= FALSE,
	root	= (getuid() == 0);
char	bfr[BUFSIZ];

	if (!G_opt) {
		G_opt = TRUE;
		showFILES();
	}
	if (edittext('g', cmdcol[1], UIDLEN, strcpy(bfr, gid2s(cSTAT.st_gid)))
	&&  (gid = s2gid(bfr)) >= 0) {
	char	newgrp[BUFSIZ];
	static	char	*fmt = "chgrp -f %s %s";

		(void)strcpy(newgrp, gid2s(gid));
		for (j = 0; j < numfiles; j++) {
			if (flist[j].s.st_gid == gid)	continue;
			if (flist[j].flag || (j == curfile)) {
				if (root) {
					if (chown(flist[j].name,
						flist[j].s.st_uid, gid) < 0) {
						warn(flist[j].name);
						return;
					}
					flist[j].s.st_gid = gid;
				} else {
					FORMAT(bfr, fmt, newgrp, fixname(j));
					(void)system(bfr);
				}
				fixtime(j);
				if (!root) {
					statLINE(j);
					showLINE(j);
					showC();
				} else
					changed++;
				if (flist[j].s.st_gid != gid) {
					FORMAT(bfr, fmt, newgrp, fixname(j));
					dedmsg(bfr);
					beep();
					break;
				}
			}
		}
	}
	restat(changed);
}

/*
 * Change file's name
 */
static
editname()
{
int	len	= COLS - (cmdcol[3] - Xbase) - 1,
	changed	= 0;
register int	j;
char	bfr[BUFSIZ];

#define	EDITNAME(n)	edittext('=', cmdcol[3], len, strcpy(bfr, n))
	if (EDITNAME(cNAME)) {
		if (dedname(curfile, bfr) >= 0) {
			re_edit = TRUE;
			for (j = 0; j < numfiles; j++) {
				if (j == curfile)
					continue;
				if (flist[j].flag) {
					(void)EDITNAME(flist[j].name);
					if (dedname(j, bfr) >= 0)
						changed++;
					else
						break;
				}
			}
			re_edit = FALSE;
		}
	}
	restat(changed);
}

/*
 * Spawn a subprocess, wait for completion.
 * patch: should parse for options a la 'bldarg()'.
 */
static
forkfile(arg0, arg1, normal)
char	*arg0, *arg1;
{
int	pid ,
	status;

	resetty();
	if ((pid = fork()) > 0) {
	int	c;
		for (;;) {
			c = wait(&status);
			if (c < 0) break;
		}
		rawterm();
		chdir(new_wd);
		if (normal) {
			retouch(0);
			restat(FALSE);
		}
	} else if (pid < 0) {
		PRINTF("fork failed\n");
	} else {
		execl(arg0, arg0, arg1, 0L);
		exit(0);		/* just in case exec-failed */
	}
}

/*
 * Enter an editor (separate process) for the current-file/directory.
 */
/*ARGSUSED*/
static
editfile(readonly, pad)
{
	switch (realstat(curfile)) {
	case 0:
		to_work();
#ifdef	apollo
		if (pad) {
			if (padedit(cNAME, readonly))
				beep();
			restat(FALSE);
		} else
#endif	apollo
			forkfile(readonly ? ENV(BROWSE)
					  : ENV(EDITOR),
				 cNAME, TRUE);
		break;
	case 1:
		to_work();
		ft_write();
		forkfile(whoami, cNAME, TRUE);
		ft_read(new_wd);
		break;
	default:
		dedmsg("cannot edit this item");
	}
}
 
/************************************************************************
 *	main program							*
 ************************************************************************/

usage()
{
	FPRINTF(stderr, "usage: ded [-IGS] [-[s|r][%s]] [filespecs]\n", sortc);
}

main(argc, argv)
char	*argv[];
{
extern	int	optind;
extern	char	*optarg;

#include	"version.h"

register j;
int	c,
	count,
	lastc	= '?',
	quit	= FALSE;
char	tpath[BUFSIZ],
	dpath[BUFSIZ];

	PRINTF("%s\r\n", version+4);	/* show me when entering process */
	(void)fflush(stdout);
	(void)sortset('s', 'n');
	ft_read(getwd(old_wd));

	/* find which copy I am executing from, for future use */
	if (which(whoami, sizeof(whoami), argv[0], old_wd) <= 0)
		failed("which-path");
	FORMAT(howami, "%s.hlp", whoami);

	while ((c = getopt(argc, argv, "GIPSUZr:s:z")) != EOF) switch (c) {
	case 'G':	G_opt = !G_opt;	break;
	case 'I':	I_opt = !I_opt;	break;
	case 'P':	P_opt = !P_opt;	break;
	case 'S':	S_opt = !S_opt;	break;
	case 'U':	U_opt = !U_opt;	break;
#ifdef	Z_RCS_SCCS
	case 'Z':	Z_opt = 1;	break;
	case 'z':	Z_opt = -1;	break;
#endif	Z_RCS_SCCS
	case 's':
	case 'r':	if (!sortset(c,*optarg))	usage();
			break;
	default:	usage();
			exit(1);
	}

	if (!initscr())			failed("initscr");
	in_screen = TRUE;
	if (LINES > BUFSIZ || COLS > BUFSIZ) {
	char	bfr[80];
		FORMAT(bfr, "screen too big: %d by %d\n", LINES, COLS);
		failed(bfr);
	}
	rawterm();

	/* patch: should trim repeated items from arg-list */
	top_argv = argv + optind;
	top_argc = argc - optind;
	if (!dedscan(top_argc, top_argv))	failed((char *)0);

	mark_W = (LINES/2);
	Xbase = Ybase = 0;
	Xscroll = (COLS/3);
	dedsort();
	curfile = 0;
	showFILES();

	while (!quit) { switch (c = cmdch(&count)) {
			/* scrolling */
	case ARO_UP:
	case '\b':
	case 'k':	upLINE(count);
			break;

	case ARO_DOWN:
	case '\r':
	case '\n':
	case 'j':	downLINE(count);
			break;

	case 'f':	forward(count);
			curfile = Ybase;
			showFILES();
			break;

	case 'b':	backward(count);
			curfile = Ybase;
			showFILES();
			break;

	case ARO_LEFT:	if (Xbase > 0) {
				Xbase -= Xscroll;
				showFILES();
			} else
				beep();
			break;

	case ARO_RIGHT:	if (Xbase + Xscroll < cmdcol[2]) {
				Xbase += Xscroll;
				showFILES();
			} else
				beep();
			break;

			/* cursor-movement in-screen */
	case 'H':	curfile = Ybase;		showC(); break;
	case 'M':	curfile = (Ybase+Ylast)/2;	showC(); break;
	case 'L':	curfile = Ylast;		showC(); break;
	case '^':	if (Ybase != curfile) {
				Ybase = curfile;
				showFILES();
			}
			break;

			/* display-toggles */
	case 'G':	G_opt = !G_opt; showFILES(); break;
	case 'I':	I_opt = !I_opt; showFILES(); break;
	case 'P':	P_opt = !P_opt; showFILES(); break;
	case 'S':	S_opt = !S_opt; showFILES(); break;
	case 'U':	U_opt = !U_opt; showFILES(); break;

#ifdef	Z_RCS_SCCS
	case 'V':	/* toggle sccs-version display */
			showSCCS();
			V_opt = !V_opt;
			showFILES();
			break;

	case 'Y':	/* show owner of file lock */
			showSCCS();
			Y_opt = !Y_opt;
			showFILES();
			break;

	case 'Z':	/* toggle sccs-date display */
			showSCCS();
			Z_opt = -Z_opt;
			showFILES();
			break;

	case 'z':	/* cancel sccs-display */
			if (Z_opt) {
				Z_opt = 0;
				showFILES();
			}
			break;
#endif	Z_RCS_SCCS

	case 'q':	/* quit this process */
			if (lastc == 't')
				retouch(mark_W+1);
			else
				quit = TRUE;
			break;

			/* move work-area marker */
	case 'A':	count = -count;
	case 'a':
			markset(mark_W + count);
			break;

	case 'R':	/* re-stat display-list */
			to_work();
			tag_count = 0;
			(void)strcpy(tpath,cNAME);
			if (dedscan(top_argc, top_argv)) {
				curfile = 0;	/* numfiles may be less now */
				dedsort();
				curfile = 0;
				for (j = 0; j < numfiles; j++)
					if (!strcmp(flist[j].name, tpath)) {
						curfile = j;
						break;
					}
				viewset();	/* scroll to current screen */
				to_file();
				showFILES();
			} else
				quit = !new_args(strcpy(tpath, new_wd), 'F', 1);
			break;

	case 'W':	/* re-stat window */
			for (j = Ybase; j <= Ylast; j++)
				statLINE(j);
			showFILES();
			break;

	case 'w':	/* refresh window */
			retouch(0);
			break;

	case 'l':	/* re-stat line */
			restat(TRUE);
			break;

	case ' ':	/* clear workspace */
			retouch(mark_W+1);
			break;

	case 'r':
	case 's':	if (sortset(c,cmdch((int *)0))) {
				dedsort();
				(void)to_file();
				showFILES();
			} else
				beep();
			break;

	case 'C':	if (++dateopt > 2)	dateopt = 0;
			showFILES();
			break;

			/* tag/untag specific files */
	case '+':	while (count-- > 0) {
				if (!cFLAG) {
					cFLAG = TRUE;
					tag_count++;
				}
				if (!showDOWN())
					break;
			}
			break;

	case '-':	while (count-- > 0) {
				if (cFLAG) {
					cFLAG = FALSE;
					tag_count--;
				}
				if (!showDOWN())
					break;
			}
			break;

	case '_':	for (j = 0; j < numfiles; j++)
				flist[j].flag = FALSE;
			tag_count = 0;
			showFILES();
			break;

			/* edit specific fields */
	case P_cmd:	editprot();	break;
	case 'u':	edit_uid();	break;
	case 'g':	edit_gid();	break;
	case '=':	editname();	break;

	case '"':	re_edit = TRUE;
			switch (*lastedit) {
			case P_cmd:	editprot();	break;
			case 'u':	edit_uid();	break;
			case 'g':	edit_gid();	break;
			case '=':	editname();	break;
			default:	beep();
			}
			re_edit = FALSE;
			break;

#ifdef	apollo
	case CTL(e):	/* pad-edit */
	case CTL(v):	/* pad-view */
			editfile(c != CTL(e), TRUE);
			break;
#endif	apollo

	case 'e':
	case 'v':	/* enter new process with current file */
			editfile(c != 'e', FALSE);
			break;

	case 'm':	to_work();
			forkfile(ENV(PAGER), cNAME, TRUE);
#ifndef	apollo
			dedwait();
#endif	apollo
			break;

			/* page thru files in work area */
	case 'h':	dedtype(howami,FALSE);
			c = 't';	/* force work-clear if 'q' */
			break;
	case 't':
	case 'T':	if (realstat(curfile) >= 0)
				dedtype(cNAME,(c == 'T'));
			c = 't';	/* force work-clear if 'q' */
			break;

	case '%':	/* execute shell command with screen refresh */
	case '!':	/* execute shell command w/o screen refresh */
	case '.':	/* re-execute last shell command */
	case ':':	/* edit last shell command */
			deddoit(c);
			break;

	case '*':	/* display last shell command */
			dedshow("Command=", bfr_sh);
			showC();
			break;

	case '/':
	case '?':
	case 'n':
	case 'N':	/* execute a search command */
			dedfind(c);
			break;

	case 'D':	/* toggle directory/filelist mode */
			(void)strcpy(dpath, strcpy(tpath,new_wd) );
			do {
			    while ((c = ft_view(tpath)) == 'e') {
			    int	y,x;

				savewin();
				getyx(stdscr, y, x);
				if (++y >= LINES)	y = LINES-1;
				move(y, x-x);
				clrtobot();
				move(y, 0);
				refresh();
				ft_write();
				forkfile(whoami, tpath, FALSE);
				unsavewin(TRUE,0);
				ft_read(new_wd);
			    }
			} while (!new_tree(tpath, 'E', 1));
			break;

	case 'E':	/* enter new directory on ring */
			if (realstat(curfile) == 1) {
				if (!new_args(strcat(
						strcat(
							strcpy(tpath, new_wd),
							"/"),
						cNAME),
					c, 1)) {
					showC();
				}
			} else
				beep();
			break;

	case 'F':	/* move forward in directory-ring */
	case 'B':	/* move backward in directory-ring */
			(void)new_args(strcpy(tpath, new_wd), c, count);
			showC();
			break;

			/* patch: not implemented */
	case 'X':	/* split/join screen (1 or 2 viewports) */

	default:	beep();
	}; lastc = c; }

	to_exit(TRUE);
	ft_write();
	exit(0);
	/*NOTREACHED*/
}
