/*
 * Title:	dedtype.c (type files for ded)
 * Author:	T.E.Dickey
 * Created:	16 Nov 1987
 * Modified:
 *		16 Aug 1999, add cast to work with BeOS's long long ino_t.
 *		15 Feb 1998, add home/end/ppage/npage keys.
 *		13 Jan 1996, mods for scrolling regions.  Move search prompt
 *			     to bottom of work area.
 *		15 Dec 1995, allow for missing newline in file-ending.
 *		01 Nov 1995, mods to use last column on display, and to
 *			     provide single column/row scrolling.
 *		04 Jul 1994, prevent nonascii characters from echoing.
 *		09 May 1994, Linux's 'ftell()' clears eof-flag.
 *		29 Oct 1993, ifdef-ident, port to HP/UX.
 *		28 Sep 1993, gcc warnings
 *		08 Oct 1992, make search/scrolling interruptible.
 *		05 Aug 1992, added search commands, and '<', '>'.
 *		04 Aug 1992, use dynamic-buffers to allow wide files
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		12 Mar 1992, if typing a file from the filelist, update the
 *			     stat in that place.
 *		18 Oct 1991, converted to ANSI
 *		19 Jul 1991, changed interface to 'markset()'
 *		01 Jul 1991, corrected column-limit logic
 *		04 Jun 1991, forgot to reset column on successive blank-lines
 *			     that are suppressed.
 *		22 Apr 1991, added stripped-mode to make looking at binary
 *			     files easier on my eyes.
 *		01 Apr 1991, added command (tab-character) to allow user to
 *			     alter tab stops in the display.
 *		03 Mar 1990, for the special case of a directory-file, create
 *			     a temporary file with the inode-values and names
 *			     of the files.  Display this instead of the raw
 *			     contents of the directory.
 *		13 Oct 1989, trim lines containing blanks so we don't display
 *			     extra gaps
 *		14 Mar 1989, interface to 'dlog'.
 *		12 Sep 1988, suppress screen-operations during 'skip' -- faster.
 *		01 Sep 1988, break out of 'get' loop if interrupted or error.
 *		17 Aug 1988, test for error return from 'fseek()'.
 *		07 Jun 1988, added CTL(K) command.
 *		02 May 1988, fixed repeat-count for forward-command.
 *			     Dynamically allocate 'LineAt[].'
 *
 * Function:	Display a text or binary file in the workspace.  For
 *		text-files we recognize backspace and carriage-return
 *		overstrikes.  These are converted to highlights.
 *
 *		For binary files, and in text files where we encounter
 *		a non-printing character which we do not recognize, we
 *		display the character as a '.'.
 *
 * patch: should not reread page from file if we don't move.
 */

#define		DIR_PTYPES	/* includes directory-stuff */
#include	"ded.h"

MODULE_ID("$Id: dedtype.c,v 12.29 1999/08/17 01:06:17 tom Exp $")

typedef	struct	{
	OFF_T	offset;
	int	blank;
	} OFFSETS;

#define	def_doalloc	OFF_T_alloc
	/*ARGSUSED*/
	def_DOALLOC(OFFSETS)

#define WORK_HIGH   (LINES - mark_W - 2)
#define NumP(count) (WORK_HIGH * count)

#define	END_COL	(my_text->max_length)
#define	Text	dyn_string(my_text)
#define	Over	dyn_string(my_over)

extern WINDOW *newscr;

int	in_dedtype;

/******************************************************************************/
static	FILE	*InFile;
static	OFFSETS	*LineAt;
static	DYN	*my_text,		/* converted display-text */
		*my_over;		/* overstrike/underline flags */
static	REGEX_T	ToFind;			/* compiled search-expression */
static	int	OptBinary,
		OptStripped,
		UsePattern,		/* set true to cause matching */
		HadPattern,		/* set true iff we found pattern */
		Shift,			/* left/right shift-column */
		Tlen,			/* strlen(Text) */
		Tcol,			/* current column in Text[] */
		tabstop,
		was_interrupted,
		max_lines;

/******************************************************************************/
private	void	typeinit(_AR0)
{
	Text[Tlen = Tcol = 0] = EOS;
}

private	int	typeline(
	_ARX(int,	y)
	_AR1(int,	skip)
		)
	_DCL(int,	y)
	_DCL(int,	skip)
{
	int	found;

	if ((found = (UsePattern && GOT_REGEX(ToFind, Text))) != 0)
		HadPattern = TRUE;

	if (!skip) {
		move(y,0);
		Tlen	-= Shift;
		if (Tlen > COLS)
			Tlen = COLS;

		if (found) {
			standout();
			if (Tlen > 0)
				PRINTW("%.*s", Tlen, Text + Shift);
			else
				addstr(" ");
			standend();
		} else if (Tlen > 0) {
			int	now	= Shift,
				j;

			while (Tlen > 0) {
				for (j = now; j < now+Tlen; j++) {
					if (Over[j] != Over[now])	break;
				}
				if (Over[now])	standout();
				PRINTW("%.*s", (j - now), Text+now);
				if (Over[now])	standend();
				Tlen -= (j - now);
				now = j;
			}
		}
		clrtoeol();
	}
	typeinit();
	return (++y);
}

private	void	typeover (
	_AR1(int,	c))
	_DCL(int,	c)
{
	size_t	need = Tcol + 1;

	if (need >= END_COL) {
		need = (need * 5)/4;
		my_text = dyn_alloc(my_text, need);
		my_over = dyn_alloc(my_over, need);
	}

	if ((Over[Tcol] = Text[Tcol]) != EOS) {
		if (ispunct(Text[Tcol]))
			Text[Tcol] = c;
	} else
		Text[Tcol] = c;

	if (++Tcol > Tlen) {
		Tlen = Tcol;
		Text[Tlen] = EOS;
	}
}

/*
 * Convert a character to displayable format, returns true when we fill a
 * display-line.
 */
private	int	typeconv(
	_AR1(int,	c))
	_DCL(int,	c)
{
	char	dot	= OptStripped ? ' ' : '.';

	if (OptBinary) {	/* highlight chars with parity */
		if (!isascii(c)) {
			c = toascii(c);
			if (OptStripped) {
				if (!isprint(c))
					c = ' ';
			} else if (Tcol < (int)END_COL)
				Text[Tcol] = '_';
		}
	}
	if (isascii(c)) {
		if (OptBinary && !isprint(c)) {
			typeover(dot);
		} else if (c == '\b') {
			if (Tcol > 0) Tcol--;
		} else if (c == '\r') {
			Tcol = 0;
		} else if (isprint(c)) {
			typeover(c);
		} else if (isspace(c)) {
			if (c == '\n') {
				while (Tlen > 0
				&& isspace(Text[Tlen-1]))
					Tlen--;
				return (TRUE);
			} else if (c == '\t') {
				typeover(' ');
				while (Tcol % tabstop) 
					typeover(' ');
			}
		} else {
			typeover(dot);
		}
	} else if (OptBinary) {
		typeover(dot);
	}
	if (OptBinary)
		if (Tlen - Shift >= COLS)
			return(TRUE);
	return (FALSE);
}

private	void	reset_catcher(_AR0)
{
	was_interrupted = FALSE;
	(void)dedsigs(TRUE);
}

private	int	GetC (_AR0)
{
	register int c = fgetc(InFile);
	if (dedsigs(TRUE))
		was_interrupted = TRUE;
	if (feof(InFile) || ferror(InFile) || was_interrupted)
		c = EOF;
	else if (c < 0)
		c &= 0xff;
	return (c);
}

private	int	reshow(
	_ARX(RING *,	gbl)
	_AR1(unsigned,	inlist)
		)
	_DCL(RING *,	gbl)
	_DCL(unsigned,	inlist)
{
	statLINE(gbl, inlist);
	showLINE(gbl, inlist);
	return TRUE;
}

/*
 * Returns the filesize (implicitly the limit on the number of pages)
 */
private	OFF_T	MaxP(_AR0)
{
	Stat_t	sb;
	OFF_T	length = 0;
	if (fstat(fileno(InFile), &sb) >= 0)
		length = sb.st_size;
	return length;
}

/*
 * Save our current position so we can replay the current page.
 */
private	void	MarkLine(
	_AR1(int *,	infile))
	_DCL(int *,	infile)
{
	static	unsigned allocated_lines = 0;

	*infile += 1;
	if ((*infile)+2 > (int)allocated_lines) {
		allocated_lines = ((*infile) + 100) * 2;
		LineAt = DOALLOC(LineAt,OFFSETS,allocated_lines);
	}
	if (*infile > max_lines) {
		max_lines = *infile;
		LineAt[max_lines].offset = ftell(InFile);
		LineAt[max_lines].blank  = FALSE;
	}
}

/*
 * Reposition at a given top-of-page marker.
 */
private	int	JumpToLine(
	_AR1(int,	infile))
	_DCL(int,	infile)
{
	return fseek(InFile, LineAt[infile].offset, 0);
}

/*
 * Find the top line in a page, given the bottom line-number
 */
private	int	TopOfPage(
	_ARX(int,	infile)
	_AR1(int *,	blank)
		)
	_DCL(int,	infile)
	_DCL(int *,	blank)
{
	int	page	= NumP(1);

	*blank = FALSE;
	if (infile > 0) { /* this was the bottom line; it should be nonzero */
		while (page > 0) {
			if (--infile <= 0)
				break;
			if (*blank && LineAt[infile].blank)
				continue;
			*blank = LineAt[infile].blank;
			page--;
		}
		if (*blank) {
			while (infile > 0 && LineAt[infile-1].blank) {
				infile--;
			}
		} else if (infile > 0
		    &&  infile == LineAt[infile-1].blank) {
			infile = 0;
			*blank = TRUE;
		}
	}
	return infile;
}

/*
 * Returns true if the current top-of-page is the top of the file
 */
private	int	AtTop(
	_AR1(int,	infile))
	_DCL(int,	infile)
{
	int	foo;
	return (TopOfPage(infile, &foo) == 0);
}

/*
 * Reposition by a given number of lines
 */
private	int	JumpBackwards(
	_ARX(int *,	infile)
	_AR1(int,	jump)
		)
	_DCL(int *,	infile)
	_DCL(int,	jump)
{
	int	savejump;
	int	blank;
	int	n;
	int	page = NumP(1);

	savejump = jump = page - jump;
	n = TopOfPage(*infile, &blank);

	/* jump the requested number of lines */
	if (jump > 0) {
		if (n == 0) {
			while ((blank = LineAt[n].blank) != 0) {
				n++;
			}
		}
		while (jump > 0) {
			if (++n >= max_lines) {
				return (-1);
			}
			if (blank && LineAt[n].blank)
				continue;
			blank = LineAt[n].blank;
			jump--;
		}
	} else {
		while (jump < 0) {
			if (--n < 0) {
				*infile = 0;	/* take what we can get */
				(void) JumpToLine(*infile);
				return (-1);
			}
			if (blank && LineAt[n].blank)
				continue;
			blank = LineAt[n].blank;
			jump++;
		}
	}
#if HAVE_WSCRL && HAVE_WSETSCRREG
	if (jump != savejump
	 && (n + NumP(1) < max_lines)
	 && (jump - savejump) < NumP(1)
	 && (savejump - jump) < NumP(1)) {
		int y, x;
		getyx(stdscr, y, x);
		move(LINES-2, 0);
		setscrreg(mark_W+1, LINES-2);
		scrl(savejump - jump);
		setscrreg(0, LINES-1);
		move(y, x);
	}
#endif
	*infile = n;
	return JumpToLine(*infile);
}

/*
 * Read the file's contents for the given page and set up the display in case
 * we decide to use it.
 */
private	int	StartPage(
	_ARX(int *,	infile)		/* in/out: current-line # */
	_ARX(int,	skip)		/* in: # of lines to skip */
	_AR1(int *,	eoff)		/* out: set iff we got eof */
		)
	_DCL(int *,	infile)
	_DCL(int,	skip)
	_DCL(int *,	eoff)
{
	register int	c;
	int	blank	= (*infile == 0)
			|| (*infile+1 == LineAt[*infile].blank);
	int	y	= mark_W + 1;

	showMARK(Shift);
	move(y, 0);
	typeinit();
	HadPattern = FALSE;

	*eoff = FALSE;

	for (;;) {
		if ((c = GetC()) == EOF) {
			*eoff = TRUE;
			if (Tlen == 0)
				break;
		}
		if (*eoff || typeconv(c)) {
			if (Tlen == 0) {
				LineAt[(*infile)].blank = *infile > 0
					? LineAt[(*infile)-1].blank + 1
					: 1;
			} else {
				LineAt[(*infile)].blank = 0;
			}
			MarkLine(infile);
			if ((Tlen == 0) && blank) {
				typeinit();
				continue;
			}
			blank = (Tlen == 0);
			y = typeline(y,skip);
			if (*eoff || (y >= LINES-1))
				break;
		}
	}
	if (!feof(InFile) && ferror(InFile))
		clearerr(InFile);

	return y;	/* result is the current line of display */
}

private	int	FinishPage(
	_ARX(RING *,	gbl)
	_ARX(int,	inlist)
	_ARX(int,	infile)
	_AR1(int,	y)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inlist)
	_DCL(int,	infile)
	_DCL(int,	y)
{
	int	shown	= FALSE;
	int	foo;
	off_t	length	= 0;

	while (y < LINES-1)
		y = typeline(y,FALSE);

	move(LINES-1,0);
	standout();
	PRINTW("---lines %d to %d", TopOfPage(infile, &foo)+1, infile);
	if (inlist >= 0) {
		int	oldy, oldx;
		int	save = gbl->AT_opt;

		getyx(stdscr,oldy,oldx);
		standend();
		gbl->AT_opt = TRUE;
		shown  = reshow(gbl, (unsigned)inlist);
		gbl->AT_opt = save;
		length = gSTAT(inlist).st_size;
		gbl->mrkfile = -1;
		markC(gbl,TRUE);
		standout();
		move(oldy,oldx);

	} else {
		length = MaxP();
	}
	if ((length = MaxP()) != 0)
		PRINTW(": %.1f%%", (ftell(InFile) * 100.) / length);
	PRINTW("---");
	standend();
	PRINTW(" ");
	clrtoeol();

	return shown;
}

private	void	IgnorePage(
	_AR1(int,	infile))
	_DCL(int,	infile)
{
	static	time_t	last;
	time_t	now = time((time_t *)0);

	if (now != last) {
		last = now;
		move(LINES-1,0);
		standout();
		PRINTW("---line %d ...skipping", infile);
		standend();
		PRINTW(" ");
		clrtoeol();
		refresh();
	}
}

/*
 * Test if the current expression is equivalent to the last one.
 */
private	int	SamePattern(
	_AR1(char *,	expr))
	_DCL(char *,	expr)
{
	static	DYN	*last;
	register char	*s = dyn_string(last);

	if (s != 0) {
		if (*s != EOS) {
			if (*expr == EOS
			 || !strcmp(s, expr))
				return TRUE;
		}
	}

	last = dyn_copy(last, expr);
	if (*expr == EOS) {
		beep();
		UsePattern = FALSE;
		return TRUE;
	}
	return FALSE;
}

private	int	FoundPattern(
	_AR1(int *,	infile))
	_DCL(int *,	infile)
{
	int	foo;
	int	top_line = *infile;

	(void)StartPage(infile, TRUE, &foo);
	if (HadPattern || was_interrupted) {
		reset_catcher();
		if (JumpToLine(top_line) >= 0) {
			*infile = top_line;
			(void)StartPage(infile, FALSE, &foo);
			return TRUE;
		}
	}
	IgnorePage(*infile);
	return FALSE;
}

private	void	FindPattern(
	_ARX(RING *,	gbl)
	_ARX(int *,	infile)
	_AR1(int,	key)
		)
	_DCL(RING *,	gbl)
	_DCL(int *,	infile)
	_DCL(int,	key)
{
	int	foo;
	register char	*s;
	auto	int	next,
			save	= *infile;
	static	DYN	*text;
	static	HIST	*History;
	static	int	order;		/* saves last legal search order */
	static	int	ok_expr;

	if (key == '/' || key == '?') {

		dyn_init(&text, BUFSIZ);
		if (!(s = dlog_string(gbl, "Target: ", LINES-1, &text, (DYN **)0,
				&History, EOS, BUFSIZ))
		 || !*s) {
			UsePattern = FALSE;
			return;
		}
		if (key == '/')	order = 1;
		if (key == '?') order = -1;
		next = order;

	} else if (order && (s = dyn_string(text))) {
		next = (key == 'n') ? order : -order;
	} else {
		waitmsg("No previous regular expression");
		return;
	}

	if (ok_expr)
		OLD_REGEX(ToFind);
	if ((ok_expr = NEW_REGEX(ToFind,s)) != 0) {

		UsePattern = TRUE;
		if (SamePattern(s)) {
			if (ispunct(key))
				return;
		} else {
			if (next < 0)
				;
			else if (JumpBackwards(infile, NumP(1)) < 0)
				return;
		}

		if (next < 0) {
			while (JumpBackwards(infile, NumP(2)) >= 0) {
				if (FoundPattern(infile))
					return;
			}
		} else {
			while (!feof(InFile)) {
				if (FoundPattern(infile))
					return;
			}
		}

		*infile = TopOfPage(save, &foo);
		if (JumpToLine(*infile) >= 0)
			(void)StartPage(infile, FALSE, &foo);
		waitmsg("Expression not found");

	} else {
		order = 0;
		UsePattern = FALSE;
		BAD_REGEX(ToFind);
		showC(gbl);
	}
}

/*
 * Recompute the left-margin of the workspace.
 */
private	void	LeftOrRight(
	_AR1(int,	count))
	_DCL(int,	count)
{
	if (OptBinary || ((count < 0) && !Shift)) {
		beep();
	} else {
		Shift += count;
		if (Shift < 0)	Shift = 0;
	}
}

/******************************************************************************/
public	void	dedtype(
	_ARX(RING *,	gbl)
	_ARX(char *,	name)
	_ARX(int,	inlist)
	_ARX(int,	binary)
	_ARX(int,	stripped)
	_AR1(int,	isdir)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
	_DCL(int,	inlist)
	_DCL(int,	binary)
	_DCL(int,	stripped)
	_DCL(int,	isdir)
{
	int	had_eof;
	register int	c;
	int	count,			/* ...and repeat-count */
		y,			/* current line-in-screen */
		shift	= COLS/4,	/* amount of left/right shift */
		done	= FALSE,
		shown	= FALSE,
		infile	= -1;		/* # of lines processed */
	OFF_T	skip	= 0;

	tabstop = 8;
	Shift	= 0;
	UsePattern  = FALSE;
	OptBinary   = binary;
	OptStripped = stripped;

	if (isdir && !OptBinary) {
		DIR	*dp;
		DirentT *de;
		char	bfr[MAXPATHLEN];
#ifdef	apollo
# define INO_FMT "%08lx"
#else
# define INO_FMT "%5lu"
#endif
		if ((InFile = tmpfile()) == 0) {
			warn(gbl, "tmp-file");
			return;
		}
		if ((dp = opendir(name)) != 0) {
			while ((de = readdir(dp)) != NULL) {
				(void)ded2string(gbl, bfr,
					(int)de->d_namlen,
					de->d_name,
					FALSE);
				FPRINTF(InFile, INO_FMT, (long) de->d_ino);
				FPRINTF(InFile, " %s\n", bfr);
			}
			(void)closedir(dp);
			rewind(InFile);
		} else {
			warn(gbl, "opendir");
			FCLOSE(InFile);
			return;
		}
	} else
		InFile = fopen(name, "r");

	in_dedtype = TRUE;	/* disable clearing of workspace via A/a cmd */

	if (InFile) {
		auto	int	jump	= 0;

		dlog_comment("type \"%s\" (%s %s)\n",
			name,
			OptBinary ? "binary" : "text",
			isdir  ? "directory" : "file");
		to_work(gbl,FALSE);

		dyn_init(&my_text, BUFSIZ);
		dyn_init(&my_over, BUFSIZ);

		max_lines = -1;
		MarkLine(&infile);

		while (!done) {

			if (jump) {
#if HAVE_WSCRL && HAVE_WSETSCRREG
				/*
				 * If we're doing single-line scrolling past
				 * the point we've read in the file, try to
				 * cache pointers so that the scrolling logic
				 * will go more smoothly.
				 */
				if (jump > 0
				 && jump < NumP(1)
				 && infile + NumP(1) >= max_lines) {
					int line = infile;
					(void)StartPage(&line, TRUE, &had_eof);
				}
#endif
				(void)JumpBackwards(&infile, jump);
				jump = 0;
			}

			markC(gbl, TRUE);
			y = StartPage(&infile, skip, &had_eof);
			if (skip && !was_interrupted) {
				if (feof(InFile)) {
					skip = 0;
					jump = NumP(1);
				} else {
					IgnorePage(infile);
					skip--;
				}
				continue;
			}
			if (had_eof) {
				int	blank;
				infile = TopOfPage(infile, &blank);
				(void)JumpToLine(infile);
				y = StartPage(&infile, 0, &had_eof);
			}
			shown |= FinishPage(gbl, inlist, infile, y);
			jump  = NumP(1);

			reset_catcher();
			switch (c = dlog_char(gbl, &count,1)) {
			case CTL('K'):
				deddump(gbl);
				break;
			case 'w':
				retouch(gbl,0);
				break;
			case '\t':
				if (OptBinary) {
					beep();
				} else {
					tabstop = (count <= 1)
						? (tabstop == 8 ? 4 : 8)
						: count;
				}
				break;

			case 'q':
				done = TRUE;
				break;

			case KEY_HOME:
			case '^':
				jump = infile;
				break;

			case KEY_END:
			case '$':
				infile = max_lines;
				skip = MaxP();
				break;

			case KEY_PPAGE:
			case '\b':
			case 'b':
				if (AtTop(infile)) {
					beep();
				} else {
					jump += NumP(count);
				}
				break;
			case KEY_NPAGE:
			case '\n':
			case ' ':
			case 'f':
				jump = 0;
				skip = count - 1;
				break;

			case '<':
			case CTL('L'):
				LeftOrRight(-shift * count);
				break;
			case '>':
			case CTL('R'):
				LeftOrRight( shift * count);
				break;

			case KEY_LEFT:
			case 'h':
				LeftOrRight(-count);
				break;
			case KEY_DOWN:
			case 'j':
				jump = NumP(1) - count;
				if ((infile - jump) > max_lines) {
					skip = (-jump + NumP(1) - 1) / NumP(1);
					jump = 0;
				}
				break;
			case KEY_UP:
			case 'k':
				if (AtTop(infile)) {
					beep();
				} else {
					jump += count;
				}
				break;
			case KEY_RIGHT:
			case 'l':
				LeftOrRight( count);
				break;

				/* move work-area marker */
			case 'A':
				count = -count;
				jump -= count;
				/*FALLTHRU*/
			case 'a':
				markset(gbl, mark_W + count);
				break;

			case '/':
			case '?':
			case 'n':
			case 'N':
				FindPattern(gbl, &infile, c);
				break;
			default:
				beep();
			}
		}
		FCLOSE(InFile);
		if (shown)
			(void)reshow(gbl, (unsigned)inlist);
		showMARK(gbl->Xbase);

		showC(gbl);
	} else
		warn(gbl, name);
	in_dedtype = FALSE;
}
