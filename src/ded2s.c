#ifndef	lint
static	char	Id[] = "$Id: ded2s.c,v 4.2 1989/10/04 17:01:06 dickey Exp $";
#endif	lint

/*
 * Title:	ded2s.c (ded-stat to string)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * $Log: ded2s.c,v $
 * Revision 4.2  1989/10/04 17:01:06  dickey
 * added code to support 'O' toggle (show object-types)
 *
 *		Revision 4.1  89/10/04  10:25:08  dickey
 *		added code for apollo SR10.1 which shows a "+" after mode
 *		like the 'ls' utility on that system.
 *		
 *		Revision 4.0  88/08/18  14:55:51  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  88/08/18  14:55:51  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.0  88/08/18  14:55:51  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.15  88/08/18  14:55:51  dickey
 *		sccs2rcs keywords
 *		
 *		12 Aug 1988, apollo sys5 environment permits symbolic links.
 *		16 Jun 1988, added uppercase-code for AT_opt.
 *		01 Jun 1988, added 'Y_opt' field.
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

#ifdef	apollo_sr10
#include	"acl.h"
#include	<apollo/base.h>
#include	<apollo/type_uids.h>
char	*type_uid2s();
#endif

#ifdef	SYSTEM5
#include	<sys/sysmacros.h>
#endif	SYSTEM5
extern  time_t  time();
extern  char	*ctime();

extern	char	*uid2s(),
		*gid2s();
#ifndef	_toupper
#define	_toupper	toupper
#endif	_toupper

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

		(void)strcpy(bfr, "---------");
		for (c = 0; c < 9; c += 3) {
			if (mj & (S_IREAD  >> c))	bfr[c]   = 'r';
			if (mj & (S_IWRITE >> c))	bfr[c+1] = 'w';
			if (mj & (S_IEXEC  >> c))	bfr[c+2] = 'x';
		}
		if (mj & S_ISUID)			bfr[2]   = 's';
		if (mj & S_ISGID)			bfr[5]   = 's';
		if (mj & S_ISVTX)			bfr[8]   = 't';
	}

#ifdef	S_IFLNK
	/* show symbolic link target mode in uppercase */
	if (AT_opt && f_->ltxt) {
		for (t = base; *t; t++)
			if (isalpha(*t))	*t = _toupper(*t);
	}
#endif	S_IFLNK
	bfr += strlen(bfr);
#ifdef	apollo_sr10
	*bfr++ = is_EXTENDED_ACL(s->st_rfu4) ? '+' : ' ';
	if (O_opt) {
		FORMAT(bfr, " %-9.9s ", type_uid2s(s));
		bfr += field(bfr,mj);
	}
#endif
	*bfr++ = ' ';

	/* translate the number of links, or the inode value */
#ifdef	apollo
	if (I_opt)	FORMAT(bfr, "%08x ", s->st_ino);
#else	unix
	if (I_opt)	FORMAT(bfr, "%5d ", s->st_ino);
#endif	apollo/unix
	else		FORMAT(bfr, "%3d ", s->st_nlink);
	bfr += field(bfr,mj);

	/* show the user-id or group-id */
	if (G_opt)	t = gid2s((int)(s->st_gid));
	else		t = uid2s((int)(s->st_uid));
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
#ifdef	Z_RCS_SCCS
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
#endif	Z_RCS_SCCS

	/* show the appropriate-date */
	fdate =	(dateopt == 1)  ? s->st_ctime
				: (dateopt == 0 ? s->st_atime
						: s->st_mtime);
	time2s(bfr,fdate);
	bfr += field(bfr,mj);

#ifdef	Z_RCS_SCCS
	if (Z_opt) {
		if (V_opt) {	/* show highest version number */
			FORMAT(bfr, "%-7s ", f_->z_vers);
			bfr += field(bfr, (unsigned)(f_->z_time != 0));
		}
		if (Y_opt) {	/* show current lock */
			FORMAT(bfr, "%-*.*s ", UIDLEN, UIDLEN, f_->z_lock);
			bfr += field(bfr, (unsigned)(f_->z_time != 0));
		}
	}
#endif	Z_RCS_SCCS

	cmdcol[2] = bfr - base;
	*bfr++ = ' ';
	*bfr++ = ' ';

	/* translate the filename */
	cmdcol[3] = bfr - base;
	len -= (bfr-base);
	bfr += ded2string(bfr, len, name, FALSE);

#ifdef	S_IFLNK
	if ((t = f_->ltxt) != 0) {
		*bfr++ = ' ';
		*bfr++ = '-';
		*bfr++ = '>';
		*bfr++ = ' ';
		len -= (bfr-base);
		bfr += ded2string(bfr, len, t, FALSE);
	} else
#endif	S_IFLNK
		if (isDIR(mj)) {
		*bfr++ = '/';
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

ded2string(bfr, len, name, flag)
char	*bfr, *name;
{
	return (name2s(bfr, len, name, flag | (U_opt ? 2 : 0)));
}

#ifdef	apollo_sr10
/*
 * patch: the trait/type manager of sr10 is not documented (yet).  return the
 * known/fixed types
 */
char	*
type_uid2s(s)
struct	stat *s;
{
	register int	c;
	register char	*t;
	static	struct	{
		uid_$t	*id;
		char	*name;
	} list[] = {
		&case_hm_$uid,		"case_hm",
		&cmpexe_$uid,		"cmpexe",
		&coff_$uid,		"coff",
		&d3m_area_$uid,		"d3m_area",
		&d3m_sch_$uid,		"d3m_sch",
		&directory_$uid,	"directory",
		&dm_edit_$uid,		"dm_edit",
		&hdr_undef_$uid,	"hdr_undef",
		&input_pad_$uid,	"ipad",
		&mbx_$uid,		"mbx",
		&mt_$uid,		"mt",
		&nulldev_$uid,		"null",
		&object_file_$uid,	"obj",
		&pad_$uid,		"pad",
		&pty_$slave_uid,	"pty_slave",
		&pty_$uid,		"pty",
		&records_$uid,		"records",
		&sio_$uid,		"sio",
		&tcp_$uid,		"tcp",
		&uasc_$uid,		"uasc",
		&unstruct_$uid,		"unstruct"
	};
	t = " ";
	if (isDIR(s->st_mode))
		t = "nil";
	else if (isFILE(s->st_mode) || isDEV(s->st_mode)) {
		t = "?";
		for (c = 0; c < sizeof(list)/sizeof(list[0]); c++) {
			if (list[c].id->high == s->st_rfu4[0]
			&&  list[c].id->low  == s->st_rfu4[1]) {
				t = list[c].name;
				break;
			}
		}
	}
	return (t);
}
#endif
