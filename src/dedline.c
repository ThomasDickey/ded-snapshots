#ifndef	lint
static	char	Id[] = "$Id: dedline.c,v 10.7 1992/04/02 12:49:56 dickey Exp $";
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

#define	CHMOD(n)	(gSTAT(n).st_mode & 07777)
#define	OWNER(n)	((geteuid() == 0) || (gSTAT(x).st_uid == geteuid()))

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
public	int	replay _ONE(int,c)
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
private	char *	name2bfr(
	_ARX(char *,	dst)
	_AR1(char *,	src)
		)
	_DCL(char *,	dst)
	_DCL(char *,	src)
{
	(void)name2s(dst, BUFSIZ, src, FALSE);
	return (dst);
}

#ifdef	S_IFLNK
/*
 * If any tagged files are symbolic links, set the AT_opt to the specified flag
 * value and re-stat them.  Return a count of the number of links.
 */
private	int	at_last(
	_ARX(RING *,	gbl)
	_AR1(int,	flag)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	flag)
{
	register int x;
	register int changed = 0;

	for (x = 0; x < gbl->numfiles; x++)
		if (GROUPED(x)
		&& gLTXT(x)) {
			gbl->AT_opt = flag;
			statLINE(gbl, x);
			showLINE(gbl, x);
			changed++;
		}
	return (changed);
}

/*
 * Save AT_opt mode when we are editing inline, and show mapped-thru stat for
 * symbolic links.
 */
private	int	at_save _ONE(RING *,gbl)
{
	if (!gbl->AT_opt) {
		/* chmod applies only to target of symbolic link */
		return (at_last(gbl, TRUE));
	}
	return (FALSE);
}

/*
 * Assign a new target for a symbolic link.
 */
private	int	relink(
	_ARX(RING *,	gbl)
	_ARX(int,	x)
	_AR1(char *,	name)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	x)
	_DCL(char *,	name)
{
	dlog_comment("relink \"%s\" (link=%s)\n", name, gNAME(x));
	if (unlink(gNAME(x)) >= 0) {
		if (symlink(name, gNAME(x)) >= 0)
			return (TRUE);
		(void)symlink(gLTXT(x), gNAME(x));	/* try to restore */
	}
	waitmsg(gNAME(x));
	return (FALSE);
}

private	int	cmd_link;	/* true if we use short-form */

/*
 * Substitute a symbolic link into short-form, so that the 'subslink()' code
 * can later expand it.
 */
private	int	subspath(
	_ARX(RING *,	gbl)
	_ARX(char *,	path)
	_ARX(int,	count)
	_ARX(char *,	short_form)
	_AR1(int,	x)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
	_DCL(int,	count)
	_DCL(char *,	short_form)
	_DCL(int,	x)
{
	register char	*p = ring_path(count);
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
	len = strlen(gNAME(x));
	while (*path) {			/* substitute current-name */
		if (!strncmp(path, gNAME(x), len)) {
			(void)strcpy(tmp, path+len);
			(void)strcat(strcpy(path, "#"), tmp);
		}
		path++;
	}
	return (changed);
}

private	char *	link2bfr(
	_ARX(RING *,	gbl)
	_ARX(char *,	dst)
	_AR1(int,	x)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	dst)
	_DCL(int,	x)
{
	(void)name2bfr(dst, gLTXT(x));
	if (cmd_link) {
		if (!subspath(gbl, dst, 1, "%F", x))
			(void)subspath(gbl, dst, -1, "%B", x);
	}
	return (dst);
}

/*
 * Substitute user's short-hand notation back to normal link-text.
 */
private	char *	subslink(
	_ARX(RING *,	gbl)
	_ARX(char *,	bfr)
	_AR1(int,	x)
		)
	_DCL(RING *,	gbl)
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
				d += strlen(strcpy(d, ring_path(1)));
			else if (s[-1] == 'B')
				d += strlen(strcpy(d, ring_path(-1)));
			else {
				d++;
				s--;	/* point back just after '%' */
			}
		} else if (*d == '#') {
			t = gNAME(x);
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
private	int	save_Xbase _ONE(int,col) /* leftmost column we need to show */
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

private	int	change_protection _ONE(RING *,gbl)
{
	int	changed = FALSE;
	register int	c, x;

	(void)dedsigs(TRUE);	/* reset interrupt counter */
	c = CHMOD(gbl->curfile);
	for (x = 0; x < gbl->numfiles; x++) {
		if (GROUPED(x)) {
			if (dedsigs(TRUE)) {
				waitmsg(gNAME(x));
				break;
			}
			statLINE(gbl, x);
			changed++;
			if (c != CHMOD(x)) {
				dlog_comment("chmod %o %s\n",
					c, gNAME(x));
#ifdef	apollo_sr10
				if (has_extended_acl(gbl, x)
					&& !OWNER(x)) {
					errno = EPERM;
					warn(gNAME(x));
					break;
				}
#endif
				if (chmod(gNAME(x), c) < 0) {
					warn(gNAME(x));
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
public	void	editprot _ONE(RING *,gbl)
{
	register
	STAT	*sb	= &gSTAT(gbl->curfile);
	int	y	= file2row(gbl->curfile),
		x	= 0,
		c;
	auto	int
		opt	= gbl->P_opt,
		changed	= FALSE,
		done	= FALSE;
#ifdef	S_IFLNK
	int	at_flag	= at_save(gbl);
#endif

	(void)save_Xbase(gbl->cmdcol[CCOL_PROT]);

	(void)replay('p');

	while (!done) {
	int	rwx,
		cols[3];

		showLINE(gbl, gbl->curfile);

		rwx	= (gbl->P_opt ? 1 : 3),
		cols[0] = gbl->cmdcol[CCOL_PROT];
		cols[1] = cols[0] + rwx;
		cols[2] = cols[1] + rwx;

		move(y, cols[x]);
		if (re_edit <= 0) refresh();
		switch (c = replay(EOS)) {
		case '\n':
		case 'p':
			changed = change_protection(gbl);
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
				sb->st_mode &= ~(7      << shift);
				sb->st_mode |= ((c-'0') << shift);
				if (x < 2)
					x++;
			} else if (c == 'P') {
				gbl->P_opt = !gbl->P_opt;
			} else if (c == 's') {
				if (x == 0)
					sb->st_mode ^= S_ISUID;
				else if (x == 1)
					sb->st_mode ^= S_ISGID;
				else
					beep();
			} else if (c == 't') {
				if (x == 2)
					sb->st_mode ^= S_ISVTX;
				else
					beep();
			} else
				beep();
		}
	}
#ifdef	S_IFLNK
	if (at_flag) {		/* we had to toggle because of sym-link	*/
		(void)at_last(gbl, FALSE);
		/* force stat on the files, cleanup */
	}
#endif
	if (opt != gbl->P_opt) {
		gbl->P_opt = opt;
		showLINE(gbl, gbl->curfile);
	}
	restat(changed);
}

/*
 * Edit a text-field on the current display line.  Use the arrow keys for
 * moving within the line, and for setting/resetting insert mode.  Use
 * backspace to delete characters.
 */
public	int	edittext(
	_ARX(RING *,	gbl)
	_ARX(int,	endc)
	_ARX(int,	col)
	_ARX(int,	len)
	_AR1(char *,	bfr)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	endc)
	_DCL(int,	col)
	_DCL(int,	len)
	_DCL(char *,	bfr)
{
	int	y	= file2row(gbl->curfile),
		x	= 0,
		c,
		shift	= 0,		/* kludge to permit long edits */
		last,			/* length of edit-window */
		code	= TRUE,
		ec	= erasechar(),
		kc	= killchar(),
		insert	= FALSE,
		delete;
	register char *s;

#ifdef	S_IFLNK
	int	at_flag	= ((endc == 'u') || (endc == 'g')) ? at_save(gbl) : FALSE;
#endif

	dlog_comment("before \"%s\"\n", bfr);
	if (len < strlen(bfr) + 2)
		len = strlen(bfr) + 2;
	col = save_Xbase(col);
#ifdef	S_IFLNK
	if (at_flag)
		showLINE(gbl, gbl->curfile);
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
		(void)at_last(gbl, FALSE);
		/* force stat on the files to cleanup */
	}
#endif
	dlog_flush();
	dlog_comment("after  \"%s\"\n", bfr);
	return (code);
}

/*
 * Change file's owner.
 */
public	void	edit_uid _ONE(RING *,gbl)
{
	register int j;
	int	uid	= gSTAT(gbl->curfile).st_uid,
		changed	= FALSE;
	char	bfr[BUFSIZ];

	if (gbl->G_opt == 1) {
		gbl->G_opt = 0;
		showFILES(FALSE,FALSE);
	}
	if (edittext(gbl, 'u', gbl->cmdcol[CCOL_UID], UIDLEN, strcpy(bfr, uid2s(uid)))
	&&  (uid = s2uid(bfr)) >= 0) {
		(void)dedsigs(TRUE);	/* reset interrupt-count */
		for (j = 0; j < gbl->numfiles; j++) {
			if (gSTAT(j).st_uid == uid)	continue;
			if (dedsigs(TRUE)) {
				waitmsg(gNAME(j));
				break;
			}
			if (GROUPED(j)) {
				if (chown(gNAME(j),
					uid, (int)gSTAT(j).st_gid) < 0) {
					warn(gNAME(j));
					break;
				}
				fixtime(j);
				gSTAT(j).st_uid = uid;
				changed++;
			}
		}
	}
	restat(changed);
}

/*
 * Change file's group.
 */
public	void	edit_gid _ONE(RING *,gbl)
{
	register int j;
	int	gid	= gSTAT(gbl->curfile).st_gid,
		changed	= FALSE,
		root	= (geteuid() == 0);
	char	bfr[BUFSIZ];

	if (!gbl->G_opt) {
		gbl->G_opt = 1;
		showFILES(FALSE,FALSE);
	}
	if (edittext(gbl, 'g', gbl->cmdcol[CCOL_GID], UIDLEN, strcpy(bfr, gid2s(gid)))
	&&  (gid = s2gid(bfr)) >= 0) {
	char	newgrp[BUFSIZ];
	static	char	*fmt = "chgrp -f %s %s";

		(void)dedsigs(TRUE);	/* reset interrupt-count */
		(void)strcpy(newgrp, gid2s(gid));
		for (j = 0; j < gbl->numfiles; j++) {
			if (gSTAT(j).st_gid == gid)	continue;
			if (dedsigs(TRUE)) {
				waitmsg(gNAME(j));
				break;
			}
			if (GROUPED(j)) {
				if (root) {
					if (chown(gNAME(j),
						(int)gSTAT(j).st_uid, gid) < 0){
						warn(gNAME(j));
						break;
					}
					gSTAT(j).st_gid = gid;
				} else {
					FORMAT(bfr, fmt, newgrp, fixname(j));
					(void)system(bfr);
				}
				fixtime(j);
				if (!root) {
					statLINE(gbl, j);
					showLINE(gbl, j);
					showC();
				} else
					changed++;
				if (gSTAT(j).st_gid != gid) {
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
public	void	editname _ONE(RING *,gbl)
{
	auto	 int	changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

#define	EDITNAME(n)	edittext(gbl, '=', gbl->cmdcol[CCOL_NAME], sizeof(bfr), name2bfr(bfr, n))
	if (EDITNAME(gNAME(gbl->curfile)) && strcmp(gNAME(gbl->curfile), bfr)) {
		if (dedname(gbl, gbl->curfile, bfr) >= 0) {
			(void)dedsigs(TRUE);	/* reset interrupt count */
			re_edit = TRUE;
			for (j = 0; j < gbl->numfiles; j++) {
				if (j == gbl->curfile)
					continue;
				if (dedsigs(TRUE)) {
					waitmsg(gNAME(j));
					break;
				}
				if (gFLAG(j)) {
					(void)EDITNAME(gNAME(j));
					if (dedname(gbl, j, bfr) >= 0)
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
public	void	editlink(
	_ARX(RING *,	gbl)
	_AR1(int,	cmd)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	cmd)
{
	auto	 int	col,
			changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

	cmd_link = (cmd == '<');

#define	EDITLINK(n)	edittext(gbl, cmd, col, sizeof(bfr), link2bfr(gbl, bfr, n))
	if (!gLTXT(gbl->curfile))
		beep();
	else {
		auto	int	restore = FALSE;
		col = save_Xbase(gbl->cmdcol[CCOL_NAME]);

		/* test if we must show substitution */
		if (cmd_link) {
			for (j = 0; j < gbl->numfiles; j++) {
				if (j == gbl->curfile)
					continue;
				if (gFLAG(j) && gLTXT(j)) {
					if (move2row(j, col)) {
						standout();
						PRINTW("-> ");
						standend();
						PRINTW("%.*s",
							COLS - col - 4,
							link2bfr(gbl, bfr, j));
						clrtoeol();
						restore = TRUE;
					}
				}
			}
		}

		(void)move2row(gbl->curfile, col);
		PRINTW("=> ");
		col += 3;
		if (EDITLINK(gbl->curfile)
		&&  strcmp(gLTXT(gbl->curfile),
			subslink(gbl, bfr, gbl->curfile))) {
			if (relink(gbl, gbl->curfile, bfr)) {
				(void)dedsigs(TRUE);
					/* reset interrupt count */
				re_edit = TRUE;
				for (j = 0; j < gbl->numfiles; j++) {
					if (j == gbl->curfile)
						continue;
					if (dedsigs(TRUE)) {
						waitmsg(gNAME(j));
						break;
					}
					if (gFLAG(j) && gLTXT(j)) {
						(void)EDITLINK(j);
						if (relink(gbl, j, subslink(gbl, bfr,j)))
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
