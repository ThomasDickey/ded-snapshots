#include	"ded.h"
#include	<ctype.h>
#ifdef	SYSTEM5
#include	<sys/sysmacros.h>
#endif	SYSTEM5
extern  time_t  time();
extern  char	*ctime();

extern	char	*uid2s(),
		*gid2s();

#define SIXDAYS	 (6 * 24 * 3600L)
#define SIXMONTHS	(30 * SIXDAYS)

ded2s(inx, bfr, len)
register char	*bfr;
{
struct	stat	*s = &flist[inx].s;
time_t  fdate, now = time((time_t *)0);
register int	mj, c;
char	*t,
	*name = flist[inx].name,
	*base = bfr;

	/* Translate the filemode (type+protection) */
	mj = s->st_mode;
	if (P_opt) {
		sprintf(bfr, "%6o ", mj);
		cmdcol[0] = 3;
	} else {
		*bfr++ = modechar(mj); /* translate the type of file */
		cmdcol[0] = bfr - base;

		strcpy(bfr, "--------- ");
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
	if (I_opt)	sprintf(bfr, "%5d ", s->st_ino);
	else		sprintf(bfr, "%3d ", s->st_nlink);
	bfr += field(bfr,mj);

	/* show the user-id or group-id */
	if (G_opt)	t = gid2s(s->st_gid);
	else		t = uid2s(s->st_uid);
	sprintf(bfr, "%-*.*s ", UIDLEN, UIDLEN, t);
	cmdcol[1] = bfr - base;
	bfr += field(bfr,mj);

	/* show the file-size (or major/minor device codes, if device) */
	switch (mj & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
#ifdef	S_IFSOCK
	case S_IFSOCK:
#endif	S_IFSOCK
		if (S_opt)
			bfr += strlen(strcpy(bfr, "      "));
		sprintf(bfr, "%3d,%3d ", major(s->st_rdev), minor(s->st_rdev));
		break;
	default:
		if (S_opt) {
			sprintf(bfr, "%5d ",
#ifdef	SYSTEM5
				s->st_size / 1024	/* patch */
#else
				s->st_blocks
#endif	SYSTEM5
				);
			bfr += field(bfr,mj);
		}
		sprintf(bfr, "%7d ", s->st_size);
	}
	bfr += field(bfr,mj);

	/* show the appropriate-date */
	fdate =	(dateopt == 1)  ? s->st_ctime
				: (dateopt == 0 ? s->st_atime
						: s->st_mtime);
	t = ctime(&fdate);			/* 0123456789.123456789.123 */
	t[24] = 0;				/* ddd mmm DD HH:MM:SS YYYY */

	if ((now - SIXDAYS) < fdate) {	  /* ddd HH:MM:SS */
		sprintf(bfr, "%.4s%.8s", t, t+11);
	} else if ((now - SIXMONTHS) < fdate) { /* mmm DD HH:MM */
		sprintf(bfr, "%.12s", t+4);
	} else {				/* mmm DD YYYY  */
		sprintf(bfr, "%.7s%.4s ", t+4, t+20);
	}
	bfr += field(bfr,mj);
	*bfr++ = ' ';
	cmdcol[2] = bfr - base;
	*bfr++ = ' ';
	*bfr++ = ' ';

	/* translate the filename */
	cmdcol[3] = bfr - base;
	while ((c = *name++) && len-- > 0) {
		if (isascii(c) && isgraph(c))
			*bfr++ = c;
		else
			*bfr++ = '?';
	}
	if (isDIR(mj)) {
		*bfr++ = '/';
	} else if (isLINK(mj) && ((t = flist[inx].ltxt) != 0)) {
		*bfr++ = ' ';
		*bfr++ = '-';
		*bfr++ = '>';
		*bfr++ = ' ';
		bfr += strlen(strcpy(bfr, t));
	} else if (executable(s))	*bfr++ = '*';
	*bfr = '\0';
}

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
{
register char *s = bfr;
register int len = strlen(s);
	if (mode == 0)
		while (*s)	*s++ = ' ';
	return (len);
}
