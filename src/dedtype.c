#ifndef	lint
static	char	Id[] = "$Id: dedtype.c,v 10.0 1991/10/18 10:14:53 ste_cm Rel $";
#endif

/*
 * Title:	dedtype.c (type files for ded)
 * Author:	T.E.Dickey
 * Created:	16 Nov 1987
 * Modified:
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
 *			     Dynamically allocate 'infile[].'
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

#define	END_COL	(sizeof(text)-1)
static	char	text[BUFSIZ],		/* converted display-text */
		over[BUFSIZ];		/* overstrike/underline flags */
static	int	Shift,			/* left/right shift-column */
		Tlen,			/* strlen(text) */
		Tcol,			/* current column in text[] */
		tabstop;

static
typeinit(_AR0)
{
	text[Tlen = Tcol = 0] = EOS;
}

static
typeline(
_ARX(int,	y)
_AR1(int,	skip)
	)
_DCL(int,	y)
_DCL(int,	skip)
{
	if (!skip) {
		move(y,0);
		if (Tlen > Shift) {
		int	now	= Shift,
			j;

			Tlen	-= Shift;
			if (Tlen > COLS-1)
				Tlen = COLS-1;
			while (Tlen > 0) {
				for (j = now; j < now+Tlen; j++) {
					if (over[j] != over[now])	break;
				}
				if (over[now])	standout();
				PRINTW("%.*s", (j - now), &text[now]);
				if (over[now])	standend();
				Tlen -= (j - now);
				now = j;
			}
		}
		clrtoeol();
	}
	typeinit();
	return (++y);
}

static
typeover _ONE(register int,c)
{
	if (Tcol <= END_COL) {
		if (over[Tcol] = text[Tcol]) {
			if (ispunct(text[Tcol]))
				text[Tcol] = c;
		} else
			text[Tcol] = c;
	}
	Tcol++;
	if (Tcol > Tlen) {
		Tlen = Tcol;
		if ((Tlen = Tcol) > END_COL)
			Tlen = END_COL;
		text[Tlen] = EOS;
	}
}

static
typeconv(
_ARX(int,	c)
_ARX(int,	binary)
_AR1(int,	stripped)
	)
_DCL(int,	c)
_DCL(int,	binary)
_DCL(int,	stripped)
{
	char	dot	= stripped ? ' ' : '.';

	if (binary) {	/* highlight chars with parity */
		if (!isascii(c)) {
			c = toascii(c);
			if (stripped) {
				if (!isprint(c))
					c = ' ';
			} else if (Tcol <= END_COL)
				text[Tcol] = '_';
		}
	}
	if (isascii(c)) {
		if (binary && !isprint(c)) {
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
				&& isspace(text[Tlen-1]))
					Tlen--;
				return (TRUE);
			} else if (c == '\t') {
				typeover(' ');
				while (Tcol % tabstop) 
					typeover(' ');
			}
		} else
			typeover(dot);
	} else
		typeover(dot);
	if (binary)
		if (Tlen - Shift >= COLS-1)
			return(TRUE);
	return (FALSE);
}

static
GetC _ONE(FILE *,fp)
{
	register int c = fgetc(fp);
	if (feof(fp) || ferror(fp) || dedsigs(TRUE))
		c = EOF;
	else if (c < 0)
		c &= 0xff;
	return (c);
}

dedtype(
_ARX(char *,	name)
_ARX(int,	binary)
_ARX(int,	stripped)
_AR1(int,	isdir)
	)
_DCL(char *,	name)
_DCL(int,	binary)
_DCL(int,	stripped)
_DCL(int,	isdir)
{
static	char	tmp_name[L_tmpnam];
struct	stat	sb;
FILE	*fp;
int	c,			/* current character */
	count,			/* ...and repeat-count */
	y,			/* current line-in-screen */
	blank,			/* flag to suppress blank lines */
	shift	= COLS/3,	/* amount of left/right shift */
	done	= FALSE,
	skip	= 0,
	page	= 0;		/* counter to show how many screens done */

	tabstop = 8;
	Shift	= 0;

	if (isdir && !binary) {
		DIR		*dp;
		struct	direct	*de;
		char		bfr[BUFSIZ];
#ifdef	apollo
		char		*fmt = "%08x %s\n";
#else
		char		*fmt = "%5u %s\n";
#endif
		FORMAT(tmp_name, "%s/ded%d", P_tmpdir, getpid());
		(void)unlink(tmp_name);
		if ((fp = fopen(tmp_name,"w+")) == 0) {
			warn("tmp-file");
			return;
		}
		if ((dp = opendir(name)) != 0) {
			while (de = readdir(dp)) {
				(void)ded2string(bfr,
					(int)de->d_namlen,
					de->d_name,
					FALSE);
				FPRINTF(fp, fmt, de->d_ino, bfr);
			}
			(void)closedir(dp);
			rewind(fp);
		} else {
			warn("opendir");
			FCLOSE(fp);
			(void)unlink(tmp_name);
			return;
		}
	} else
		fp = fopen(name, "r");

	if (fp) {
		static	OFF_T	*infile;
		static	unsigned maxpage = 0;
		auto	int	again	= 0;

		dlog_comment("type \"%s\" (%s %s)\n",
			name,
			binary ? "binary" : "text",
			isdir  ? "directory" : "file");
		to_work(FALSE);
		while (!done) {

			if (again) {
				page -= again;
				if (page < 0) page = 0;
				if (fseek(fp, infile[page], 0) < 0) {
					done = -1;
					break;
				}
				again = 0;
			}

			y	= mark_W + 1;
			blank	= TRUE;

			markC(TRUE);
			showMARK(Shift);
			move(y, 0);
			typeinit();

			if (page+2 > maxpage) {
				maxpage = page + 100;
				infile = DOALLOC(infile,OFF_T,maxpage);
			}
			infile[page++] = ftell(fp);
			while ((c = GetC(fp)) != EOF) {
				if (typeconv(c,binary,stripped)) {
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
			if (!feof(fp) && ferror(fp))	clearerr(fp);

			if (!skip) {
				while (y < LINES-1)
					y = typeline(y,FALSE);

				move(LINES-1,0);
				standout();
				PRINTW("---page");
				if ((fstat(fileno(fp), &sb) >= 0)
				&&  sb.st_size > 0) {
					PRINTW(" %d: %.1f%%",
						page,
						(ftell(fp) * 100.)/ sb.st_size);
				}
				PRINTW("---");
				standend();
				PRINTW(" ");
				clrtoeol();
				refresh();
			} else {
				if (feof(fp)) {
					skip = 0;
					again = 1;
				} else
					skip--;
				continue;
			}

			switch (dlog_char(&count,1)) {
			case CTL('K'):
				deddump();
				again = 1;
				break;
			case 'w':
				retouch(0);
				again = 1;
				break;
			case '\t':
				if (binary)
					beep();
				else
					tabstop = (count <= 1)
						? (tabstop == 8 ? 4 : 8)
						: count;
				again  = 1;
				break;
			case 'q':
				done = 1;
				break;
			case ARO_UP:
			case '\b':
			case 'b':
				again = count + 1;
				break;
			case ARO_DOWN:
			case '\n':
			case '\r':
			case ' ':
			case 'f':
				skip = count - 1;
				if (feof(fp))
					if (fseek(fp, infile[--page], 0) < 0)
						done = -1;
				break;
			case ARO_LEFT:
				if (binary)
					beep();
				else {
					Shift -= (shift * count);
					if (Shift < 0)	Shift = 0;
				}
				again = 1;
				break;
			case ARO_RIGHT:
				if (binary)
					beep();
				else
					Shift += (shift * count);
				again = 1;
				break;
				/* move work-area marker */
			case 'A':	count = -count;
			case 'a':
				markset(mark_W + count,FALSE);
				again = 1;
				break;
			default:
				again = 1;
				beep();
			}
		}
		FCLOSE(fp);
		showMARK(Xbase);
		if (isdir && !binary)
			(void)unlink(tmp_name);
		if (done < 0)
			warn("fseek");
		else
			showC();
	} else
		warn(name);
}
