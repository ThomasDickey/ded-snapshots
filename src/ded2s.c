#ifndef	lint
static	char	Id[] = "$Id: ded2s.c,v 8.0 1990/04/24 16:27:56 ste_cm Rel $";
#endif	lint

/*
 * Title:	ded2s.c (ded-stat to string)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * $Log: ded2s.c,v $
 * Revision 8.0  1990/04/24 16:27:56  ste_cm
 * BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *
 *		Revision 7.0  90/04/24  16:27:56  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.1  90/04/24  16:27:56  dickey
 *		corrected 'time2s()' to handle dates past coming midnight
 *		
 *		Revision 6.0  90/01/30  08:38:15  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.1  90/01/30  08:38:15  dickey
 *		if 'T_opt' is set, display all date+time fields in long form,
 *		as returned by 'ctime()'
 *		
 *		Revision 5.0  89/10/13  13:39:40  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.11  89/10/13  13:39:40  dickey
 *		gave up on function-prototype for type_$get_name(), since
 *		ref-variables only work for input-args.
 *		
 *		Revision 4.10  89/10/13  09:33:08  dickey
 *		corrected pointer-bug in z_lock/z_vers display
 *		
 *		Revision 4.9  89/10/12  15:53:54  dickey
 *		refined the date-conversion in 'time2s()'.  added procedure
 *		'has_extended_acl()'
 *		
 *		Revision 4.8  89/10/12  09:15:15  dickey
 *		show z_vers, z_lock fields even if z_time is null, since we
 *		may have gotten to the view via a symbolic link.
 *		
 *		Revision 4.7  89/10/12  08:58:57  dickey
 *		recoded 'type_uid2s()' using 'type_$get_name()'
 *		
 *		Revision 4.6  89/10/11  16:47:27  dickey
 *		recoded type-uid table with a pipe-call to 'lty'
 *		
 *		Revision 4.5  89/10/06  08:08:58  dickey
 *		modified computation of 'cmdcol[]' so that it is not reset
 *		per-line, but accumulated in a file-list.  added column
 *		after size-field, since sr10.1 has some long dev-ids!
 *		
 *		Revision 4.4  89/10/05  14:32:44  dickey
 *		corrected treatment of nil-objects
 *		
 *		Revision 4.3  89/10/05  07:57:37  dickey
 *		don't show deleted-files as having extended acls
 *		
 *		Revision 4.2  89/10/04  17:01:06  dickey
 *		added code to support 'O' toggle (show object-types)
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
 */

#include	"ded.h"
#include	<time.h>
#include	<ctype.h>

#ifdef	apollo_sr10
#include	"acl.h"
#include	<apollo/base.h>
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

#define ONE_WEEK	(7 * 24 * HOUR)
#define SIXMONTHS	(26 * ONE_WEEK)

#define	OK_S(s)		(s != 0 && s[0] != '?')

static
char	*
setcol(bfr,n,val)
char	*bfr;
{
	if (cmdcol[n] < val)	cmdcol[n] = val;
	else {
		while (val++ < cmdcol[n])
			*bfr++ = ' ';
	}
	return (bfr);
}

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
		cmdcol[CCOL_PROT] = 3;
	} else {
		*bfr++ = modechar(mj); /* translate the type of file */
		cmdcol[CCOL_PROT] = bfr - base;

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
	*bfr++ = ((mj != 0) && has_extended_acl(inx)) ? '+' : ' ';
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
	if (I_opt == 2)	{
		FORMAT(bfr, "%08x ", s->st_dev);
		bfr += field(bfr,mj);
	}

	bfr = setcol(bfr, CCOL_UID, bfr - base);
	if (!(G_opt & 1)) {	/* show the user-id */
		FORMAT(bfr, "%-*.*s ", UIDLEN, UIDLEN, uid2s((int)(s->st_uid)));
		bfr += field(bfr,mj);
	}

	if (G_opt != 0) {	/* show the group-id */
		bfr = setcol(bfr, CCOL_GID, bfr - base);
		FORMAT(bfr, "%-*.*s ", UIDLEN, UIDLEN, gid2s((int)(s->st_gid)));
		bfr += field(bfr,mj);
	} else
		cmdcol[CCOL_GID] = cmdcol[CCOL_UID];

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
	bfr = setcol(bfr, CCOL_DATE, bfr - base);

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
			if (!(t = f_->z_vers))	t = "";
			FORMAT(bfr, "%-7s ", t);
			bfr += field(bfr, (unsigned)(OK_S(t)));
		}
		if (Y_opt) {	/* show current lock */
			if (!(t = f_->z_lock))	t = "";
			FORMAT(bfr, "%-*.*s ", UIDLEN, UIDLEN, t);
			bfr += field(bfr, (unsigned)(OK_S(t)));
		}
	}
#endif	Z_RCS_SCCS

	bfr = setcol(bfr, CCOL_CMD, bfr - base);
	*bfr++ = ' ';
	*bfr++ = ' ';

	/* translate the filename */
	bfr = setcol(bfr, CCOL_NAME, bfr - base);
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
	static	time_t	midnite;
	auto	time_t  now	= time((time_t *)0);
	auto	char	*t	= ctime(&fdate);

						/* 0123456789.123456789.123 */
	t[24]	= ' ';				/* ddd mmm DD HH:MM:SS YYYY */

	if (T_opt) {
		(void)strcpy(bfr, t);
	} else {
		if (midnite == 0) {
			struct	tm	*p = localtime(&now);
			midnite	= now	/* compute next 00:00:00 time */
				+ (23 - p->tm_hour) * HOUR
				+ (59 - p->tm_min) * 60
				+ (60 - p->tm_sec);
		}
		if (now >= midnite)	/* bump if we ran past midnite */
			midnite += (24 * HOUR);

		if (midnite <= fdate) {			     /* future? */
			FORMAT(bfr, "%.7s%.4s  ", t+4, t+20);
		} if ((midnite - ONE_WEEK) <= fdate) {	     /* ddd HH:MM:SS */
			FORMAT(bfr, "%.4s%.8s ", t, t+11);
		} else if ((midnite - SIXMONTHS) < fdate) {  /* mmm DD HH:MM */
			FORMAT(bfr, "%.12s ", t+4);
		} else {				     /* mmm DD YYYY  */
			FORMAT(bfr, "%.7s%.4s  ", t+4, t+20);
		}
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
std_$call	type_$get_name();

typedef	struct	_lty {
	struct	_lty	*link;
	uid_$t		uid;
	char		*name;
	} LTY;

	/*ARGSUSED*/
	def_ALLOC(LTY)

/*
 * return the string corresponding to apollo type-uid stored in the stat-block.
 */
char	*
type_uid2s(s)
struct	stat *s;
{
	extern	char	*txtalloc();
	static	LTY	*list;
	register LTY	*p;
	auto	char	*t;
	auto	uid_$t	type_uid;
	auto	name_$name_t	typename;
	auto	short		namelen;
	auto	status_$t	status;

	if (s->st_mode != 0) {
		type_uid.high = s->st_rfu4[0];
		type_uid.low  = s->st_rfu4[1];
		t = 0;
		for (p = list; p != 0; p = p->link) {
			if (p->uid == type_uid) {
				t = p->name;
				break;
			}
		}
		if (t == 0) {
			type_$get_name(uid_$nil, type_uid,
				typename, namelen, status);  
			if (status.all != status_$ok)
				namelen = 0;
      			typename[namelen] = EOS;
			p = ALLOC(LTY,1);
			p->link = list;
			p->uid  = type_uid;
			p->name = t = txtalloc(typename);
			list    = p;
		}
	} else
		t = " ";
	return (t);
}

has_extended_acl(x)
{
	return (is_EXTENDED_ACL(xSTAT(x).st_rfu4));
}
#endif
