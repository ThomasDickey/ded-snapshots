#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)ded2s.c	1.8 88/05/23 06:57:48";
#endif	NO_SCCS_ID

/*
 * Title:	ded2s.c (ded-stat to string)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		23 May 1988, absorbed 'z_rels' into 'z_vers'.
 *		18 May 1988, show Apollo inodes in hex.
 *		09 May 1988, sockets do not have major/minor numbers.
 *
 * Function:	Convert ded's FLIST structure to printing form (controlled by
 *		options).  In doing so, save into the global 'cmdcol[]' the
 *		index of various fields to which we may wish to move the
 *		cursor.
 *
 * patch:	should consider showing both user+group columns.
 */

#include	"ded.h"
#include	<ctype.h>
#ifdef	SYSTEM5
#include	<sys/sysmacros.h>
#endif	SYSTEM5
extern  time_t  time();
extern  char	*ctime();

extern	char	*uid2s(),
		*gid2s();

#define SIXDAYS		(6 * 24 * HOUR)
#define SIXMONTHS	(30 * SIXDAYS)

ded2s(inx, bfr, len)
register char	*bfr;
{
FLIST		*f_	= &flist[inx];
struct	stat	*s	= &(f_->s);
time_t  fdate;
register unsigned mj;
register int	c;
char	*t,
	*name = f_->name,
	*base = bfr;

	/* Translate the filemode (type+protection) */
	mj = s->st_mode;
	if (P_opt) {
		FORMAT(bfr, "%6o ", mj);
		cmdcol[0] = 3;
	} else {
		*bfr++ = modechar(mj); /* translate the type of file */
		cmdcol[0] = bfr - base;

		(void)strcpy(bfr, "--------- ");
		for (c = 0; c < 9; c += 3) {
			if (mj & (S_IREAD  >> c))	bfr[c]   = 'r';
			if (mj & (S_IWRITE >> c))	bfr[c+1] = 'w';
			if (mj & (S_IEXEC  >> c))	bfr[c+2] = 'x';
		}
		if (mj & S_ISUID)			bfr[2]   = 's';
		if (mj & S_ISGID)			bfr[5]   = 's';
		if (mj & S_ISVTX)			bfr[8]   = 't';
	}
	bfr += strlen(bfr);

	/* translate the number of links, or the inode value */
#ifdef	apollo
	if (I_opt)	FORMAT(bfr, "%08x ", s->st_ino);
#else	apollo
	if (I_opt)	FORMAT(bfr, "%5d ", s->st_ino);
#endif	apollo
	else		FORMAT(bfr, "%3d ", s->st_nlink);
	bfr += field(bfr,mj);

	/* show the user-id or group-id */
	if (G_opt)	t = gid2s(s->st_gid);
	else		t = uid2s(s->st_uid);
	FORMAT(bfr, "%-*.*s ", UIDLEN, UIDLEN, t);
	cmdcol[1] = bfr - base;
	bfr += field(bfr,mj);

	/* show the file-size (or major/minor device codes, if device) */
	switch (mj & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
		if (S_opt)
			bfr += strlen(strcpy(bfr, "      "));
		FORMAT(bfr, "%3d,%3d ", major(s->st_rdev), minor(s->st_rdev));
		break;
	default:
		if (S_opt) {
			FORMAT(bfr, "%5d ",
#ifdef	SYSTEM5
				s->st_size / 1024	/* patch */
#else
				s->st_blocks
#endif	SYSTEM5
				);
			bfr += field(bfr,mj);
		}
		FORMAT(bfr, "%7d ", s->st_size);
	}
	bfr += field(bfr,mj);

	/* show sccs-date, if any */
#ifdef	Z_SCCS
	if (Z_opt > 0) {
		time2s(bfr, f_->z_time);
		bfr += field(bfr,mj);
	}
	if (Z_opt != 0) {	/* show relationship between dates */
		if (mj != 0 && f_->z_time) {
		long	diff = s->st_mtime - f_->z_time;
			*bfr++ = ((diff > 0) ? '<' : ((diff < 0) ? '>' : '='));
		} else
			*bfr++ = ' ';
		*bfr++ = ' ';
	}
#endif	Z_SCCS

	/* show the appropriate-date */
	fdate =	(dateopt == 1)  ? s->st_ctime
				: (dateopt == 0 ? s->st_atime
						: s->st_mtime);
	time2s(bfr,fdate);
	bfr += field(bfr,mj);

#ifdef	Z_SCCS
	if (Z_opt && V_opt) {
		FORMAT(bfr, "%-7s ", f_->z_vers);
		bfr += field(bfr, (unsigned)(f_->z_time != 0));
	}
#endif	Z_SCCS

	cmdcol[2] = bfr - base;
	*bfr++ = ' ';
	*bfr++ = ' ';

	/* translate the filename */
	cmdcol[3] = bfr - base;
	len -= (bfr-base);
	bfr += name2s(bfr, name, len, FALSE);

	if (isDIR(mj)) {
		*bfr++ = '/';
	} else if (isLINK(mj) && ((t = f_->ltxt) != 0)) {
		*bfr++ = ' ';
		*bfr++ = '-';
		*bfr++ = '>';
		*bfr++ = ' ';
		len -= (bfr-base);
		bfr += name2s(bfr, t, len, FALSE);
	} else if (executable(s))	*bfr++ = '*';
	*bfr = '\0';
}

/************************************************************************
 *	local procedures						*
 ***********************************************************************/
static
executable(s)
struct	stat *s;
{
int	uid = getuid();
int	gid = getgid();

	if (!uid) {		/* root can do anything */
		uid = s->st_uid;
		gid = s->st_gid;
	}
	if (uid == s->st_uid) {
		if (s->st_mode & S_IEXEC)		return(TRUE);
	}
	if (gid == s->st_gid) {
		if (s->st_mode & (S_IEXEC >> 3))	return(TRUE);
	}
	return (s->st_mode & (S_IEXEC >> 6));
}

/*
 * Provide skip-over-field (blanking it if the file has been deleted).
 */
static
field(bfr, mode)
char	*bfr;
unsigned mode;
{
register char *s = bfr;
register int len = strlen(s);
	if (mode == 0)
		while (*s)	*s++ = ' ';
	return (len);
}

/*
 * Convert a unix time to an appropriate printing-string
 */
static
time2s(bfr,fdate)
char	*bfr;
time_t  fdate;
{
time_t  now	= time((time_t *)0);
char	*t	= ctime(&fdate);	/* 0123456789.123456789.123 */
	t[24]	= 0;			/* ddd mmm DD HH:MM:SS YYYY */

	if ((now - SIXDAYS) < fdate) {	  /* ddd HH:MM:SS */
		FORMAT(bfr, "%.4s%.8s ", t, t+11);
	} else if ((now - SIXMONTHS) < fdate) { /* mmm DD HH:MM */
		FORMAT(bfr, "%.12s ", t+4);
	} else {				/* mmm DD YYYY  */
		FORMAT(bfr, "%.7s%.4s  ", t+4, t+20);
	}
	if (fdate == 0L)
		(void)field(bfr,0);
}

/*
 * Convert a filename-string to printing form (for display)
 */
name2s(bfr, name, len, esc)
char	*bfr, *name;
int	len;
int	esc;		/* true if we escape dollar-signs, etc. */
{
char	*base = bfr;
register int c;

	while ((c = *name++) && len-- > 0) {
#ifdef	apollo
		if (U_opt) {	/* show underlying apollo filenames */
			if (isascii(c) && isgraph(c)) {
				if (isalpha(c) && isupper(c)) {
					*bfr++ = ':';
					c = _tolower(c);
				} else if ((c == ':')
				||	   (c == '.' && bfr == base))
					*bfr++ = ':';
				*bfr++ = c;
			} else if (c == ' ') {
				*bfr++ = ':';
				*bfr++ = '_';
			} else {
				FORMAT(bfr, "%s#%02x", esc ? "\\" : "", c);
				bfr += strlen(bfr);
			}
		} else
#endif	apollo
		if (esc) {
			if(iscntrl(c)
			|| isspace(c)
			|| (c == '$')
			|| (c == '\\')
			|| (c == '>')
			|| (c == '&')
			|| (c == '#'))
				*bfr++ = '\\';	/* escape the nasty thing */
			*bfr++ = c;
		} else {
			if (isascii(c) && isgraph(c)) {
				*bfr++ = c;
			} else
				*bfr++ = '?';
		}
	}
	*bfr = '\0';
	return (bfr-base);
}
