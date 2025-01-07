/*
 * Title:	dedline.c (directory-editor inline editing)
 * Author:	T.E.Dickey
 * Created:	01 Aug 1988 (from 'ded.c')
 * Modified:
 *		02 May 2020, log errors from chdir.
 *		14 Dec 2014, coverity warnings
 *		25 May 2010, fix clang --analyze warnings.
 *		07 Sep 2004, add editdate().
 *		07 Mar 2004, remove K&R support, indent'd
 *		01 Mar 1998, mods to build with OS/2 EMX 0.9b
 *		15 Feb 1998, remove special code for apollo sr10
 *			     fix compiler warnings.
 *		05 Nov 1995, use 80th column
 *		23 Jul 1994, removed apollo chgrp hack (spawning commands).
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
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

MODULE_ID("$Id: dedline.c,v 12.38 2025/01/07 01:22:07 tom Exp $")

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
static int
at_last(RING * gbl, int flag)
{
    unsigned x;
    int changed = 0;

    for_each_file(gbl, x)
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
static int
at_save(RING * gbl)
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
static int
relink(RING * gbl, unsigned x, char *name)
{
    dlog_comment("relink \"%s\" (link=%s)\n", name, gNAME(x));
    if (unlink(gNAME(x)) >= 0) {
	if (symlink(name, gNAME(x)) >= 0)
	    return (TRUE);
	if (symlink(gLTXT(x), gNAME(x)) < 0)	/* try to restore */
	    dlog_comment("relink failed: %s\n", strerror(errno));
    }
    waitmsg(gNAME(x));
    return (FALSE);
}

static int cmd_link;		/* true if we use short-form */

/*
 * Substitute a symbolic link into short-form, so that the 'subslink()' code
 * can later expand it.
 */
static int
subs_path(const char path[MAXPATHLEN], char result[MAXPATHLEN], const char *short_form)
{
    size_t len = strlen(path);
    int changed = FALSE;
    char tmp[MAXPATHLEN];

    if (!result[len]) {		/* exact match ? */
	if (!strcmp(result, path)) {
	    (void) strcpy(result, short_form);
	    changed = TRUE;
	}
    } else if (result[len] == '/') {	/* prefix-match ? */
	if (!strncmp(result, path, len) &&
	    strlen(result + len) < sizeof(tmp)) {
	    (void) strcpy(tmp, result + len);
	    (void) strcat(strcpy(result, short_form), tmp);
	    changed = TRUE;
	}
    }
    return (changed);
}

static void
subs_leaf(char leaf[MAXPATHLEN], char result[MAXPATHLEN])
{
    char tmp[MAXPATHLEN];
    size_t len = strlen(leaf);

    while (*result) {		/* substitute current-name */
	if (!strncmp(result, leaf, len)
	    && (strlen(result + len) < (sizeof(tmp) - 3))) {
	    (void) strcpy(tmp, result + len);
	    (void) strcat(strcpy(result, "#"), tmp);
	}
	result++;
    }
}

static char *
link2bfr(RING * gbl, char dst[MAXPATHLEN], unsigned x)
{
    (void) strcpy(dst, gLTXT(x));
    if (cmd_link) {
	/* *INDENT-OFF* */
	static struct {
	    const char *code;
	    const char *path;
	} ppp[] = {
	    { "%F", NULL },
	    { "%B", NULL },
	    { "%d", NULL },
	    { "%D", old_wd }
	};
	/* *INDENT-ON* */

	unsigned j;
	size_t maxlen = 0;

	ppp[0].path = ring_path(gbl, 1);
	ppp[1].path = ring_path(gbl, -1);
	ppp[2].path = ring_path(gbl, 0);
	ppp[3].path = old_wd;

	/* ignore duplicates */
	if (!strcmp(ppp[0].path, ppp[2].path))
	    ppp[0].path = NULL;
	if (!strcmp(ppp[1].path, ppp[2].path))
	    ppp[1].path = NULL;
	if (!strcmp(ppp[3].path, ppp[2].path))
	    ppp[3].path = NULL;

	/* find a starting length */
	for (j = 0; j < SIZEOF(ppp); j++) {
	    if (ppp[j].path != NULL) {
		size_t len = strlen(ppp[j].path);
		if (len > maxlen)
		    maxlen = len;
	    }
	}

	/* match, looking for the longest strings first */
	do {
	    size_t next = 0;
	    for (j = 0; j < SIZEOF(ppp); j++) {
		const char *path = ppp[j].path;
		if (path != NULL) {
		    size_t len = strlen(path);
		    if (len < maxlen) {
			if (len > next)
			    next = len;
		    } else if (subs_path(path,
					 dst, ppp[j].code)) {
			next = 0;
			break;
		    } else
			ppp[j].path = NULL;
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
static char *
subslink(RING * gbl, char bfr[MAXPATHLEN], unsigned x)
{
    char tmp[MAXPATHLEN];

    if (strlen(bfr) < sizeof(tmp)) {
	char *s = strcpy(tmp, bfr);
	char *d = bfr;
	char *t;
	while ((*d = *s) != EOS) {
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
		while ((*d = *t++) != EOS)
		    d++;
	    } else
		d++;
	}
    }
    return (bfr);
}
#endif /* S_IFLNK */

/*
 * Coerce Xbase (left/right scrolling) so we can display a given column
 */
static int
save_Xbase(RING * gbl,
	   int col)		/* leftmost column we need to show */
{
    int old = gbl->Xbase;
    if (col < gbl->Xbase)
	gbl->Xbase = 0;
    if (col > (gbl->Xbase + COLS))
	gbl->Xbase = col;
    if (old != gbl->Xbase)
	showFILES(gbl, FALSE);
    return (col - gbl->Xbase);
}

static int
change_protection(RING * gbl)
{
    int changed = FALSE;
    int c;
    unsigned x;

    (void) dedsigs(TRUE);	/* reset interrupt counter */
    c = (int) CHMOD(gbl->curfile);
    for_each_file(gbl, x) {
	if (GROUPED(x)) {
	    if (dedsigs(TRUE)) {
		waitmsg(gNAME(x));
		break;
	    }
	    statLINE(gbl, x);
	    changed++;
	    if (c != (int) CHMOD(x)) {
		dlog_comment("chmod %o %s\n",
			     c, gNAME(x));
		if (chmod(gNAME(x), (mode_t) c) < 0) {
		    warn(gbl, gNAME(x));
		    break;
		}
		fixtime(gbl, x);
	    }
	}
    }
    return changed;
}

static int
day_of_month(time_t when)
{
    struct tm *tm = localtime(&when);
    return tm->tm_mday;
}

static void
toggle_timestamp(time_t *stamp, int field, int by)
{
    time_t when = *stamp;
    int before;
    int after;

    /* datemask[] = "ddd mmm DD HH:MM:SS YYYY" */
    switch (field) {
    case 0:
    case 2:
	when += (by * DAY);
	break;
    case 1:
	before = day_of_month(when);
	when += (by * 30 * DAY);
	after = day_of_month(when);
	when += (before - after) * DAY;
	break;
    case 3:
	when += (by * HOUR);
	break;
    case 4:
	when += (by * MINUTE);
	break;
    case 5:
	when += by;
	break;
    case 6:
	before = day_of_month(when);
	when += (by * 365 * DAY);
	after = day_of_month(when);
	when += (before - after) * DAY;
	break;
    }
    *stamp = when;
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/*
 * edit protection-code for current & tagged files
 */
void
editprot(RING * gbl)
{
    Stat_t *sb = &cSTAT;
    int y = file2row(gbl->curfile), x = 0, c;
#ifdef	S_IFLNK
    int at_flag = at_save(gbl);
#endif
    int opt = gbl->P_opt;
    int changed = FALSE;
    int done = FALSE;
    int init = TRUE;
    mode_t oldmode = sb->st_mode;

    (void) save_Xbase(gbl, gbl->cmdcol[CCOL_PROT]);

    ReplayStart('p');

    while (!done) {
	int rwx, cols[3];

	if (init) {
	    x = 0;
	    init = FALSE;
	    sb->st_mode = oldmode;
	}
	showLINE(gbl, gbl->curfile);

	rwx = (gbl->P_opt ? 1 : 3);
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
	case KEY_UP:
	    init = up_inline();
	    break;
	case KEY_DOWN:
	    init = down_inline();
	    break;
	case KEY_RIGHT:
	case '\f':
	case ' ':
	    if (x < 2)
		x++;
	    else
		beep();
	    break;
	case KEY_LEFT:
	case '\b':
	    if (x > 0)
		x--;
	    else
		beep();
	    break;
	default:
	    if (c >= '0' && c <= '7') {
		int shift = 6 - (x * 3);
		sb->st_mode &= (mode_t) (~(7 << shift));
		sb->st_mode |= (mode_t) (((c - '0') << shift));
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
#ifdef S_ISVTX
	    } else if (c == 't') {
		if (x == 2)
		    sb->st_mode ^= S_ISVTX;
		else
		    beep();
#endif
	    } else
		beep();
	}
    }
#ifdef	S_IFLNK
    if (at_flag) {		/* we had to toggle because of sym-link */
	(void) at_last(gbl, FALSE);
	/* force stat on the files, cleanup */
    }
#endif
    if (opt != gbl->P_opt) {
	gbl->P_opt = opt;
	showLINE(gbl, gbl->curfile);
    }
    restat(gbl, changed);
}

/*
 * Edit a text-field on the current display line.  Use the arrow keys for
 * moving within the line, and for setting/resetting insert mode.  Use
 * backspace to delete characters.
 */
int
edittext(RING * gbl, int endc, int col, int len, char *bfr)
{
    static DYN *result;
    int y = file2row(gbl->curfile);
    int code = TRUE;
    int limit = (int) strlen(bfr) + 2;
    char *s;

#ifdef	S_IFLNK
    int at_flag = (((endc == 'u') || (endc == 'g'))
		   ? at_save(gbl)
		   : FALSE);
#endif

    dlog_comment("before \"%s\"\n", bfr);
    if (len < limit)
	len = limit;
    col = save_Xbase(gbl, col);
#ifdef	S_IFLNK
    if (at_flag)
	showLINE(gbl, gbl->curfile);
#endif
    ReplayStart(endc);

    move(y, col);
    result = dyn_alloc(result, (size_t) len + 1);
    result = dyn_copy(result, bfr);
    if ((s = dlog_string(
			    gbl,
			    (char *) 0,
			    -1,
			    &result,
			    inline_text(),
			    inline_hist(),
			    endc,
			    len)) != NULL) {
	(void) strcpy(bfr, s);
	ReplayFinish();
    } else {
	ReplayQuit();
	code = FALSE;
    }

#ifdef	S_IFLNK
    if (at_flag) {		/* we had to toggle because of sym-link */
	(void) at_last(gbl, FALSE);
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
void
edit_uid(RING * gbl)
{
#if defined(HAVE_CHOWN)
    unsigned j;
    int uid = (int) cSTAT.st_uid;
    int changed = FALSE;
    char bfr[BUFSIZ];
    char *uid_s;

    if (gbl->G_opt == 1) {
	gbl->G_opt = 0;
	showFILES(gbl, FALSE);
    }

    uid_s = uid2s((uid_t) uid);
    if ((strlen(uid_s) < sizeof(bfr))
	&& EDITTEXT('u', CCOL_UID, UIDLEN, strcpy(bfr, uid_s))
	&& (uid = s2uid(bfr)) >= 0) {
	(void) dedsigs(TRUE);	/* reset interrupt-count */
	for_each_file(gbl, j) {
	    if ((int) gSTAT(j).st_uid == uid)
		continue;
	    if (dedsigs(TRUE)) {
		waitmsg(gNAME(j));
		break;
	    }
	    if (GROUPED(j)) {
		if (chown(gNAME(j), (uid_t) uid, gSTAT(j).st_gid) < 0) {
		    warn(gbl, gNAME(j));
		    break;
		}
		fixtime(gbl, j);
		gSTAT(j).st_uid = (uid_t) uid;
		changed++;
	    }
	}
    }
    restat(gbl, changed);
#else
    beep();
#endif
}

/*
 * Change file's group.
 */
void
edit_gid(RING * gbl)
{
#if defined(HAVE_CHOWN)
    unsigned j;
    int gid = (int) cSTAT.st_gid;
    int changed = FALSE;
    char bfr[BUFSIZ];
    char *gid_s;

    if (!gbl->G_opt) {
	gbl->G_opt = 1;
	showFILES(gbl, FALSE);
    }

    gid_s = gid2s((gid_t) gid);
    if ((strlen(gid_s) < sizeof(bfr))
	&& EDITTEXT('g', CCOL_GID, UIDLEN, strcpy(bfr, gid_s))
	&& (gid = s2gid(bfr)) >= 0) {

	(void) dedsigs(TRUE);	/* reset interrupt-count */
	for_each_file(gbl, j) {
	    if ((int) gSTAT(j).st_gid == gid)
		continue;
	    if (dedsigs(TRUE)) {
		waitmsg(gNAME(j));
		break;
	    }
	    if (GROUPED(j)) {
		if (chown(gNAME(j), gSTAT(j).st_uid, (gid_t) gid) < 0) {
		    warn(gbl, gNAME(j));
		    break;
		}
		gSTAT(j).st_gid = (gid_t) gid;
		fixtime(gbl, j);	/* some systems touch too */
		changed++;
	    }
	}
    }
    restat(gbl, changed);
#else
    beep();
#endif
}

/*
 * Change file's name
 */
void
editname(RING * gbl)
{
    int changed = 0;
    unsigned j;
    char bfr[MAXPATHLEN];

#define	EDITNAME(n)	EDITTEXT('=', CCOL_NAME, sizeof(bfr), strcpy(bfr, n))

    if (strlen(cNAME) < sizeof(bfr)
	&& EDITNAME(cNAME)
	&& strcmp(cNAME, bfr)) {
	if (dedname(gbl, (int) gbl->curfile, bfr) >= 0) {
	    (void) dedsigs(TRUE);	/* reset interrupt count */
	    hide_inline(TRUE);
	    for_each_file(gbl, j) {
		if (j == gbl->curfile)
		    continue;
		if (dedsigs(TRUE)) {
		    waitmsg(gNAME(j));
		    break;
		}
		if (gFLAG(j)
		    && (strlen(gNAME(j)) < sizeof(bfr))) {
		    (void) EDITNAME(gNAME(j));
		    if (dedname(gbl, (int) j, bfr) >= 0)
			changed++;
		    else
			break;
		}
	    }
	    hide_inline(FALSE);
	}
    }
    restat(gbl, changed);
}

#ifdef	S_IFLNK
/*
 * Change file's link-text
 */
void
editlink(RING * gbl, int cmd)
{
    int col, changed = 0;
    unsigned j;
    char bfr[MAXPATHLEN];

    cmd_link = (cmd == '<');

#define	EDITLINK(n) edittext(gbl, cmd, col, sizeof(bfr), link2bfr(gbl, bfr, n))

    if (!cLTXT)
	beep();
    else {
	int restore = FALSE;
	col = save_Xbase(gbl, gbl->cmdcol[CCOL_NAME]);

	/* test if we must show substitution */
	if (cmd_link) {
	    for_each_file(gbl, j) {
		if (j == gbl->curfile)
		    continue;
		if (gFLAG(j) && gLTXT(j)
		    && move2row(j, col)) {
		    int y0, y1, x;
		    getyx(stdscr, y0, x);
		    if (COLS - x > 3) {
			(void) standout();
			PRINTW("-> ");
			(void) standend();
			PRINTW("%.*s",
			       COLS - col - 3,
			       link2bfr(gbl, bfr, j));
			getyx(stdscr, y1, x);
			if (y1 == y0)
			    clrtoeol();
			restore = TRUE;
		    }
		}
	    }
	}

	(void) move2row(gbl->curfile, col);
	PRINTW("=> ");
	col += 3;
	if (EDITLINK(gbl->curfile)
	    && strcmp(cLTXT,
		      subslink(gbl, bfr, gbl->curfile))) {
	    if (relink(gbl, gbl->curfile, bfr)) {
		(void) dedsigs(TRUE);
		/* reset interrupt count */
		hide_inline(TRUE);
		for_each_file(gbl, j) {
		    if (j == gbl->curfile)
			continue;
		    if (dedsigs(TRUE)) {
			waitmsg(gNAME(j));
			break;
		    }
		    if (gFLAG(j) && gLTXT(j)) {
			(void) EDITLINK(j);
			if (relink(gbl, j, subslink(gbl, bfr, j)))
			    changed++;
			else
			    break;
		    }
		}
		hide_inline(FALSE);
	    }
	}
	if (restore && !changed)
	    showFILES(gbl, FALSE);
    }
    restat(gbl, changed);
}
#endif /* S_IFLNK */

/*
 * Edit the file's modification time.
 */
void
editdate(RING * gbl, unsigned current, int recur)
{
    static const char datemask[] = "ddd mmm DD HH:MM:SS YYYY";

    Stat_t *sb = &gSTAT(current);
    int y = file2row(current);
    int x = 0;
    int c;
    int changed = FALSE;
    int done = FALSE;
    int init = TRUE;
    time_t oldmtime = sb->st_mtime;
    int cols[sizeof(datemask)];
    int fields;

    (void) save_Xbase(gbl, gbl->cmdcol[CCOL_DATE]);

    ReplayStart('T');

    for (c = fields = 0; datemask[c] != 0; ++c) {
	if (c == 0 || (isalpha(UCH(datemask[c])) &&
		       datemask[c] != datemask[c - 1])) {
	    cols[fields++] = gbl->cmdcol[CCOL_DATE] + c;
	}
    }

    while (!done) {
	if (init) {
	    x = 0;
	    init = FALSE;
	    sb->st_mtime = oldmtime;
	}
	showLINE(gbl, current);

	move(y, cols[x]);
	switch (ReplayChar()) {
	case '\n':
	case 'T':
	    ReplayFinish();
	    ++changed;
	    fixtime(gbl, current);
	    if (!recur) {
		unsigned n;
		edit_inline(TRUE);
		for_each_file(gbl, n) {
		    if (n != (unsigned) current && GROUPED(n)) {
			editdate(gbl, n, TRUE);
		    }
		}
		edit_inline(FALSE);
	    }
	    done = TRUE;
	    break;
	case 'q':
	    ReplayQuit();
	    done = TRUE;
	    sb->st_mtime = oldmtime;
	    break;
	case TO_FIRST:
	    x = 0;
	    break;
	case TO_LAST:
	    x = fields - 1;
	    break;
	case KEY_UP:
	    init = up_inline();
	    break;
	case KEY_DOWN:
	    init = down_inline();
	    break;
	case KEY_RIGHT:
	case '\f':
	case ' ':
	    if (x < fields - 1)
		x++;
	    else
		beep();
	    break;
	case KEY_LEFT:
	case '\b':
	    if (x > 0)
		x--;
	    else
		beep();
	    break;
	case '+':
	    toggle_timestamp(&(sb->st_mtime), x, 1);
	    break;
	case '-':
	    toggle_timestamp(&(sb->st_mtime), x, -1);
	    break;
	default:
	    beep();
	    break;
	}
    }
    restat(gbl, changed);
}
