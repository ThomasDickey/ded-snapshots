#ifndef	lint
static	char	Id[] = "$Id: dedline.c,v 12.0 1992/10/16 11:08:19 ste_cm Rel $";
#endif

/*
 * Title:	dedline.c (directory-editor inline editing)
 * Author:	T.E.Dickey
 * Created:	01 Aug 1988 (from 'ded.c')
 * Modified:
 *		27 May 1992, make '<' substitution recognize "%D" and "%d".
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

#define	SIZEOF(v)	(sizeof(v)/sizeof(v[0]))
#define	CHMOD(n)	(gSTAT(n).st_mode & 07777)
#define	OWNER(n)	((geteuid() == 0) || (gSTAT(x).st_uid == geteuid()))

#define	TO_FIRST	CTL('b')
#define	TO_LAST		CTL('f')

#define	EDITTEXT(cmd,col,len,buffer)\
	edittext(gbl, cmd, gbl->cmdcol[col], len, buffer)

/************************************************************************
 *	local procedures						*
 ************************************************************************/

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
private	int	subs_path(
	_ARX(char *,	path)
	_ARX(char *,	result)
	_AR1(char *,	short_form)
		)
	_DCL(char *,	path)
	_DCL(char *,	result)
	_DCL(char *,	short_form)
{
	register size_t	len = strlen(path);
	auto	 int	changed = FALSE;
	auto	 char	tmp[BUFSIZ];

	if (!result[len]) {		/* exact match ? */
		if (!strcmp(result,path)) {
			(void)strcpy(result, short_form);
			changed = TRUE;
		}
	} else if (result[len] == '/') { /* prefix-match ? */
		if (!strncmp(result,path,len)) {
			(void)strcpy(tmp, result+len);
			(void)strcat(strcpy(result, short_form), tmp);
			changed = TRUE;
		}
	}
	return (changed);
}

private	void	subs_leaf(
	_ARX(char *,	leaf)
	_AR1(char *,	result)
		)
	_DCL(char *,	leaf)
	_DCL(char *,	result)
{
	auto	 char	tmp[BUFSIZ];
	auto	 size_t	len = strlen(leaf);

	while (*result) {		/* substitute current-name */
		if (!strncmp(result, leaf, len)) {
			(void)strcpy(tmp, result+len);
			(void)strcat(strcpy(result, "#"), tmp);
		}
		result++;
	}
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
	(void)strcpy(dst, gLTXT(x));
	if (cmd_link) {
		static	struct	{
			char	*code;
			char	*path;
			} ppp[] = {
				"%F",	0,
				"%B",	0,
				"%d",	0,
				"%D",	old_wd
			};
		register int	j;
		size_t	maxlen	= 0;

		ppp[0].path = ring_path(gbl, 1);
		ppp[1].path = ring_path(gbl, -1);
		ppp[2].path = ring_path(gbl, 0);
		ppp[3].path = old_wd;

		/* ignore duplicates */
		if (!strcmp(ppp[0].path, ppp[2].path)) ppp[0].path = 0;
		if (!strcmp(ppp[1].path, ppp[2].path)) ppp[1].path = 0;
		if (!strcmp(ppp[3].path, ppp[2].path)) ppp[3].path = 0;

		/* find a starting length */
		for (j = 0; j < SIZEOF(ppp); j++) {
			if (ppp[j].path != 0) {
				size_t	len = strlen(ppp[j].path);
				if (len > maxlen)
					maxlen = len;
			}
		}

		/* match, looking for the longest strings first */
		do {
			size_t	next = 0;
			for (j = 0; j < SIZEOF(ppp); j++) {
				char	*path	= ppp[j].path;
				if (path != 0) {
					size_t	len = strlen(path);
					if (len < maxlen) {
						if (len > next)
							next = len;
					} else if (subs_path(path,
						dst, ppp[j].code)) {
						next = 0;
						break;
					} else
						ppp[j].path = 0;
				}
			}
			maxlen = next;

		} while (maxlen > 0);

		subs_leaf(gNAME(x), dst);
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
			switch (*s++) {
			case 'F':
				d += strlen(strcpy(d, ring_path(gbl, 1)));
				break;
			case 'B':
				d += strlen(strcpy(d, ring_path(gbl, -1)));
				break;
			case 'D':
				d += strlen(strcpy(d, old_wd));
				break;
			case 'd':
				d += strlen(strcpy(d, ring_path(gbl, 0)));
				break;
			default:
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
private	int	save_Xbase (
	_ARX(RING *,	gbl)
	_AR1(int,	col) /* leftmost column we need to show */
		)
	_DCL(RING *,	gbl)
	_DCL(int,	col)
{
	auto	int	old = gbl->Xbase;
	if (col < gbl->Xbase)
		gbl->Xbase = 0;
	if (col > (gbl->Xbase + COLS - 1))
		gbl->Xbase = col;
	if (old != gbl->Xbase)
		showFILES(gbl,FALSE,FALSE);
	return (col - gbl->Xbase);
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
					warn(gbl, gNAME(x));
					break;
				}
#endif
				if (chmod(gNAME(x), c) < 0) {
					warn(gbl, gNAME(x));
					break;
				}
				fixtime(gbl, x);
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
	STAT	*sb	= &cSTAT;
	int	y	= file2row(gbl->curfile),
		x	= 0,
		c;
#ifdef	S_IFLNK
	int	at_flag	= at_save(gbl);
#endif
	auto	int
		opt	= gbl->P_opt,
		changed	= FALSE,
		done	= FALSE,
		init	= TRUE,
		oldmode	= sb->st_mode;

	(void)save_Xbase(gbl, gbl->cmdcol[CCOL_PROT]);

	ReplayStart('p');

	while (!done) {
	int	rwx,
		cols[3];

		if (init) {
			x = 0;
			init = FALSE;
			sb->st_mode = oldmode;
		}
		showLINE(gbl, gbl->curfile);

		rwx	= (gbl->P_opt ? 1 : 3),
		cols[0] = gbl->cmdcol[CCOL_PROT];
		cols[1] = cols[0] + rwx;
		cols[2] = cols[1] + rwx;

		move(y, cols[x]);
		switch (c = ReplayChar()) {
		case '\n':
		case 'p':
			ReplayFinish();
			changed = change_protection(gbl);
			done = TRUE;
			break;
		case 'q':
			ReplayQuit();
			done = TRUE;
			break;
		case TO_FIRST:
			x = 0;
			break;
		case TO_LAST:
			x = 2;
			break;
		case ARO_UP:
			init = up_inline();
			break;
		case ARO_DOWN:
			init = down_inline();
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
	restat(gbl,changed);
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
	static	DYN	*result;
	int	y	= file2row(gbl->curfile),
		code	= TRUE;
	register char *s;

#ifdef	S_IFLNK
	int	at_flag	= ((endc == 'u') || (endc == 'g'))
			? at_save(gbl)
			: FALSE;
#endif

	dlog_comment("before \"%s\"\n", bfr);
	if (len < strlen(bfr) + 2)
		len = strlen(bfr) + 2;
	col = save_Xbase(gbl, col);
#ifdef	S_IFLNK
	if (at_flag)
		showLINE(gbl, gbl->curfile);
#endif
	ReplayStart(endc);

	move(y,col);
	result = dyn_alloc(result, (size_t)len+1);
	result = dyn_copy (result, bfr);
	if (s = dlog_string(
			&result,
			inline_text(),
			inline_hist(),
			endc,
			len)) {
		(void)strcpy(bfr, s);
		ReplayFinish();
	} else {
		ReplayQuit();
		code = FALSE;
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
	int	uid	= cSTAT.st_uid,
		changed	= FALSE;
	char	bfr[BUFSIZ];

	if (gbl->G_opt == 1) {
		gbl->G_opt = 0;
		showFILES(gbl,FALSE,FALSE);
	}

	if (EDITTEXT('u', CCOL_UID, UIDLEN, strcpy(bfr, uid2s(uid)))
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
					warn(gbl, gNAME(j));
					break;
				}
				fixtime(gbl, j);
				gSTAT(j).st_uid = uid;
				changed++;
			}
		}
	}
	restat(gbl,changed);
}

/*
 * Change file's group.
 */
public	void	edit_gid _ONE(RING *,gbl)
{
	register int j;
	int	gid	= cSTAT.st_gid,
		changed	= FALSE,
		root	= (geteuid() == 0);
	char	bfr[BUFSIZ];

	if (!gbl->G_opt) {
		gbl->G_opt = 1;
		showFILES(gbl,FALSE,FALSE);
	}

	if (EDITTEXT('g', CCOL_GID, UIDLEN, strcpy(bfr, gid2s(gid)))
	&&  (gid = s2gid(bfr)) >= 0) {
		auto	char	newgrp[BUFSIZ];
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
						warn(gbl, gNAME(j));
						break;
					}
					gSTAT(j).st_gid = gid;
				} else {
					FORMAT(bfr, fmt, newgrp, fixname(gbl,j));
					(void)system(bfr);
				}
				fixtime(gbl, j);
				if (!root) {
					statLINE(gbl, j);
					showLINE(gbl, j);
					showC(gbl);
				} else
					changed++;
				if (gSTAT(j).st_gid != gid) {
					FORMAT(bfr, fmt, newgrp, fixname(gbl,j));
					dedmsg(gbl, bfr);
					beep();
					break;
				}
			}
		}
	}
	restat(gbl,changed);
}

/*
 * Change file's name
 */
public	void	editname _ONE(RING *,gbl)
{
	auto	 int	changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

#define	EDITNAME(n)	EDITTEXT('=', CCOL_NAME, sizeof(bfr), strcpy(bfr, n))

	if (EDITNAME(cNAME) && strcmp(cNAME, bfr)) {
		if (dedname(gbl, gbl->curfile, bfr) >= 0) {
			(void)dedsigs(TRUE);	/* reset interrupt count */
			hide_inline(TRUE);
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
			hide_inline(FALSE);
		}
	}
	restat(gbl,changed);
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

#define	EDITLINK(n) edittext(gbl, cmd, col, sizeof(bfr), link2bfr(gbl, bfr, n))

	if (!cLTXT)
		beep();
	else {
		auto	int	restore = FALSE;
		col = save_Xbase(gbl, gbl->cmdcol[CCOL_NAME]);

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
		&&  strcmp(cLTXT,
			subslink(gbl, bfr, gbl->curfile))) {
			if (relink(gbl, gbl->curfile, bfr)) {
				(void)dedsigs(TRUE);
					/* reset interrupt count */
				hide_inline(TRUE);
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
				hide_inline(FALSE);
			}
		}
		if (restore && !changed)
			showFILES(gbl,FALSE,FALSE);
	}
	restat(gbl,changed);
}
#endif	/* S_IFLNK */
