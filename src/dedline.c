#ifndef	lint
static	char	sccs_id[] = "@(#)dedline.c	1.1 88/08/01 13:58:05";
#endif	lint

/*
 * Title:	dedline.c (directory-editor inline editing)
 * Author:	T.E.Dickey
 * Created:	01 Aug 1988 (from 'ded.c')
 * Modified:
 *
 * Function:	Procedures which perform in-line editing of particular fields
 *		of the file-list.
 */

#include	"ded.h"
extern	char	*fixname();

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
 * Save AT_opt mode when we are editing inline
 */
#ifndef	SYSTEM5
static
at_save()
{
	register int x;

	if (!AT_opt) {	/* chmod applies only to target of symbolic link */
		for (x = 0; x < numfiles; x++)
			if (x == curfile || flist[x].flag)
				if (flist[x].ltxt) {
					AT_opt = TRUE;
					statLINE(curfile);
					return (TRUE);
				}
	}
	return (FALSE);
}
#endif	SYSTEM5

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * edit protection-code for current & tagged files
 */
#define	CHMOD(n)	(flist[n].s.st_mode & 07777)

editprot()
{
register
int	y	= file2row(curfile),
	x	= 0,
	c,
	opt	= P_opt,
	changed	= FALSE,
	done	= FALSE;
#ifndef	SYSTEM5
int	at_flag	= at_save();
#endif	SYSTEM5

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
			c = CHMOD(curfile);
			for (x = 0; x < numfiles; x++) {
				if (flist[x].flag || x == curfile) {
					statLINE(x);
					changed++;
					if (c != CHMOD(x)) {
						if (chmod(flist[x].name, c) < 0) {
							warn(flist[x].name);
							break;
						}
						fixtime(x);
					}
				}
			}
			done = TRUE;
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
#ifndef	SYSTEM5
	if (at_flag) {		/* we had to toggle because of sym-link	*/
		AT_opt = FALSE;	/* restore mode we toggled from		*/
		changed = TRUE;	/* force restat on the files anyway	*/
	}
#endif	SYSTEM5
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
	ec	= erasechar(),
	kc	= killchar(),
	insert	= FALSE,
	delete;
register char *s;

	if ((col -= Xbase) < 1) {	/* convert to absolute column */
		col += Xbase;
		Xbase = 0;
		showFILES();
	}
	(void)replay(endc);

	for (;;) {
		move(y,col-1);
		printw("%c", insert ? ' ' : '^');	/* a la rawgets */
		if (!insert)	standout();
		for (s = bfr; *s; s++)
			addch(isprint(*s) ? *s : '?');
		while ((s++ - bfr) < len)
			addch(' ');
		if (!insert)	standend();
		move(y,x+col);
		if (!re_edit) refresh();

		delete = -1;
		c = replay(0);

		if (c == '\n') {
			return (TRUE);
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
				return (FALSE);
			} else if (c == endc) {
				return (TRUE);
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
}

/*
 * Change file's owner.
 */
edit_uid()
{
register int j;
int	uid,
	changed	= FALSE;
char	bfr[BUFSIZ];

	if (G_opt) {
		G_opt = FALSE;
		showFILES();
	}
	if (edittext('u', cmdcol[1], UIDLEN, strcpy(bfr, uid2s(cSTAT.st_uid)))
	&&  (uid = s2uid(bfr)) >= 0) {
		for (j = 0; j < numfiles; j++) {
			if (flist[j].s.st_uid == uid)	continue;
			if (flist[j].flag || (j == curfile)) {
				if (chown(flist[j].name,
					uid, flist[j].s.st_gid) < 0) {
					warn(flist[j].name);
					return;
				}
				fixtime(j);
				flist[j].s.st_uid = uid;
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
int	gid,
	changed	= FALSE,
	root	= (getuid() == 0);
char	bfr[BUFSIZ];

	if (!G_opt) {
		G_opt = TRUE;
		showFILES();
	}
	if (edittext('g', cmdcol[1], UIDLEN, strcpy(bfr, gid2s(cSTAT.st_gid)))
	&&  (gid = s2gid(bfr)) >= 0) {
	char	newgrp[BUFSIZ];
	static	char	*fmt = "chgrp -f %s %s";

		(void)strcpy(newgrp, gid2s(gid));
		for (j = 0; j < numfiles; j++) {
			if (flist[j].s.st_gid == gid)	continue;
			if (flist[j].flag || (j == curfile)) {
				if (root) {
					if (chown(flist[j].name,
						flist[j].s.st_uid, gid) < 0) {
						warn(flist[j].name);
						return;
					}
					flist[j].s.st_gid = gid;
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
				if (flist[j].s.st_gid != gid) {
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
int	len	= COLS - (cmdcol[3] - Xbase) - 1,
	changed	= 0;
register int	j;
char	bfr[BUFSIZ];

#define	EDITNAME(n)	edittext('=', cmdcol[3], len, strcpy(bfr, n))
	if (EDITNAME(cNAME)) {
		if (dedname(curfile, bfr) >= 0) {
			re_edit = TRUE;
			for (j = 0; j < numfiles; j++) {
				if (j == curfile)
					continue;
				if (flist[j].flag) {
					(void)EDITNAME(flist[j].name);
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

/*
 * Initiate/conclude repetition of inline editing.
 */
dedline(flag)
{
	if (re_edit = flag)
		return (*lastedit);
	return (0);
}
