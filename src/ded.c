#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)ded.c	1.2 87/11/25 09:00:00";
#endif	NO_SCCS_ID

/*
 * Title:	ded.c (directory-editor)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 *
 * Function:	Interactively display/modify unix directory structures
 *		and files.
 */

#define	MAIN
#include	"ded.h"
#include	<ctype.h>
extern	char	*getenv(),
		*strchr();

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
static	char	whoami[BUFSIZ],		/* my execution-path */
		howami[BUFSIZ];		/* my help-file */

static	char	sortc[] = "cgilnprstuwGTUvzZ";/* valid sort-keys */
					/* (may correspond with cmds) */

/************************************************************************
 *	local procedures						*
 ************************************************************************/
static
sortset(ord,opt)
{
#ifndef	Z_SCCS
	if (strchr("vzZ", opt) != 0)
		opt = '?';
#endif	Z_SCCS
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
 * Clear the work-area, and move the cursor there.
 */
to_work()
{
	markC(TRUE);
	move(mark_W + 1, 0);
	clrtobot();
	move(mark_W + 1, 0);
	refresh();
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
 * Read a command-character.  If 'repeats' is set true, permit a repeat-count
 * to be associated with the command.
 */
command(repeats)
{
int	c,
	done	= FALSE;

	count	= 0;
	while (!done) {
		c = getch();
		if (c == '\033') {	/* expect arrow-keys (patch) */
			while ((c = getch()) != '[');
			done	= TRUE;
			switch(getch()) {
			case 'A':	c = ARO_UP;	break;
			case 'B':	c = ARO_DOWN;	break;
			case 'C':	c = ARO_RIGHT;	break;
			case 'D':	c = ARO_LEFT;	break;
			default:	done = FALSE;
			}
		} else if (repeats && isdigit(c)) {
			count = (count * 10) + (c - '0');
		} else
			done = TRUE;
	}
	if (!count) count = 1;
	return(c);
}

/*
 * Sound audible alarm
 */
beep()
{
	putchar('\007');
}

/*
 * Show a "blip" while (re)stating files, etc.
 */
blip(c)
{
	putchar(c);
	fflush(stdout);
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
	clrtobot();
	showC();
}

#ifdef	Z_SCCS
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
#endif	Z_SCCS

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
 * patch: this kludge bypasses bug in Apollo curses
 */
retouch(row)
{
chtype	*s;

	resetty();	/* ...otherwise "blank" lines aren't cleared */
	touchwin(stdscr);
	touchwin(curscr);
	while (row < LINES) {	/* force "touchwin()" to work! */
		for (s = stdscr->_y[row]; *s; *s++ = '?');
		for (s = curscr->_y[row]; *s; *s++ = '*');
		row++;
	}
	showFILES();
	rawterm();
}

static
restat()		/* re-'stat()' the current line */
{
	statLINE(curfile);
	showLINE(curfile);
	showC();
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
register char	*s, *d;
	for (s = flist[j].name, d = nbfr; *s; s++) {
	register int c = *s;
		if(iscntrl(c)
		|| isspace(c)
		|| (c == '$')
		|| (c == '\\')
		|| (c == '>')
		|| (c == '&')
		|| (c == '#'))
			*d++ = '\\';	/* escape the nasty thing */
		*d++ = c;
	}
	*d = EOS;
	return (nbfr);
}

/*
 * Adjust mtime-field so that chmod, chown do not alter it.
 * This fixes Apollo kludges!
 */
static
fixtime(j)
{
#ifdef	SYSTEM5
struct { time_t x, y; } tp;
	tp.x = flist[j].s.st_atime + 1;
	tp.y = flist[j].s.st_mtime + 1;
	if (utime(flist[j].name, &tp) < 0)	warn("utime");
#else	SYSTEM5
time_t	tv[2];
	tv[0] = flist[j].s.st_atime + 1;
	tv[1] = flist[j].s.st_mtime + 1;
	if (utime(flist[j].name, tv) < 0)	warn("utime");
#endif	SYSTEM5
}

/*
 * edit protection-code for current & tagged files
 */
#define	CHMOD(n)	(flist[n].s.st_mode & 07777)

static
editprot()
{
int	y	= file2row(curfile),
	x	= cmdcol[0],
	c,
	rwx	= (P_opt ? 1 : 3),
	done	= FALSE;

	if (Xbase > 0) {
		Xbase = 0;
		showFILES();
	}

	while (!done) {
		showLINE(curfile);
		move(y, x);
		refresh();
		switch (c = command(FALSE)) {
		case P_cmd:
			c = CHMOD(curfile);
			for (x = 0; x < numfiles; x++) {
				if (flist[x].flag || x == curfile) {
					statLINE(x);
					if (c != CHMOD(x)) {
						if (chmod(flist[x].name, c) < 0) {
							warn(flist[x].name);
							break;
						}
						fixtime(x);
						statLINE(x);
					}
					showLINE(x);
				}
			}
		case 'q':
			done = TRUE;
			break;
		case ARO_RIGHT:
		case ' ':
			if (x < (cmdcol[0] + (rwx+rwx)))
				x += rwx;
			else
				beep();
			break;
		case ARO_LEFT:
		case '\b':
			if (x > cmdcol[0]) {
				x -= rwx;
			} else
				beep();
			break;
		default:
			if (c >= '0' && c <= '7') {
			int	shift = 6 - (x-cmdcol[0]) * (P_opt ? 3 : 1);
				cSTAT.st_mode &= ~(7      << shift);
				cSTAT.st_mode |= ((c-'0') << shift);
				if (x < cmdcol[0] + (rwx+rwx))
					x += rwx;
			} else
				beep();
		}
	}
	restat();
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
	insert	= FALSE,
	done	= FALSE;

	while (!done) {
	int	delete = -1;
		move(y,col);
		if (!insert)	standout();
		printw("%-*.*s", len, len, bfr);
		if (!insert)	standend();
		move(y,x+col);
		refresh();
		switch (c = command(FALSE)) {
		case ARO_LEFT:	if (x > 0)		x--;	break;
		case ARO_RIGHT:	if (x < strlen(bfr))	x++;	break;
		case ARO_UP:
		case ARO_DOWN:	insert = !insert;		break;
		case '\b':	delete = x-1;			break;
		case '\177':	delete = x;			break;
		case '\n':
		case '\r':	bfr[x] = EOS;			break;
		default:
			if (insert) {
				if (isascii(c) && isprint(c)) {
				int	d,j = 0;
					do {
						d = c;
						c = bfr[x+j];
					} while (bfr[x+(j++)] = d);
					bfr[len] = EOS;
					if (x < len)	x++;
				} else
					beep();
			} else {
				if (c == 'q' || c == endc)
					done = TRUE;
				else
					beep();
			}
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
int	uid;
char	bfr[UIDLEN+1];

	if (G_opt) {
		G_opt = FALSE;
		showFILES();
	}
	edittext('u', cmdcol[1], UIDLEN, strcpy(bfr, uid2s(cSTAT.st_uid)));
	if ((uid = s2uid(bfr)) >= 0) {
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
				showLINE(j);
			}
		}
	}
	restat();
}

/*
 * Change file's group.
 */
static
edit_gid()
{
register int j;
int	gid,
	root	= (getuid() == 0);
char	bfr[BUFSIZ];

	if (!G_opt) {
		G_opt = TRUE;
		showFILES();
	}
	edittext('g', cmdcol[1], UIDLEN, strcpy(bfr, gid2s(cSTAT.st_gid)));
	if ((gid = s2gid(bfr)) >= 0) {
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
					sprintf(bfr, "chgrp -f %s %s",
						gid2s(gid),
						fixname(j));
					system(bfr);
				}
				fixtime(j);
				if (!root)
					statLINE(j);
				showLINE(j);
				if (flist[j].s.st_gid != gid) {
					beep();
					break;
				}
			}
		}
	}
	restat();
}

/*
 * Spawn a subprocess, wait for completion.
 * patch: should parse for options a la 'bldarg()'.
 */
static
forkfile(arg0)
char	*arg0;
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
		retouch(0);
		restat();
	} else if (pid < 0) {
		printf("fork failed\n");
	} else {
		execl(arg0, arg0, cNAME, 0L);
		exit(0);		/* just in case exec-failed */
	}
}

/************************************************************************
 *	main program							*
 ************************************************************************/

usage()
{
	fprintf(stderr, "usage: ded [-IGS] [-[s|r][%s]] [filespecs]\n", sortc);
}

main(argc, argv)
char	*argv[];
{
extern	int	optind;
extern	char	*optarg;

#include	"version.h"

struct	stat	sb;
register j;
int	c,
	lastc	= '?',
	quit	= FALSE;

	(void)printf("%s\r\n", version+4);	/* show me when entering process */
	(void)fflush(stdout);
	(void)sortset('s', 'n');
	getcwd(old_wd, sizeof(old_wd)-2);

	/* find which copy I am executing from, for future use */
	which(whoami, sizeof(whoami), argv[0], old_wd);
	sprintf(howami, "%s.hlp", whoami);

	while ((c = getopt(argc, argv, "GIPSUZr:s:z")) != EOF) switch (c) {
	case 'G':	G_opt = !G_opt;	break;
	case 'I':	I_opt = !I_opt;	break;
	case 'P':	P_opt = !P_opt;	break;
	case 'S':	S_opt = !S_opt;	break;
	case 'U':	U_opt = !U_opt;	break;
#ifdef	Z_SCCS
	case 'Z':	Z_opt = 1;	break;
	case 'z':	Z_opt = -1;	break;
#endif	Z_SCCS
	case 's':
	case 'r':	if (!sortset(c,*optarg))	usage();
			break;
	default:	usage();
			exit(1);
	}

	(void)initscr();
	rawterm();

	/* patch: should trim repeated items from arg-list */
	argv += optind;
	argc -= optind;
	if (!dedscan(argc, argv))	exit(0);

	mark_W = (LINES/2);
	Xbase = Ybase = 0;
	Xscroll = (COLS/3);
	dedsort();
	curfile = 0;
	showFILES();

	while (!quit) { switch (c = command(TRUE)) {
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

			/* display-toggles */
	case 'G':	G_opt = !G_opt; showFILES(); break;
	case 'I':	I_opt = !I_opt; showFILES(); break;
	case 'P':	P_opt = !P_opt; showFILES(); break;
	case 'S':	S_opt = !S_opt; showFILES(); break;
	case 'U':	U_opt = !U_opt; showFILES(); break;

#ifdef	Z_SCCS
	case 'V':	/* toggle sccs-version display */
			showSCCS();
			V_opt = !V_opt;
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
#endif	Z_SCCS

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

	case 'R':	/* restat display-list */
			to_work();
			tag_count = 0;
			if (!(quit = !dedscan(argc, argv))) {
				curfile = 0;	/* numfiles may be less now */
				dedsort();
				Ybase = curfile = 0;
				viewset();	/* scroll to first screen */
				showFILES();
			}
			break;

	case 'W':	/* restat window */
			for (j = Ybase; j <= Ylast; j++)
				statLINE(j);
			showFILES();
			break;

	case 'w':	/* refresh window */
			retouch(0);
			break;

	case 'l':	/* restat line */
			restat();
			break;

	case ' ':	/* clear workspace */
			retouch(mark_W+1);
			break;

	case 'r':
	case 's':	if (sortset(c,command(FALSE))) {
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
				showLINE(curfile);
				if (curfile < numfiles)
					downLINE(1);
				else
					break;
			}
			break;

	case '-':	while (count-- > 0) {
				if (tag_count) {
					cFLAG = FALSE;
					tag_count--;
				}
				showLINE(curfile);
				if (curfile < numfiles)
					downLINE(1);
				else
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

	case 'e':
	case 'v':	/* enter new process with current file */
			if (stat(cNAME, &sb) < 0) {
				warn(cNAME);
			} else if (isDIR(sb.st_mode)) {
				to_work();
				forkfile(whoami);
			} else
				forkfile(c == 'e' ? ENV(EDITOR)
						  : ENV(BROWSE));
			break;

	case 'm':	to_work();
			forkfile(ENV(PAGER));
			break;

			/* page thru files in work area */
	case 'h':	dedtype(howami,FALSE);
			c = 't';	/* force work-clear if 'q' */
			break;
	case 't':
	case 'T':	dedtype(cNAME,(c == 'T'));
			c = 't';	/* force work-clear if 'q' */
			break;

	case '%':
	case '!':
	case '.':	/* execute shell command */
			deddoit(c);
			break;

	case '/':
	case '?':
	case 'n':
	case 'N':	/* execute a search command */
			dedfind(c);
			break;

			/* patch: not implemented */
	case ':':	/* edit last shell command */
	case '*':	/* display last shell command */
	case 'X':	/* split/join screen (1 or 2 viewports) */
	case 'D':	/* toggle directory/filelist mode */
	case 'E':	/* enter new directory on ring */
	case 'F':	/* move forward in directory-ring */
	case 'B':	/* move backward in directory-ring */

	default:	beep();
	}; lastc = c; }
	endwin();
	exit(0);
	/*NOTREACHED*/
}
