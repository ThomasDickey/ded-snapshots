#ifndef	lint
static	char	sccs_id[] = "@(#)dedtype.c	1.6 88/05/02 14:40:25";
#endif	lint

/*
 * Title:	dedtype.c (type files for ded)
 * Author:	T.E.Dickey
 * Created:	16 Nov 1987
 * Modified:
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
#include	"ded.h"

static	char	text[BUFSIZ],		/* converted display-text */
		over[BUFSIZ];		/* overstrike/underline flags */
static	int	Shift,			/* left/right shift-column */
		Tlen,			/* strlen(text) */
		Tcol;			/* current column in text[] */

static
typeinit()
{
	text[Tlen = Tcol = 0] = EOS;
}

static
typeline(y)
{
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
			printw("%.*s", (j - now), &text[now]);
			if (over[now])	standend();
			Tlen -= (j - now);
			now = j;
		}
	}
	clrtoeol();
	typeinit();
	return (++y);
}

static
typeover(c)
{
	if (over[Tcol] = text[Tcol]) {
		if (ispunct(text[Tcol]))
			text[Tcol] = c;
	} else
		text[Tcol] = c;
	Tcol++;
	if (Tcol > Tlen)	text[Tlen = Tcol] = EOS;
}

static
typeconv(c,binary)
{
	if (Tcol < sizeof(text)-1) {
		if (binary) {	/* highlight chars with parity */
			if (!isascii(c)) {
				c = toascii(c);
				text[Tcol] = '_';
			}
		}
		if (isascii(c)) {
			if (binary && !isprint(c)) {
				typeover('.');
			} else if (c == '\b') {
				if (Tcol > 0) Tcol--;
			} else if (c == '\r') {
				Tcol = 0;
			} else if (isprint(c)) {
				typeover(c);
			} else if (isspace(c)) {
				if (c == '\n') {
					return (TRUE);
				} else if (c == '\t') {
					typeover(' ');
					while (Tcol & 7) 
						typeover(' ');
				}
			} else
				typeover('.');
		} else
			typeover('.');
	}
	if (binary)
		if (Tlen - Shift >= COLS-1)
			return(TRUE);
	return (FALSE);
}

dedtype(name,binary)
char	*name;
{
struct	stat	sb;
FILE	*fp	= fopen(name, "r");
int	c,			/* current character */
	count,			/* ...and repeat-count */
	y,			/* current line-in-screen */
	blank,			/* flag to suppress blank lines */
	shift	= COLS/3,	/* amount of left/right shift */
	done	= FALSE,
	skip	= 0,
	page	= 0;		/* counter to show how many screens done */

	Shift	= 0;

	if (fp) {
		to_work();
		while (!done) {
		static	off_t	*infile;
		static	int	maxpage = 0;
		int	replay	= 0;

			y	= mark_W + 1;
			blank	= TRUE;

			markC(TRUE);
			move(y, 0);
			clrtobot();
			typeinit();

			if (page > maxpage-2) {
				maxpage = page + 100;
				infile = (off_t *)doalloc((char *)infile,
							maxpage * sizeof(off_t));
			}
			infile[page++] = ftell(fp);
			while (c = fgetc(fp), !feof(fp)) {
				if (typeconv(c,binary)) {
					if ((Tlen == 0) && blank)
						continue;
					blank = (Tlen == 0);
					y = typeline(y);
					if (y >= LINES-1)
						break;
				}
			}

			while (y < LINES-1)
				y = typeline(y);

			move(LINES-1,0);
			standout();
			printw("---page");
			if ((stat(name, &sb) >= 0) && sb.st_size > 0) {
				printw(" %d: %ld%%",
					page,
					(ftell(fp) * 100) / sb.st_size);
			}
			printw("---");
			standend();
			printw(" ");
			clrtoeol();

			if (feof(fp))
				skip = 0;
			if (!skip)
				refresh();
			else {
				skip--;
				continue;
			}

			switch (cmdch(&count)) {
			case 'w':
				retouch(0);
				replay = 1;
				break;
			case 'q':
				done = TRUE;
				break;
			case ARO_UP:
			case '\b':
			case 'b':
				replay = count + 1;
				break;
			case ARO_DOWN:
			case '\n':
			case '\r':
			case ' ':
			case 'f':
				skip = count - 1;
				if (feof(fp))
					fseek(fp, infile[--page], 0);
				break;
			case ARO_LEFT:
				if (binary)
					beep();
				else {
					Shift -= (shift * count);
					if (Shift < 0)	Shift = 0;
				}
				replay = 1;
				break;
			case ARO_RIGHT:
				if (binary)
					beep();
				else
					Shift += (shift * count);
				replay = 1;
				break;
				/* move work-area marker */
			case 'A':	count = -count;
			case 'a':
				markset(mark_W + count);
				replay = 1;
				break;
			default:
				replay = 1;
				beep();
			}

			if (replay) {
				page -= replay;
				if (page < 0) page = 0;
				fseek(fp, infile[page], 0L);
			}
		}
		(void)fclose(fp);
		move(LINES+1,0);
		clrtobot();
		showC();
	} else
		warn(name);
}
