#ifndef	lint
static	char	sccs_id[] = "@(#)dedline.c	1.10 88/09/02 13:13:30";
#endif	lint

/*
 * Title:	dedline.c (directory-editor inline editing)
 * Author:	T.E.Dickey
 * Created:	01 Aug 1988 (from 'ded.c')
 * Modified:
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
extern	char	*fixname();
extern	char	*txtalloc();

static	int	re_edit;		/* flag for 'edittext()' */
static	char	lastedit[BUFSIZ];	/* command-stream for 'edittext()' */

/************************************************************************
 *	local procedures						*
 ************************************************************************/

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
 * Save AT_opt mode when we are editing inline, and show mapped-thru stat for
 * symbolic links.
 */
#ifdef	S_IFLNK
static
at_save()
{
	if (!AT_opt) {	/* chmod applies only to target of symbolic link */
		return (at_last(TRUE));
	}
	return (FALSE);
}

/*
 * If any tagged files are symbolic links, set the AT_opt to the specified flag
 * value and re-stat them.  Return a count of the number of links.
 */
static
at_last(flag)
{
	register int x;
	register int changed = 0;

	for (x = 0; x < numfiles; x++)
		if (GROUPED(x)
		&& xLTXT(x)) {
			AT_opt = flag;
			statLINE(x);
			showLINE(x);
			changed++;
		}
	return (changed);
}

/*
 * Assign a new target for a symbolic link.
 */
static
relink(x, name)
char	*name;
{
	if (unlink(xNAME(x)) >= 0) {
		if (symlink(name, xNAME(x)) >= 0)
			return (TRUE);
		(void)symlink(xLTXT(x), xNAME(x));	/* try to restore */
	}
	waitmsg(xNAME(x));
	return (FALSE);
}
#endif	S_IFLNK

/*
 * Construct an editable-string for 'editname()' and 'editlink()'.
 * All nonprinting characters will be shown as '?'.
 */
static
char *
name2bfr(dst,src)
char	*dst;
char	*src;
{
	(void)name2s(dst, BUFSIZ, src, FALSE);
	return (dst);
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * edit protection-code for current & tagged files
 */
#define	CHMOD(n)	(xSTAT(n).st_mode & 07777)

editprot()
{
register
int	y	= file2row(curfile),
	x	= 0,
	c,
	opt	= P_opt,
	changed	= FALSE,
	done	= FALSE;
#ifdef	S_IFLNK
int	at_flag	= at_save();
#endif	S_IFLNK

	if (Xbase > 0) {
		Xbase = 0;
		showFILES();
	}

	(void)replay('p');

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
		case 'p':
			(void)dedsigs(TRUE);	/* reset interrupt counter */
			done = TRUE;
			c = CHMOD(curfile);
			for (x = 0; x < numfiles; x++) {
				if (GROUPED(x)) {
					if (dedsigs(TRUE)) {
						waitmsg(xNAME(x));
						break;
					}
					statLINE(x);
					changed++;
					if (c != CHMOD(x)) {
						if (chmod(xNAME(x), c) < 0) {
							warn(xNAME(x));
							break;
						}
						fixtime(x);
					}
				}
			}
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
#ifdef	S_IFLNK
	if (at_flag) {		/* we had to toggle because of sym-link	*/
		(void)at_last(FALSE); /* force stat on the files, cleanup */
	}
#endif	S_IFLNK
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
edittext(endc, col, len, bfr)
char	*bfr;
{
int	y	= file2row(curfile),
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
#endif	S_IFLNK

	if (len < strlen(bfr) + 2)
		len = strlen(bfr) + 2;
	if ((col -= Xbase) < 1) {	/* convert to absolute column */
		col += Xbase;
		Xbase = 0;
		showFILES();
	}
#ifdef	S_IFLNK
	else if (at_flag)
		showLINE(curfile);
#endif	S_IFLNK
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
		if (!re_edit) refresh();

		delete = -1;
		c = replay(0);

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
				code = FALSE;
				break;
			} else if (c == endc) {
				break;
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

#ifdef	S_IFLNK
	if (at_flag) {		/* we had to toggle because of sym-link	*/
		(void)at_last(FALSE); /* force stat on the files to cleanup */
	}
#endif	S_IFLNK
	return (code);
}

/*
 * Change file's owner.
 */
edit_uid()
{
register int j;
int	uid	= cSTAT.st_uid,
	changed	= FALSE;
char	bfr[BUFSIZ];

	if (G_opt) {
		G_opt = FALSE;
		showFILES();
	}
	if (edittext('u', cmdcol[1], UIDLEN, strcpy(bfr, uid2s(uid)))
	&&  (uid = s2uid(bfr)) >= 0) {
		(void)dedsigs(TRUE);	/* reset interrupt-count */
		for (j = 0; j < numfiles; j++) {
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
edit_gid()
{
register int j;
int	gid	= cSTAT.st_gid,
	changed	= FALSE,
	root	= (getuid() == 0);
char	bfr[BUFSIZ];

	if (!G_opt) {
		G_opt = TRUE;
		showFILES();
	}
	if (edittext('g', cmdcol[1], UIDLEN, strcpy(bfr, gid2s(gid)))
	&&  (gid = s2gid(bfr)) >= 0) {
	char	newgrp[BUFSIZ];
	static	char	*fmt = "chgrp -f %s %s";

		(void)dedsigs(TRUE);	/* reset interrupt-count */
		(void)strcpy(newgrp, gid2s(gid));
		for (j = 0; j < numfiles; j++) {
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
					statLINE(j);
					showLINE(j);
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
editname()
{
	auto	 int	changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

#define	EDITNAME(n)	edittext('=', cmdcol[3], sizeof(bfr), name2bfr(bfr, n))
	if (EDITNAME(cNAME)) {
		if (dedname(curfile, bfr) >= 0) {
			(void)dedsigs(TRUE);	/* reset interrupt count */
			re_edit = TRUE;
			for (j = 0; j < numfiles; j++) {
				if (j == curfile)
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
editlink()
{
	auto	 int	col	= cmdcol[3] + 3,
			changed	= 0;
	register int	j;
	auto	 char	bfr[BUFSIZ];

#define	EDITLINK(n)	edittext('>', col, sizeof(bfr), name2bfr(bfr, xLTXT(n)))
	if (!xLTXT(curfile))
		beep();
	else {
		move(file2row(curfile), cmdcol[3] - Xbase);
		PRINTW("=> ");
		if (EDITLINK(curfile)) {
			if (relink(curfile, bfr)) {
				(void)dedsigs(TRUE);
					/* reset interrupt count */
				re_edit = TRUE;
				for (j = 0; j < numfiles; j++) {
					if (j == curfile)
						continue;
					if (dedsigs(TRUE)) {
						waitmsg(xNAME(j));
						break;
					}
					if (xFLAG(j) && xLTXT(j)) {
						(void)EDITLINK(j);
						if (relink(j, bfr))
							changed++;
						else
							break;
					}
				}
				re_edit = FALSE;
			}
		}
	}
	restat(changed);
}
#endif	S_IFLNK

/*
 * Initiate/conclude repetition of inline editing.
 */
dedline(flag)
{
	return ((re_edit = flag) ? *lastedit : 0);
}
