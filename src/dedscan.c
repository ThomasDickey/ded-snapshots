#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)dedscan.c	1.16 88/06/16 09:09:31";
#endif	NO_SCCS_ID

/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		01 Jun 1988, added 'z_lock'.
 *		26 May 1988, made 'flist[]' allocation in chunks.
 *		23 May 1988, changed interface to 'rcslast()', 'sccslast()'.
 *		13 May 1988, put only links-to-directory via 'ft_linkto()'.
 *		09 May 1988, corrected full-name computation for ftree.
 *		22 Apr 1988, use external 'txtalloc()', integrated with ftree.
 *
 * Function:	Scan a list of arguments, to make up a display list.
 * Arguments:   argc, argv passed down from the original invocation, with
 *		leading options parsed off.
 */
#include	"ded.h"
extern	FLIST	*dedfree();
extern	char	*txtalloc();

/************************************************************************
 *	dedscan(@)							*
 *----------------------------------------------------------------------*
 * Function:	Scan a list of arguments, to make up a display list.	*
 * Arguments:   argc, argv passed down from the original invocation,	*
 *		with leading options parsed off.			*
 ************************************************************************/
dedscan(argc, argv)
char	*argv[];
{
DIR		*dp;
struct	direct	*de;
register int	j;
char	name[BUFSIZ];

	flist = dedfree(flist, numfiles);

	if (argc == 0) {
	static	char	dot[]	= ".",
			*dot_[]	= {dot,0};
		argv = dot_;
		argc = 1;
	}

	numfiles = 0;
	chdir(strcpy(new_wd,old_wd));
	if (argc > 1) {
		for (j = 0; j < argc; j++)
			(void)argstat(argv[j], TRUE);
	} else {
		if (argstat(argv[0], FALSE) > 0) {
			if (chdir(strcpy(new_wd, argv[0])) < 0) {
				warn(new_wd);
				return(0);
			}
				/* mark dep's for purge */
			ft_remove(getwd(new_wd));
			if (dp = opendir(".")) {
			int	len = strlen(strcpy(name, new_wd));
				if (name[len-1] != '/') {
					name[len++] = '/';
					name[len]   = '\0';
				}
				while (de = readdir(dp)) {
					if (dotname(de->d_name))
						continue;
					j = argstat(strcpy(name+len,
							de->d_name), TRUE);
					if (j > 0)
						ft_insert(name);
#ifndef	SYSTEM5
					if ((j == 0)
					&&  ((j = lookup(de->d_name)) >= 0)
					&&  (linkstat(name,flist+j)) )
						ft_linkto(name);
#endif	SYSTEM5
				}
				closedir(dp);
				if (!numfiles)
					dedmsg("no files found");
			}
			ft_purge();	/* remove items not reinserted */
		}
	}
	return(numfiles);
}

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Find a given name in the display list, returning -1 if not found, or index.
 */
static
lookup(name)
char	*name;
{
register int j;

	for (j = 0; j < numfiles; j++) {
		if (!strcmp(flist[j].name, name))
			return (j);
	}
	return (-1);
}

/*
 * For a given name, update/append to the display-list the new FLIST data.
 */
static
append(name, f_)
char	*name;
FLIST	*f_;
{
register int j = lookup(name);

	blip('.');
	if (j >= 0) {
		name          = flist[j].name;
		flist[j]      = *f_;
		flist[j].name = name;
		return;
	}

	/* append a new entry on the end of the list */
	j = (numfiles | 31) + 1;
	flist = DOALLOC(FLIST,flist,j);

	Zero(&flist[numfiles]);
	flist[numfiles]      = *f_;
	flist[numfiles].name = txtalloc(name);
	numfiles++;
}

/*
 * Clear an FLIST data block.
 */
static
Zero(f_)
FLIST	*f_;
{
register char *s = (char *)f_;
register int  len = sizeof(FLIST);
	while (len-- > 0) *s++ = 0;
}

/*
 * Reset an FLIST data block.  Release storage used by symbolic link, but
 * retain the name-string.
 */
static
ReZero(f_)
FLIST	*f_;
{
char	*name = f_->name;
	if (f_->ltxt)	txtfree(f_->ltxt);
	Zero(f_);
	f_->name = name;
}

/*
 * Find the given file's stat-block.  Use the return-code to distinguish the
 * directories from ordinary files.  Save link-text for display, but don't
 * worry about whether a symbolic link really points anywhere real.
 * (We will worry about that when we have to do something with it.)
 *
 * If the flag AT_opt is set, we obtain the stat for the target, and will use
 * the presence of the 'ltxt' to do basic testing on whether the file was a
 * symbolic link.
 */
static
dedstat (name, f_)
char	*name;
FLIST	*f_;
{
int	len;
char	bfr[BUFSIZ];

	ReZero(f_);

	if (lstat(name, &f_->s) < 0) {
		ReZero(f_);	/* zero all but name-pointer */
		return(-1);
	}
	if (isDIR(f_->s.st_mode))
		return(1);
#ifndef	SYSTEM5
	if (isLINK(f_->s.st_mode)) {
		len = readlink(name, bfr, sizeof(bfr));
		if (len > 0) {
			bfr[len] = EOS;
			if (f_->ltxt)	txtfree(f_->ltxt);
			f_->ltxt = txtalloc(bfr);
			if (AT_opt)
				(void)stat(name, &f_->s);
		}
	}
#endif	SYSTEM5
#ifdef	Z_RCS_SCCS
	statSCCS(name, f_);
#endif	Z_RCS_SCCS
	return (0);
}

/*
 * Get statistics for a name which is in the original argument list.
 * We handle a special case: if a single argument is given (!list),
 * links are tested to see if they resolve to a directory.
 */
static
argstat(name, list)
char	*name;
{
FLIST	fb;

	Zero(&fb);
	if (dedstat(name, &fb) >= 0) {
#ifndef	SYSTEM5
		if (!list && linkstat(name, &fb))
			return(2);			/* link to directory */
#endif	SYSTEM5
		if (!(fb.ltxt || isDIR(fb.s.st_mode)) || list)
			append(name, &fb);
		return(isDIR(fb.s.st_mode) ? 1 : 0);	/* directory ? */
	}
	return(-1);					/* not found */
}

/*
 * Test a name to see if it is a symbolic link to a directory.
 */
#ifndef	SYSTEM5
static
linkstat(name, f)
char	*name;
FLIST	*f;
{
struct	stat	sb;
	if (f->ltxt) {	/* we already decided this is a link */
		if (stat(name, &sb) >= 0) {
			if (isDIR(sb.st_mode))
				return(TRUE);	/* link to directory */
		}
	}
	return (FALSE);
}
#endif	SYSTEM5

/************************************************************************
 *	alternate entrypoints						*
 ************************************************************************/

/*
 * This entrypoint explicitly examines a file to see what sccs file relates
 * to it.  It is called both locally (within this module), and on-the-fly from
 * the main command-decoder when we set the Z_opt flag.
 */
#ifdef	Z_RCS_SCCS
#define	LAST(p)	p(new_wd, name, &(f_->z_vers), &(f_->z_time), &(f_->z_lock))
statSCCS(name, f_)
char	*name;
FLIST	*f_;
{
	if (Z_opt) {
		if (isFILE(f_->s.st_mode)) {
#ifdef	Z_RCS
			LAST(rcslast);
#ifdef	Z_SCCS
			if (f_->z_time == 0)
#endif	Z_SCCS
#endif	Z_RCS
#ifdef	Z_SCCS
			LAST(sccslast);
#endif	Z_SCCS
		} else {
			f_->z_lock =
			f_->z_vers = "";
			f_->z_time = 0;
		}
	}
}
#endif	Z_RCS_SCCS

/*
 * This entrypoint is called to re-stat entries which already have been put
 * into the display-list.
 */
statLINE(j)
{
int	flag = flist[j].flag;
	(void)dedstat(flist[j].name, &flist[j]);
	flist[j].flag = flag;
}
