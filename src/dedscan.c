#ifndef	NO_SCCS_ID
static	char	sccs_id[] = "@(#)dedscan.c	1.3 88/03/24 13:15:25";
#endif	NO_SCCS_ID

/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *
 * Function:	Scan a list of arguments, to make up a display list.
 * Arguments:   argc, argv passed down from the original invocation, with leading
 *		options parsed off.
 */
#include	"ded.h"

/********************************************************************************
 *	dedscan(@)								*
 *------------------------------------------------------------------------------*
 * Function:	Scan a list of arguments, to make up a display list.		*
 * Arguments:   argc, argv passed down from the original invocation, with	*
 *		leading options parsed off.					*
 *******************************************************************************/
dedscan(argc, argv)
char	*argv[];
{
DIR		*dp;
struct	direct	*de;
register int	j;
char	name[BUFSIZ];

	if (flist != 0) {	/* we are rescanning display-list */
		for (j = 0; j < numfiles; j++) {
			if (flist[j].name)	free(flist[j].name);
			if (flist[j].ltxt)	free(flist[j].ltxt);
		}
		free(flist);
		flist = 0;
	}

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
			getcwd(new_wd, sizeof(new_wd)-2);
			if (dp = opendir(".")) {
				while (de = readdir(dp)) {
					(void)argstat(strcpy(name, de->d_name), TRUE);
				}
				closedir(dp);
				if (!numfiles)
					dedmsg("no files found");
			}
		}
	}
	return(numfiles);
}

/********************************************************************************
 *	local procedures							*
 *******************************************************************************/

/*
 * Allocate space for, and load, a null-terminated string.  Return its address.
 */
static
char *
stralloc(d,s)
char	*d, *s;
{
	return (strcpy(doalloc(d, (unsigned)strlen(s)+1), s));
}

/*
 * For a given name, update/append to the display-list the new FLIST data.
 */
static
append(name, f_)
char	*name;
FLIST	*f_;
{
register int j;

	blip('.');
	for (j = 0; j < numfiles; j++) {
		if (!strcmp(flist[j].name, name)) {
			name          = flist[j].name;
			flist[j]      = *f_;
			flist[j].name = name;
			return;
		}
	}

	/* append a new entry on the end of the list */
	flist = (FLIST *)doalloc(flist, (numfiles+1) * sizeof(FLIST));

	Zero(&flist[numfiles]);
	flist[numfiles]      = *f_;
	flist[numfiles].name = stralloc((char *)0, name);
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
	if (f_->ltxt)	free(f_->ltxt);
	Zero(f_);
	f_->name = name;
}

/*
 * Find the given file's stat-block.  Use the return-code to distinguish the
 * directories from ordinary files.  Save link-text for display, but don't
 * worry about whether a symbolic link really points anywhere real.
 * (We will worry about that when we have to do something with it.)
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
	if (isLINK(f_->s.st_mode)) {
		len = readlink(name, bfr, sizeof(bfr));
		if (len > 0) {
			bfr[len] = EOS;
			f_->ltxt = stralloc(f_->ltxt, bfr);
		}
	}
#ifdef	Z_SCCS
	statSCCS(name, f_);
#endif	Z_SCCS
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
struct	stat	sb;

	Zero(&fb);
	if (dedstat(name, &fb) >= 0) {
		if (!list && isLINK(fb.s.st_mode)) {
			if (stat(name, &sb) >= 0) {
				if (isDIR(sb.st_mode))
					return(TRUE);
			}
		}
		if (!isDIR(fb.s.st_mode) || list)
			append(name, &fb);
		return(isDIR(fb.s.st_mode));
	}
	return(-1);
}

/********************************************************************************
 *	alternate entrypoints							*
 *******************************************************************************/

/*
 * This entrypoint explicitly examines a file to see what sccs file relates
 * to it.  It is called both locally (within this module), and on-the-fly from
 * the main command-decoder when we set the Z_opt flag.
 */
#ifdef	Z_SCCS
statSCCS(name, f_)
char	*name;
FLIST	*f_;
{
	if (Z_opt && isFILE(f_->s.st_mode)) {
	extern	long	sccszone();
		sccslast(new_wd, name,
			&(f_->z_rels), &(f_->z_vers), &(f_->z_time));
		if (f_->z_time != 0L)	f_->z_time -= sccszone();
	}
}
#endif	Z_SCCS

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
