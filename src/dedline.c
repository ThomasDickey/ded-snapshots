#ifndef	lint
static	char	Id[] = "$Id: dedline.c,v 10.2 1992/04/01 14:56:11 dickey Exp $";
#endif

/*
 * Title:	dedline.c (directory-editor inline editing)
 * Author:	T.E.Dickey
 * Created:	01 Aug 1988 (from 'ded.c')
 * Modified:
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		12 Nov 1991, killchar in 'edittext()' was not properly erasing
 *			     the buffer.
 *		16 Oct 1991, mods to support replay of 'c'-commands.
 *			     Allow newline to end 'p'-command, CTL/F and CTL/B
 *			     recognized as in edit-text.
 *		15 Oct 1991, converted to ANSI.
 *		11 Jul 1991, modified interface to 'showFILES()' so that
 *			     workspace is not cleared when doing the inline
 *			     operations.
 *		28 Jun 1991, corrected code which tests for user's id (look at
 *			     effective uid, not real-uid).
 *		15 May 1991, mods to accommodate apollo sr10.3
 *		18 Apr 1991, fixed end-of-buffer code for 'edittext()' (caused
 *			     spurious data overwrites).
 *		06 Mar 1990, lint
 *		26 Oct 1989, altered 'editmode()' to reduce number of register
 *			     variables used (bypasses bug on sun3)
 *		12 Oct 1989, altered format so that uid,gid columns are not
 *			     necessarily obscured (G_opt == 2).  Also, prevent
 *			     chmod if object has extended acls -- and user is
 *			     not owner (prevents trouble!)
 *		06 Oct 1989, modified treatment of 'cmdcol[]' (cf: showFILES)
 *		11 Aug 1989, modified "<" command so that we show all
 *			     intermediate substitutions (i.e., "%F", "%B" and
 *			     "#") which would be applied to a tagged-group --
 *			     before we begin editing.
 *		12 Jun 1989, corrected '<' command-substitution, which lost
 *			     chars after the '#' substitution.
 *		14 Mar 1989, added '<' to do %F, %B, # substitution (was in '>')
 *			     Interface to 'dlog'.
 *		23 Jan 1989, in '>', '=', do nothing if no text changed.
 *			     For '>', provide '%B' and '%F' expansion.
 *		02 Sep 1988, added 'editlink()'
 *		12 Aug 1988, apollo sys5 permits symbolic links.
 *		03 Aug 1988, use 'dedsigs()' to permit interrupt of group-ops.
 *			     For edit_uid, edit_gid, ensure that we map-thru
 *			     with symbolic links.
 *
 * Function:	Procedures which perform in-line editing of particular fields
 *		of the file-list.
 */

#include	"ded.h"

#define	CHMOD(n)	(xSTAT(n).st_mode & 07777)
#define	OWNER(n)	((geteuid() == 0) || (xSTAT(x).st_uid == geteuid()))

#define	TO_FIRST	CTL('b')
#define	TO_LAST		CTL('f')

static	int	re_edit;		/* flag for 'edittext()' */
static	char	lastedit[BUFSIZ];

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Store/retrieve field-editing commands.  The first character of the buffer
 * 'lastedit[]' is reserved to tell us what the command was.
 */
replay _ONE(int,c)
{
	static	int	in_edit;

	if (c != EOS) {
		switch (c) {
		case -2:	/* remove all data (quit/abend) */
			in_edit = 1;
		case -1:	/* remove prior-data (e.g., for retry/append) */
			if (in_edit > 0) {
				c = lastedit[--in_edit];
				lastedit[in_edit] = EOS;
			}
			break;
		default:
			lastedit[0] = c;
			in_edit = 1;
		}
	} else {
		if (re_edit && lastedit[in_edit] != EOS) {
			c = lastedit[in_edit++];
		} else {
			c = dlog_char((int *)0,0);
			if (c == '\r') c = '\n';
			lastedit[in_edit++] = c;
			lastedit[in_edit]   = EOS;
		}
	}

	return (c & 0xff);
}

/*
 * Construct an editable-string for 'editname()' and 'editlink()'.
 * All nonprinting characters will be shown as '?'.
 */
static
char *
name2bfr(
_ARX(char *,	dst)
_AR1(char *,	src)
	)
_DCL(char *,	dst)
_DCL(char *,	src)
{
	(void)name2s(dst, BUFSIZ, src, FALSE);
	return (dst);
}

/*
 * Save AT_opt mode when we are editing inline, and show mapped-thru stat for
 * symbolic links.
 */
#ifdef	S_IFLNK
static
at_save(_AR0)
{
	if (!FOO->AT_opt) {	/* chmod applies only to target of symbolic link */
		return (at_last(TRUE));
	}
	return (FALSE);
}

/*
 * If any tagged files are symbolic links, set the AT_opt to the specified flag
 * value and re-stat them.  Return a count of the number of links.
 */
static
at_last _ONE(int,flag)
{
	register int x;
	register int changed = 0;

	for (x = 0; x < FOO->numfiles; x++)
		if (GROUPED(x)
		&& xLTXT(x)) {
			FOO->AT_opt = flag;
			statLINE(FOO, x);
			showLINE(FOO, x);
			changed++;
		}
	return (changed);
}

/*
 * Assign a new target for a symbolic link.
 */
static
relink(
_ARX(int,	x)
_AR1(char *,	name)
	)
_DCL(int,	x)
_DCL(char *,	name)
{
	dlog_comment("relink \"%s\" (link=%s)\n", name, xNAME(x));
	if (unlink(xNAME(x)) >= 0) {
		if (symlink(name, xNAME(x)) >= 0)
			return (TRUE);
		(void)symlink(xLTXT(x), xNAME(x));	/* try to restore */
	}
	waitmsg(xNAME(x));
	return (FALSE);
}

static	int	cmd_link;	/* true if we use short-form */

/*
 * Substitute a symbolic link into short-form, so that the 'subslink()' code
 * can later expand it.
 */
static
subspath(
_ARX(char *,	path)
_ARX(int,	count)
_ARX(char *,	short_form)
_AR1(int,	x)
	)
_DCL(char *,	path)
_DCL(int,	count)
_DCL(char *,	short_form)
_DCL(int,	x)
{
	register char	*p = dedrung(count);
	register size_t	len = strlen(p);
	auto	 int	changed = FALSE;
	auto	 char	tmp[BUFSIZ];

	if (!path[len]) {		/* exact match ? */
		if (!strcmp(path,p)) {
			(void)strcpy(path, short_form);
			changed = TRUE;
		}
	} else if (path[len] == '/') {	/* prefix-match ? */
		if (!strncmp(path,p,len)) {
			(void)strcpy(tmp, path+len);
			(void)strcat(strcpy(path++, short_form), tmp);
			changed = TRUE;
		}
	}
	if (changed)
		path += strlen(short_form);
	len = strlen(xNAME(x));
	while (*path) {			/* substitute current-name */
		if (!strncmp(path, xNAME(x), len)) {
			(void)strcpy(tmp, path+len);
			(void)strcat(strcpy(path, "#"), tmp);
		}
		path++;
	}
	return (changed);
}

static
char *
link2bfr(
_ARX(char *,	dst)
_AR1(int,	x)
	)
_DCL(char *,	dst)
_DCL(int,	x)
{
	(void)name2bfr(dst, xLTXT(x));
	if (cmd_link) {
		if (!subspath(dst, 1, "%F", x))
			(void)subspath(dst, -1, "%B", x);
	}
	return (dst);
}

/*
 * Substitute user's short-hand notation back to normal link-text.
 */
static
char *
subslink(
_ARX(char *,	bfr)
_AR1(int,	x)
	)
_DCL(char *,	bfr)
_DCL(int,	x)
{
	auto	char	tmp[BUFSIZ];
	register char	*s = strcpy(tmp, bfr);
	register char	*d = bfr;
	register char	*t;

	while (*d = *s) {
		if (*s++ == '%') {
			if (*s++ == 'F')
				d += strlen(strcpy(d, dedrung(1)));
			else if (s[-1] == 'B')
				d += strlen(strcpy(d, dedrung(-1)));
			else {
				d++;
				s--;	/* point back just after '%' */
			}
		} else if (*d == '#') {
			t = xNAME(x);
			while (*d = *t++)	d++;
		} else
			d++;
	}
	return (bfr);
}
#endif	/* S_IFLNK */

/*
 * Coerce Xbase (left/right scrolling) so we can display a given column
 */
static
save_Xbase _ONE(int,col) /* leftmost column we need to show */
{
	auto	int	old = Xbase;
	if (col < Xbase)
		Xbase = 0;
	if (col > (Xbase + COLS - 1))
		Xbase = col;
	if (old != Xbase)
		showFILES(FALSE,FALSE);
	return (col - Xbase);
}

static
change_protection(_AR0)
{
	int	changed = FALSE;
	register int	c, x;

	(void)dedsigs(TRUE);	/* reset interrupt counter */
	c = CHMOD(FOO->curfile);
	for (x = 0; x < FOO->numfiles; x++) {
		if (GROUPED(x)) {
			if (dedsigs(TRUE)) {
				waitmsg(xNAME(x));
				break;
			}
			statLINE(FOO, x);
			changed++;
			if (c != CHMOD(x)) {
				dlog_comment("chmod %o %s\n",
					c, xNAME(x));
#ifdef	apollo_sr10
				if (has_extended_acl(x)
					&& !OWNER(x)) {
					errno = EPERM;
					warn(xNAME(x));
					break;
				}
#endif
				if (chmod(xNAME(x), c) < 0) {
					warn(xNAME(x));
					break;
				}
				fixtime(x);
			}
		}
	}
	return changed;
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * edit protection-code for current & tagged files
 */
editprot(_AR0)
{
register
int	y	= file2row(FOO->curfile),
	x	= 0,
	c;
auto	int
	opt	= FOO->P_opt,
	changed	= FALSE,
	done	= FALSE;
#ifdef	S_IFLNK
int	at_flag	= at_save();
#endif

	(void)save_Xbase(FOO->cmdcol[CCOL_PROT]);

	(void)replay('p');

	while (!done) {
	int	rwx,
		cols[3];

		showLINE(FOO, FOO->curfile);

		rwx	= (FOO->P_opt ? 1 : 3),
		cols[0] = FOO->cmdcol[CCOL_PROT];
		cols[1] = cols[0] + rwx;
		cols[2] = cols[1] + rwx;

		move(y, cols[x]);
		if (re_edit <= 0) refresh();
		switch (c = replay(EOS)) {
		case '\n':
		case 'p':
			changed = change_protection();
			done = TRUE;
			break;
		case 'q':
			(void)replay(-2);
			done = TRUE;
			break;
		case TO_FIRST:
			x = 0;
			break;
		case TO_LAST:
			x = 2;
			break;
		case ARO_RIGHT:
		case '\f':
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
				FOO->P_opt = !FOO->P_opt;
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
#ifdef	S_IFLNK
	if (at_flag) {		/* we had to toggle because of sym-link	*/
		(void)at_last(FALSE); /* force stat on the files, cleanup */
	}
#endif
	if (opt != FOO->P_opt) {
		FOO->P_opt = opt;
		showLINE(FOO, FOO->curfile);
	}
	restat(changed);
}

/*
 * Edit a text-field on the current display line.  Use the arrow keys for
 * moving within the line, and for setting/resetting insert mode.  Use
 * backspace to delete characters.
 */
edittext(
_ARX(int,	endc)
_ARX(int,	col)
_ARX(int,	len)
_AR1(char *,	bfr)
	)
_DCL(int,	endc)
_DCL(int,	col)
_DCL(int,	len)
_DCL(char *,	bfr)
{
int	y	= file2row(FOO->curfile),
	x	= 0,
	c,
	shift	= 0,			/* kludge to permit long edits */
	last,				/* length of edit-window */
	code	= TRUE,
	ec	= erasechar(),
	kc	= killchar(),
	insert	= FALSE,
	delete;
register char *s;

#ifdef	S_IFLNK
int	at_flag	= ((endc == 'u') || (endc == 'g')) ? at_save() : FALSE;
#endif

	dlog_comment("before \"%s\"\n", bfr);
	if (len < strlen(bfr) + 2)
		len = strlen(bfr) + 2;
	col = save_Xbase(col);
#ifdef	S_IFLNK
	if (at_flag)
		showLINE(FOO, FOO->curfile);
#endif
	(void)replay(endc);

	last = (COLS - 1 - col);
	if (last > len) last = len;

	for (;;) {
		move(y,col-1);
		PRINTW("%c", insert ? ' ' : '^');	/* a la rawgets */
		if (!insert)	standout();
		c = last;
		while (shift > x)
			shift -= 5;
		while ((x - shift) >= last)
			shift += 5;
		for (s = bfr + shift; (*s != EOS) && (c-- > 0); s++)
			addch(isprint(*s) ? *s : '?');
		while (c-- > 0)
			addch(' ');
		if (!insert)	standend();
		move(y,x-shift+col);
		if (re_edit <= 0) refresh();

		delete = -1;
		c = replay(EOS);

		if (c == '\n') {
			break;
		} else if (c == '\t') {
			insert = ! insert;
		} else if (insert) {
			if (isascii(c) && isprint(c)) {
			int	d,j = 0;
				do {
					d = c;
					c = bfr[x+j];
				} while (bfr[x+(j++)] = d);
				bfr[len-1] = EOS;
				if (x < len-1)	x++;
			} else if (c == ec) {
				delete = x-1;
			} else if (c == kc) {
				bfr[x = 0] = EOS;
			} else if (c == ARO_LEFT) {
				if (x > 0)	x--;
			} else if (c == ARO_RIGHT) {
				if (x < strlen(bfr))	x++;
			} else if (c == TO_FIRST) {
				x = 0;
			} else if (c == TO_LAST) {
				x = strlen(bfr);
			} else
				beep();
		} else {
			if (c == 'q') {
				(void)replay(-2);
				code = FALSE;
				break;
			} else if (c == endc) {
				break;
			} else if (c == '\b' || c == ARO_LEFT) {
				if (x > 0)	x--;
			} else if (c == '\f' || c == ARO_RIGHT) {
				if (x < strlen(bfr))	x++;
			} else if (c == TO_FIRST) {
				x = 0;
			} else if (c == TO_LAST) {
				x = strlen(bfr);
			} else
				beep();
		}
		if (delete >= 0) {
			x = delete;
			while (bfr[delete] = bfr[delete+1]) delete++;
		}
	}

#ifdef	S_IFLNK
	if (at_flag) {		/* we had to toggle because of sym-link	*/
		(void)at_last(FALSE); /* force stat on the files to cleanup */
	}
#endif
	dlog_flush();
	dlog_comment("after  \"%s\"\n", bfr);
	return (code);
}

/*
 * Change file's owner.
 */
edit_uid(_AR0)
{
register int j;
int	uid	= cSTAT.st_uid,
	changed	= FALSE;
char	bfr[BUFSIZ];

	if (FOO->G_opt == 1) {
		FOO->G_opt = 0;
		showFILES(FALSE,FALSE);
	}
	if (edittext('u', FOO->cmdcol[CCOL_UID], UIDLEN, strcpy(bfr, uid2s(uid)))
	&&  (uid = s2uid(bfr)) >= 0) {
		(void)dedsigs(TRUE);	/* reset interrupt-count */
		for (j = 0; j < FOO->numfiles; j++) {
			if (xSTAT(j).st_uid == uid)	continue;
			if (dedsigs(TRUE)) {
				waitmsg(xNAME(j));
				break;
			}
			if (GROUPED(j)) {
				if (chown(xNAME(j),
					uid, (int)xSTAT(j).st_gid) < 0) {
					warn(xNAME(j));
					break;
				}
				fixtime(j);
				xSTAT(j).st_uid = uid;
				changed++;
			}
		}
	}
	restat(changed);
}

/*
 * Change file's group.
 */
edit_gid(_AR0)
{
register int j;
int	gid	= cSTAT.st_gid,
	changed	= FALSE,
	root	= (geteuid() == 0);
char	bfr[BUFSIZ];

	if (!FOO->G_opt) {
		FOO->G_opt = 1;
		showFILES(FALSE,FALSE);
	}
	if (edittext('g', FOO->cmdcol[CCOL_GID], UIDLEN, strcpy(bfr, gid2s(gid)))
	&&  (gid = s2gid(bfr)) >= 0) {
	char	newgrp[BUFSIZ];
	static	char	*fmt = "chgrp -f %s %s";

		(void)dedsigs(TRUE);	/* reset interrupt-count */
		(void)strcpy(newgrp, gid2s(gid));
		for (j = 0; j < FOO->numfiles; j++) {
			if (xSTAT(j).st_gid == gid)	continue;
			if (dedsigs(TRUE)) {
				waitmsg(xNAME(j));
				break;
			}
			if (GROUPED(j)) {
				if (root) {
					if (chown(xNAME(j),
						(int)xSTAT(j).st_uid, gid) < 0){
						warn(xNAME(j));
						break;
					}
					xSTAT(j).st_gid = gid;
				} else {
					FORMAT(bfr, fmt, newgrp, fixname(j));
					(void)system(bfr);
				}
				fixtime(j);
				if (!root) {
					statLINE(FOO, j);
					showLINE(FOO, j);
					showC();
				} else
					changed++;
				if (xSTAT(j).st_gid != gid) {
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
editname(_AR0)
{
	auto	 int	changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

#define	EDITNAME(n)	edittext('=', FOO->cmdcol[CCOL_NAME], sizeof(bfr), name2bfr(bfr, n))
	if (EDITNAME(cNAME) && strcmp(cNAME, bfr)) {
		if (dedname(FOO->curfile, bfr) >= 0) {
			(void)dedsigs(TRUE);	/* reset interrupt count */
			re_edit = TRUE;
			for (j = 0; j < FOO->numfiles; j++) {
				if (j == FOO->curfile)
					continue;
				if (dedsigs(TRUE)) {
					waitmsg(xNAME(j));
					break;
				}
				if (xFLAG(j)) {
					(void)EDITNAME(xNAME(j));
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

#ifdef	S_IFLNK
/*
 * Change file's link-text
 */
editlink _ONE(int,cmd)
{
	auto	 int	col,
			changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

	cmd_link = (cmd == '<');

#define	EDITLINK(n)	edittext(cmd, col, sizeof(bfr), link2bfr(bfr, n))
	if (!cLTXT)
		beep();
	else {
		auto	int	restore = FALSE;
		col = save_Xbase(FOO->cmdcol[CCOL_NAME]);

		/* test if we must show substitution */
		if (cmd_link) {
			for (j = 0; j < FOO->numfiles; j++) {
				if (j == FOO->curfile)
					continue;
				if (xFLAG(j) && xLTXT(j)) {
					if (move2row(j, col)) {
						standout();
						PRINTW("-> ");
						standend();
						PRINTW("%.*s",
							COLS - col - 4,
							link2bfr(bfr, j));
						clrtoeol();
						restore = TRUE;
					}
				}
			}
		}

		(void)move2row(FOO->curfile, col);
		PRINTW("=> ");
		col += 3;
		if (EDITLINK(FOO->curfile)
		&&  strcmp(cLTXT, subslink(bfr,FOO->curfile))) {
			if (relink(FOO->curfile, bfr)) {
				(void)dedsigs(TRUE);
					/* reset interrupt count */
				re_edit = TRUE;
				for (j = 0; j < FOO->numfiles; j++) {
					if (j == FOO->curfile)
						continue;
					if (dedsigs(TRUE)) {
						waitmsg(xNAME(j));
						break;
					}
					if (xFLAG(j) && xLTXT(j)) {
						(void)EDITLINK(j);
						if (relink(j, subslink(bfr,j)))
							changed++;
						else
							break;
					}
				}
				re_edit = FALSE;
			}
		}
		if (restore && !changed)
			showFILES(FALSE,FALSE);
	}
	restat(changed);
}
#endif	/* S_IFLNK */

/*
 * Initiate/conclude repetition of inline editing.
 */
dedline _ONE(int,flag)
{
	return ((re_edit = flag) ? *lastedit : 0);
}
