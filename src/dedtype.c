#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: dedtype.c,v 12.15 1994/07/04 14:16:25 tom Exp $";
#endif

/*
 * Title:	dedtype.c (type files for ded)
 * Author:	T.E.Dickey
 * Created:	16 Nov 1987
 * Modified:
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
 *			     Dynamically allocate 'PageAt[].'
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

#define	def_doalloc	OFF_T_alloc
	/*ARGSUSED*/
	def_DOALLOC(OFF_T)

#define	END_COL	(my_text->max_length-1)
#define	Text	dyn_string(my_text)
#define	Over	dyn_string(my_over)

/******************************************************************************/
static	FILE	*InFile;
static	OFF_T	*PageAt;
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
		was_interrupted;

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
		if (Tlen > COLS-1)
			Tlen = COLS-1;

		if (found) {
			standout();
			if (Tlen > 0)
				PRINTW("%.*s", Tlen, Text + Shift);
			else
				PRINTW(" ");
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
	if (Tcol+1 >= END_COL) {
		size_t need = (Tcol * 5)/4;
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
			} else if (Tcol <= END_COL)
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
		if (Tlen - Shift >= COLS-1)
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
	_AR1(int,	inlist)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inlist)
{
	statLINE(gbl, inlist);
	showLINE(gbl, inlist);
	return TRUE;
}

/*
 * Save our current position so we can replay the current page.
 */
private	void	MarkPage(
	_AR1(int,	page))
	_DCL(int,	page)
{
	static	unsigned maxpage = 0;

	if (page+2 > maxpage) {
		maxpage = page + 100;
		PageAt = DOALLOC(PageAt,OFF_T,maxpage);
	}
	PageAt[page] = ftell(InFile);
}

/*
 * Reposition at a given top-of-page marker.
 */
private	int	JumpBackwards(
	_ARX(int *,	page)
	_AR1(int,	jump)
		)
	_DCL(int *,	page)
	_DCL(int,	jump)
{
	*page -= jump;
	if (*page < 0)
		return -1;
	return fseek(InFile, PageAt[*page], 0);
}

/*
 * Read the file's contents for the given page and set up the display in case
 * we decide to use it.
 */
private	int	StartPage(
	_ARX(int *,	page)		/* in/out: current-page # */
	_ARX(int,	skip)		/* in: # of pages to skip */
	_AR1(int *,	eoff)		/* out: set iff we got eof */
		)
	_DCL(int *,	page)
	_DCL(int,	skip)
	_DCL(int *,	eoff)
{
	register int	c;
	int	blank	= TRUE,		/* flag to suppress blank lines */
		y	= mark_W + 1;

	showMARK(Shift);
	move(y, 0);
	typeinit();
	HadPattern = FALSE;

	MarkPage(*page);
	*page += 1;
	*eoff = FALSE;

	for (;;) {
		if ((c = GetC()) == EOF) {
			*eoff = TRUE;
			break;
		}
		if (typeconv(c)) {
			if ((Tlen == 0) && blank) {
				typeinit();
				continue;
			}
			blank = (Tlen == 0);
			y = typeline(y,skip);
			if (y >= LINES-1)
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
	_ARX(int,	page)
	_AR1(int,	y)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inlist)
	_DCL(int,	page)
	_DCL(int,	y)
{
	int	shown	= FALSE;
	off_t	length	= 0;
	Stat_t	sb;

	while (y < LINES-1)
		y = typeline(y,FALSE);

	move(LINES-1,0);
	standout();
	PRINTW("---page %d", page);
	if (inlist >= 0) {
		int	oldy, oldx;
		int	save = gbl->AT_opt;

		getyx(stdscr,oldy,oldx);
		standend();
		gbl->AT_opt = TRUE;
		shown  = reshow(gbl, inlist);
		gbl->AT_opt = save;
		length = gSTAT(inlist).st_size;
		gbl->mrkfile = -1;
		markC(gbl,TRUE);
		standout();
		move(oldy,oldx);

	} else if (fstat(fileno(InFile), &sb) >= 0) {
		length = sb.st_size;
	}
	if (length != 0)
		PRINTW(": %.1f%%",
			(ftell(InFile) * 100.)/ length);
	PRINTW("---");
	standend();
	PRINTW(" ");
	clrtoeol();
	refresh();

	return shown;
}

private	void	IgnorePage(
	_AR1(int,	page))
	_DCL(int,	page)
{
	move(LINES-1,0);
	standout();
	PRINTW("---page %d ...skipping", page);
	standend();
	PRINTW(" ");
	clrtoeol();
	refresh();
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
	_AR1(int *,	page))
	_DCL(int *,	page)
{
	int	foo;
	(void)StartPage(page, TRUE, &foo);
	if (HadPattern || was_interrupted) {
		reset_catcher();
		if (JumpBackwards(page, 1) >= 0) {
			(void)StartPage(page, FALSE, &foo);
			return TRUE;
		}
	}
	IgnorePage(*page);
	return FALSE;
}

private	void	FindPattern(
	_ARX(RING *,	gbl)
	_ARX(int *,	page)
	_AR1(int,	key)
		)
	_DCL(RING *,	gbl)
	_DCL(int *,	page)
	_DCL(int,	key)
{
	int	foo;
	register char	*s;
	auto	int	next,
			save	= *page;
	static	DYN	*text;
	static	HIST	*History;
	static	int	order;		/* saves last legal search order */
	static	int	ok_expr;

	if (key == '/' || key == '?') {

		dyn_init(&text, BUFSIZ);
		if (!(s = dlog_string(gbl, "Target: ", &text, (DYN **)0,
				&History, EOS, 0)))
			return;
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
			else if (JumpBackwards(page, 1) < 0)
				return;
		}

		if (next < 0) {
			while (JumpBackwards(page, 2) >= 0) {
				if (FoundPattern(page))
					return;
			}
		} else {
			while (!feof(InFile)) {
				if (FoundPattern(page))
					return;
			}
		}

		*page = save;
		if (JumpBackwards(page, 1) >= 0)
			(void)StartPage(page, FALSE, &foo);
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
	if (OptBinary || ((count < 0) && !Shift))
		beep();
	else {
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
		skip	= 0,
		page	= 0;		/* # of screens done */

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
# define INO_FMT "%08lx %s\n"
#else
# define INO_FMT "%5lu %s\n"
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
				FPRINTF(InFile, INO_FMT, de->d_ino, bfr);
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

	if (InFile) {
		auto	int	jump	= 0;

		dlog_comment("type \"%s\" (%s %s)\n",
			name,
			OptBinary ? "binary" : "text",
			isdir  ? "directory" : "file");
		to_work(gbl,FALSE);

		dyn_init(&my_text, BUFSIZ);
		dyn_init(&my_over, BUFSIZ);

		while (!done) {

			if (jump) {
				if ((done = JumpBackwards(&page, jump)) != 0)
					break;
				jump = 0;
			}

			markC(gbl, TRUE);
			y = StartPage(&page, skip, &had_eof);
			if (skip && !was_interrupted) {
				if (feof(InFile)) {
					skip = 0;
					jump = 1;
				} else {
					IgnorePage(page);
					skip--;
				}
				continue;
			}
			shown |= FinishPage(gbl, inlist, page, y);
			jump  = 1;

			reset_catcher();
			switch (c = dlog_char(gbl, &count,1)) {
			case CTL('K'):
				deddump(gbl);
				break;
			case 'w':
				retouch(gbl,0);
				break;
			case '\t':
				if (OptBinary)
					beep();
				else
					tabstop = (count <= 1)
						? (tabstop == 8 ? 4 : 8)
						: count;
				break;

			case 'q':
				done = TRUE;
				break;

			case ARO_UP:
			case '\b':
			case 'b':
				if (page <= 1 && count > 0)
					beep();
				else if ((jump += count) > page)
					jump = page;
				break;
			case ARO_DOWN:
			case '\n':
			case ' ':
			case 'f':
				jump = 0;
				skip = count - 1;
				if (had_eof)
					done = JumpBackwards(&page, 1);
				break;

			case '<':
				LeftOrRight(-count);
				break;
			case '>':
				LeftOrRight( count);
				break;
			case CTL('L'):
			case ARO_LEFT:
				LeftOrRight(-shift * count);
				break;
			case CTL('R'):
			case ARO_RIGHT:
				LeftOrRight( shift * count);
				break;

				/* move work-area marker */
			case 'A':	count = -count;
			case 'a':
				markset(gbl, mark_W + count);
				break;

			case '/':
			case '?':
			case 'n':
			case 'N':
				FindPattern(gbl, &page, c);
				break;
			default:
				beep();
			}
		}
		FCLOSE(InFile);
		if (shown)
			(void)reshow(gbl, inlist);
		showMARK(gbl->Xbase);

		if (done < 0)
			warn(gbl, "fseek");
		else
			showC(gbl);
	} else
		warn(gbl, name);
}
