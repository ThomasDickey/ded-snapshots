/*
 * Title:	ded2s.c (ded-stat to string)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		15 Feb 1998, remove special code for apollo sr10.
 *			     Correct a missing 'else' in time2s that caused
 *			     future dates to be formatted as in the past.
 *		05 Oct 1994, refined executable-access test with getgroups.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		23 Nov 1992, for RCS version 5, show differences in ztime/mtime
 *			     that are probably due to localtime/gmt_offset diff
 *			     as "~".
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		04 Feb 1992, show differences between mtime & ztime specially
 *			     when +/- 1.
 *		18 Oct 1991, converted to ANSI
 *		16 Aug 1991, added interpretation of "2T"
 *		16 Jul 1991, inode-value and number-of-blocks are unsigned
 *		02 Jul 1991, make S_opt, P_opt 3-way toggles.
 *		28 Jun 1991, corrected code which tests for executable-access
 *			     (must look at effective-id, not real-id).
 *		24 Apr 1990, corrected 'time2s()' to handle dates past coming
 *			     midnight
 *		30 Jan 1990, if 'T_opt' is set, display all date+time fields
 *			     in long form, as returned by 'ctime()'
 *		13 Oct 1989, gave up on function-prototype for type_$get_name(),
 *			     since ref-variables only work for input-args.
 *			     Corrected pointer-bug in z_lock/z_vers display
 *		12 Oct 1989, refined the date-conversion in 'time2s()'.  Added
 *			     procedure 'has_extended_acl()'.  Show z_vers,
 *			     z_lock fields even if z_time is null, since we
 *			     may have gotten to the view via a symbolic link.
 *			     recoded 'type_uid2s()' using 'type_$get_name()'
 *		11 Oct 1989, recoded type-uid table with a pipe-call to 'lty'
 *		06 Oct 1989, modified computation of 'cmdcol[]' so that it is
 *			     not reset per-line, but accumulated in a file-list.
 *			     Added column after size-field, since sr10.1 has
 *			     some long dev-ids!
 *		05 Oct 1989, corrected treatment of nil-objects.  Don't show
 *			     deleted-files as having extended acls.
 *		04 Oct 1989, added code to support 'O' toggle (show apollo
 *			     object-types).  Added code for apollo SR10.1 which
 *			     shows a "+" after mode like the 'ls' utility on
 *			     that system.
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

MODULE_ID("$Id: ded2s.c,v 12.21 1998/02/16 01:51:28 tom Exp $")

#if MAJOR_IN_MKDEV
#  include	<sys/mkdev.h>
#else
#  if MAJOR_IN_SYSMACROS
#    include	<sys/sysmacros.h>
#  endif
#endif

#define ONE_WEEK	(7 * 24 * HOUR)
#define SIXMONTHS	(26 * ONE_WEEK)

#define	OK_S(s)		(s != 0 && s[0] != '?')

#define	SETCOL(p,col)	p = setcol(p, &(gbl->cmdcol[col]), p - base)

/************************************************************************
 *	local procedures						*
 ***********************************************************************/
/*
 * Provide skip-over-field (blanking it if the file has been deleted).
 */
private	int	field(
	_ARX(char *,	bfr)
	_AR1(unsigned,	mode)
		)
	_DCL(char *,	bfr)
	_DCL(unsigned,	mode)
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
private	void	time2s(
	_ARX(char *,	bfr)
	_ARX(time_t,	fdate)
	_AR1(int,	option)
		)
	_DCL(char *,	bfr)
	_DCL(time_t,	fdate)
	_DCL(int,	option)
{
	static	time_t	midnite;
	auto	time_t  now	= time((time_t *)0);
	auto	char	*t	= ctime(&fdate);

						/* 0123456789.123456789.123 */
	t[24]	= ' ';				/* ddd mmm DD HH:MM:SS YYYY */

	if (option == 2) {
		FORMAT(bfr, "%12.6f ", (now - fdate) / (24.0 * HOUR));
	} else if (option == 1) {
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
		} else if ((midnite - ONE_WEEK) <= fdate) {  /* ddd HH:MM:SS */
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

private	char *	setcol(
	_ARX(char *,	bfr)
	_ARX(int *,	col)
	_AR1(int,	val)
		)
	_DCL(char *,	bfr)
	_DCL(int *,	col)
	_DCL(int,	val)
{
	if (*col < val)	*col = val;
	else {
		while (val++ < *col)
			*bfr++ = ' ';
	}
	return (bfr);
}

/************************************************************************
 *	public procedures						*
 ***********************************************************************/
public	void	ded2s(
	_ARX(RING *,	gbl)
	_ARX(int,	inx)
	_ARX(char *,	bfr)
	_AR1(int,	len)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	inx)
	_DCL(char *,	bfr)
	_DCL(int,	len)
{
	FLIST	*f_	= &gENTRY(inx);
	Stat_t	*s	= &(f_->s);
	time_t  fdate;
	register unsigned mj;
	register int	c;
	char	*t,
		*name = f_->name,
		*base = bfr;

	/* Translate the filemode (type+protection) */
	mj = s->st_mode;
	if (gbl->P_opt) {
		FORMAT(bfr, "%6o ", mj);
		gbl->cmdcol[CCOL_PROT] = 3;
	} else {
		*bfr++ = modechar(mj); /* translate the type of file */
		gbl->cmdcol[CCOL_PROT] = bfr - base;

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
	if (gbl->AT_opt && f_->ltxt) {
		for (t = base; *t; t++)
			UpperCase(*t);
	}
#endif
	bfr += strlen(bfr);
	*bfr++ = ' ';

	/* translate the number of links, or the inode value */
#ifdef	apollo
	if (gbl->I_opt)	FORMAT(bfr, "%08lx ", (unsigned long)s->st_ino);
#else	/* unix */
	if (gbl->I_opt)	FORMAT(bfr, "%5lu ", (unsigned long)s->st_ino);
#endif	/* apollo/unix */
	else		FORMAT(bfr, "%3ld ", (long)s->st_nlink);
	bfr += field(bfr,mj);
	if (gbl->I_opt == 2)	{
		FORMAT(bfr, "%08lx ", (long)s->st_dev);
		bfr += field(bfr,mj);
	}

	SETCOL(bfr, CCOL_UID);
	if (!(gbl->G_opt & 1)) {	/* show the user-id */
		if (gbl->P_opt > 1)
			FORMAT(bfr, "%-*d ", UIDLEN, (int)(s->st_uid));
		else
			FORMAT(bfr, "%-*.*s ",
				UIDLEN, UIDLEN, uid2s((int)(s->st_uid)));
		bfr += field(bfr,mj);
	}

	if (gbl->G_opt != 0) {	/* show the group-id */
		SETCOL(bfr, CCOL_GID);
		if (gbl->P_opt > 1)
			FORMAT(bfr, "%-*d ", UIDLEN, (int)(s->st_gid));
		else
			FORMAT(bfr, "%-*.*s ",
				UIDLEN, UIDLEN, gid2s((int)(s->st_gid)));
		bfr += field(bfr,mj);
	} else
		gbl->cmdcol[CCOL_GID] = gbl->cmdcol[CCOL_UID];

	/* show the file-size (or major/minor device codes, if device) */
	switch (mj & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
		if (gbl->S_opt >= 1)
			bfr += strlen(strcpy(bfr, "      "));
		if (gbl->S_opt != 1) {
			FORMAT(bfr, "%3ld,%3ld ",
				(long)major(s->st_rdev),
				(long)minor(s->st_rdev));
			bfr += field(bfr,mj);
		}
		break;
	default:
		if (gbl->S_opt >= 1) {
			FORMAT(bfr, "%5lu ", ded_blocks(s));
			bfr += field(bfr,mj);
		}
		if (gbl->S_opt != 1) {
			FORMAT(bfr, "%7ld ", (unsigned long)s->st_size);
			bfr += field(bfr,mj);
		}
	}
	SETCOL(bfr, CCOL_DATE);

	/* show sccs-date, if any */
#ifdef	Z_RCS_SCCS
	if (gbl->Z_opt > 0) {
		time2s(bfr, f_->z_time, gbl->T_opt);
		bfr += field(bfr,mj);
	}
	if (gbl->Z_opt != 0) {	/* show relationship between dates */
		if (mj != 0 && f_->z_time) {
			long	diff = s->st_mtime - f_->z_time;
			int	mark = '=';

			if (diff < 0)
				mark = (diff == -1) ? '+' : '>';
			else if (diff > 0)
				mark = (diff ==  1) ? '-' : '<';
#if	RCS_VERSION >= 5
			if (diff == -gmt_offset(s->st_mtime))
				mark = '~';
#else	/* RCS_VERSION <= 4 */
			if (diff == gmt_offset(s->st_mtime))
				mark = '~';
#endif

			*bfr++ = mark;
		} else
			*bfr++ = ' ';
		*bfr++ = ' ';
	}
#endif	/* Z_RCS_SCCS */

	/* show the appropriate-date */
	fdate =	(gbl->dateopt == 1)  ? s->st_ctime
				: (gbl->dateopt == 0 ? s->st_atime
						: s->st_mtime);
	time2s(bfr,fdate, gbl->T_opt);
	bfr += field(bfr,mj);

#ifdef	Z_RCS_SCCS
	if (gbl->Z_opt) {
		if (gbl->V_opt) {	/* show highest version number */
			if (!(t = f_->z_vers))	t = "";
			FORMAT(bfr, "%-7s ", t);
			bfr += field(bfr, (unsigned)(OK_S(t)));
		}
		if (gbl->Y_opt) {	/* show current lock */
			if (!(t = f_->z_lock))	t = "";
			FORMAT(bfr, "%-*.*s ", UIDLEN, UIDLEN, t);
			bfr += field(bfr, (unsigned)(OK_S(t)));
		}
	}
#endif	/* Z_RCS_SCCS */

	SETCOL(bfr, CCOL_CMD);
	*bfr++ = ' ';
	*bfr++ = ' ';

	/* translate the filename */
	SETCOL(bfr, CCOL_NAME);
	len -= (bfr-base);
	f_->namlen = ded2string(gbl, bfr, len, name, FALSE);
	bfr += f_->namlen;

#ifdef	S_IFLNK
	if ((t = f_->ltxt) != 0) {
		*bfr++ = ' ';
		*bfr++ = '-';
		*bfr++ = '>';
		*bfr++ = ' ';
		len -= (bfr-base);
		bfr += ded2string(gbl, bfr, len, t, FALSE);
	} else
#endif	/* S_IFLNK */
		if (isDIR(mj)) {
		*bfr++ = '/';
	} else if (ded_access(s, S_IEXEC))
		*bfr++ = '*';
	*bfr = '\0';
}

public	int	ded2string(
	_ARX(RING *,	gbl)
	_ARX(char *,	bfr)
	_ARX(int,	len)
	_ARX(char *,	name)
	_AR1(int,	flag)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	bfr)
	_DCL(int,	len)
	_DCL(char *,	name)
	_DCL(int,	flag)
{
	return (name2s(bfr, len, name, flag | (gbl->U_opt ? 2 : 0)));
}

public	int	ded_access (
	_ARX(Stat_t *,	s)
	_AR1(int,	mask)	/* one of S_IEXEC, S_IREAD, S_IWRITE */
		)
	_DCL(Stat_t *,	s)
	_DCL(int,	mask)
{
	int	uid = geteuid();
	int	gid = getegid();

	if (in_group(s->st_gid))
		gid = s->st_gid;

	if (!uid) {		/* root can do anything */
		if (mask != S_IEXEC)
			return TRUE;
		return (s->st_mode & (mask | (mask >> 3) | (mask >> 6)));
	} else if (uid == s->st_uid) {
		return (s->st_mode & mask);
	} else if (gid == s->st_gid) {
		return (s->st_mode & (mask >> 3));
	}
	return (s->st_mode & (mask >> 6));
}
