#ifndef	lint
static	char	Id[] = "$Id: dedline.c,v 9.0 1991/05/15 13:40:16 ste_cm Rel $";
#endif

/*
 * Title:	dedline.c (directory-editor inline editing)
 * Author:	T.E.Dickey
 * Created:	01 Aug 1988 (from 'ded.c')
 * $Log: dedline.c,v $
 * Revision 9.0  1991/05/15 13:40:16  ste_cm
 * BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *
 *		Revision 8.2  91/05/15  13:40:16  dickey
 *		mods to accommodate apollo sr10.3
 *		
 *		Revision 8.1  91/04/18  10:43:16  dickey
 *		fixed end-of-buffer code for 'edittext()' (caused spurious
 *		data overwrites).
 *		
 *		Revision 8.0  90/03/06  08:26:44  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  90/03/06  08:26:44  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/03/06  08:26:44  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.1  90/03/06  08:26:44  dickey
 *		lint
 *		
 *		Revision 5.0  89/10/26  11:58:38  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.3  89/10/26  11:58:38  dickey
 *		altered 'editmode()' to reduce number of register variables
 *		used (bypasses bug on sun3)
 *		
 *		Revision 4.2  89/10/12  16:09:10  dickey
 *		altered format so that uid,gid columns are not necessarily
 *		obscured (G_opt == 2).  also, prevent chmod if object has
 *		extended acls -- and user is not owner (prevents trouble!)
 *		
 *		Revision 4.1  89/10/06  09:40:17  dickey
 *		modified treatment of 'cmdcol[]' (cf: showFILES)
 *		
 *		Revision 4.0  89/08/11  14:26:07  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.1  89/08/11  14:26:07  dickey
 *		modified "<" command so that we show all intermediate
 *		substitutions (i.e., "%F", "%B" and "#") which would be
 *		applied to a tagged-group -- before we begin editing.
 *		
 *		Revision 3.0  89/06/12  13:12:51  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.1  89/06/12  13:12:51  dickey
 *		corrected '<' command-substitution, which lost chars after
 *		the '#' substitution.
 *		
 *		Revision 2.0  89/03/14  13:19:29  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.14  89/03/14  13:19:29  dickey
 *		sccs2rcs keywords
 *		
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
extern	char	*dedrung();
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
name2bfr(dst,src)
char	*dst;
char	*src;
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
subspath(path, count, short_form, x)
char	*path;
char	*short_form;
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
link2bfr(dst, x)
char	*dst;
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
subslink(bfr,x)
char	*bfr;
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
save_Xbase(col)
int	col;			/* leftmost column we need to show */
{
	auto	int	old = Xbase;
	if (col < Xbase)
		Xbase = 0;
	if (col > (Xbase + COLS - 1))
		Xbase = col;
	if (old != Xbase)
		showFILES(FALSE);
	return (col - Xbase);
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
	c;
auto	int
	opt	= P_opt,
	changed	= FALSE,
	done	= FALSE;
#ifdef	S_IFLNK
int	at_flag	= at_save();
#endif

	(void)save_Xbase(cmdcol[CCOL_PROT]);

	(void)replay('p');

	while (!done) {
	int	rwx,
		cols[3];

		showLINE(curfile);

		rwx	= (P_opt ? 1 : 3),
		cols[0] = cmdcol[CCOL_PROT];
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
						dlog_comment("chmod %o %s\n",
							c, xNAME(x));
#ifdef	apollo_sr10
						if (has_extended_acl(x)
						&& xSTAT(x).st_uid != getuid()){
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
#endif
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
#endif

	dlog_comment("before \"%s\"\n", bfr);
	if (len < strlen(bfr) + 2)
		len = strlen(bfr) + 2;
	col = save_Xbase(col);
#ifdef	S_IFLNK
	if (at_flag)
		showLINE(curfile);
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
				bfr[len-1] = EOS;
				if (x < len-1)	x++;
			} else if (c == ec) {
				delete = x-1;
			} else if (c == kc) {
				delete = x;
			} else if (c == ARO_LEFT) {
				if (x > 0)	x--;
			} else if (c == ARO_RIGHT) {
				if (x < strlen(bfr))	x++;
			} else if (c == CTL('b')) {
				x = 0;
			} else if (c == CTL('f')) {
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
			} else if (c == CTL('b')) {
				x = 0;
			} else if (c == CTL('f')) {
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
edit_uid()
{
register int j;
int	uid	= cSTAT.st_uid,
	changed	= FALSE;
char	bfr[BUFSIZ];

	if (G_opt == 1) {
		G_opt = 0;
		showFILES(FALSE);
	}
	if (edittext('u', cmdcol[CCOL_UID], UIDLEN, strcpy(bfr, uid2s(uid)))
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
		G_opt = 1;
		showFILES(FALSE);
	}
	if (edittext('g', cmdcol[CCOL_GID], UIDLEN, strcpy(bfr, gid2s(gid)))
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

#define	EDITNAME(n)	edittext('=', cmdcol[CCOL_NAME], sizeof(bfr), name2bfr(bfr, n))
	if (EDITNAME(cNAME) && strcmp(cNAME, bfr)) {
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
editlink(cmd)
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
		col = save_Xbase(cmdcol[CCOL_NAME]);

		/* test if we must show substitution */
		if (cmd_link) {
			for (j = 0; j < numfiles; j++) {
				if (j == curfile)
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

		(void)move2row(curfile, col);
		PRINTW("=> ");
		col += 3;
		if (EDITLINK(curfile)
		&&  strcmp(cLTXT, subslink(bfr,curfile))) {
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
			showFILES(FALSE);
	}
	restat(changed);
}
#endif	/* S_IFLNK */

/*
 * Initiate/conclude repetition of inline editing.
 */
dedline(flag)
{
	return ((re_edit = flag) ? *lastedit : 0);
}
