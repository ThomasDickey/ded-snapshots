#ifndef	lint
static	char	Id[] = "$Id: dedtype.c,v 7.0 1990/03/05 10:42:09 ste_cm Rel $";
#endif	lint

/*
 * Title:	dedtype.c (type files for ded)
 * Author:	T.E.Dickey
 * Created:	16 Nov 1987
 * $Log: dedtype.c,v $
 * Revision 7.0  1990/03/05 10:42:09  ste_cm
 * BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *
 *		Revision 6.0  90/03/05  10:42:09  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.2  90/03/05  10:42:09  dickey
 *		port to sun3 (os3.4)
 *		
 *		Revision 5.1  90/03/02  08:19:22  dickey
 *		for the special case of a directory-file, create a temporary
 *		file with the inode-values and names of the files.  Display
 *		this instead of the raw contents of the directory.
 *		
 *		Revision 5.0  89/10/13  13:43:28  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.1  89/10/13  13:43:28  dickey
 *		trim lines containing blanks so we don't display extra gaps
 *		
 *		Revision 4.0  89/03/14  10:32:36  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/03/14  10:32:36  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  89/03/14  10:32:36  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.17  89/03/14  10:32:36  dickey
 *		sccs2rcs keywords
 *		
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
typeline(y,skip)
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
typeover(c)
register c;
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
register c;
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
					while (Tlen > 0
					&& isspace(text[Tlen-1]))
						Tlen--;
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

static
GetC(fp)
FILE	*fp;
{
	register int c = fgetc(fp);
	if (feof(fp) || ferror(fp) || dedsigs(TRUE))
		c = EOF;
	else if (c < 0)
		c &= 0xff;
	return (c);
}

dedtype(name,binary,isdir)
char	*name;
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

	Shift	= 0;

	if (isdir && !binary) {
		DIR		*dp;
		struct	direct	*de;
		char		bfr[BUFSIZ];
#ifdef	apollo
		char		*fmt = "%08x %s\n";
#else
		char		*fmt = "%5d %s\n";
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
				fprintf(fp, fmt, de->d_ino, bfr);
			}
			closedir(dp);
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
		auto	int	replay	= 0;

		dlog_comment("type \"%s\" (%s %s)\n",
			name,
			binary ? "binary" : "text",
			isdir  ? "directory" : "file");
		to_work();
		while (!done) {

			if (replay) {
				page -= replay;
				if (page < 0) page = 0;
				if (fseek(fp, infile[page], 0) < 0) {
					done = -1;
					break;
				}
				replay = 0;
			}

			y	= mark_W + 1;
			blank	= TRUE;

			markC(TRUE);
			move(y, 0);
			clrtobot();
			typeinit();

			if (page+2 > maxpage) {
				maxpage = page + 100;
				infile = DOALLOC(infile,OFF_T,maxpage);
			}
			infile[page++] = ftell(fp);
			while ((c = GetC(fp)) != EOF) {
				if (typeconv(c,binary)) {
					if ((Tlen == 0) && blank)
						continue;
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
					replay = 1;
				} else
					skip--;
				continue;
			}

			switch (dlog_char(&count,1)) {
			case CTL(K):
				deddump();
				replay = 1;
				break;
			case 'w':
				retouch(0);
				replay = 1;
				break;
			case 'q':
				done = 1;
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
		}
		FCLOSE(fp);
		if (isdir && !binary)
			(void)unlink(tmp_name);
		if (done < 0)
			warn("fseek");
		else {
			move(LINES+1,0);
			clrtobot();
			showC();
		}
	} else
		warn(name);
}
