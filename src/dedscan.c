#ifndef	lint
static	char	sccs_id[] = "@(#)dedscan.c	1.25 88/09/12 12:35:01";
#endif	lint

/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		12 Sep 1988, don't force resolution of symbolic links unless
 *			     AT_opt is set.  Added 'statMAKE()'.
 *		01 Sep 1988, look for, and reduce common leading pathname.
 *		17 Aug 1988, don't use 'dedmsg()', which assumes cursor position
 *		12 Aug 1988, added 'dir_order' to support sort into the order
 *			     in which we obtained names from the directory.
 *			     This is useful for tracking programs such as
 *			     'find', which operate in directory-order.
 *		04 Aug 1988, added debug-option
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

#define		DIR_PTYPES	/* includes directory-stuff */
#include	"ded.h"
extern	FLIST	*dedfree();
extern	char	*strchr();
extern	char	*strrchr();
extern	char	*txtalloc();

extern	int	debug;

#define	def_doalloc	FLIST_alloc
	/*ARGSUSED*/
	def_DOALLOC(FLIST)

static	int	dir_order;

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
	register int	j, k;
	auto	 int	common = -1;
	char	name[BUFSIZ];
	char	*s;

	flist = dedfree(flist, numfiles);
	dir_order = 0;

	numfiles = 0;
	if (chdir(strcpy(new_wd,old_wd)) < 0)
		failed(old_wd);
	if (argc > 1) {
		for (j = 0; j < argc; j++)
			if (argstat(argv[j], TRUE) >= 0)
				common = 0;
	} else {
		if ((common = argstat(argv[0], FALSE)) > 0) {
			if (chdir(strcpy(new_wd, argv[0])) < 0) {
				warn(new_wd);
				return(0);
			}
				/* mark dep's for purge */
			ft_remove(getwd(new_wd),AT_opt);
			if (dp = opendir(".")) {
			int	len = strlen(strcpy(name, new_wd));
				if (name[len-1] != '/') {
					name[len++] = '/';
					name[len]   = EOS;
				}
				while (de = readdir(dp)) {
					if (dotname(s = de->d_name))
						continue;
					if (debug)
						PRINTF(" file \"%s\"\r\n", s);
					j = argstat(strcpy(name+len, s), TRUE);
					if (j > 0
					&&  (k = lookup(s)) >= 0) {
#ifdef	S_IFLNK
						if (xLTXT(k))
							ft_linkto(name);
						else
#endif	S_IFLNK
							ft_insert(name);
					}
				}
				closedir(dp);
				if (!numfiles)
					waitmsg("no files found");
			}
			ft_purge();	/* remove items not reinserted */
		}
	}

	/*
	 * If the user specified in the command arguments a set of files from
	 * multiple directories (or even a lot of files in the same directory)
	 * find the longest common leading pathname component and readjust
	 * everything if it is nonnull.
	 */
	if (common == 0 && numfiles != 0) {
		common = strlen(strcpy(name,argv[0]));
		for (j = 0; j < argc; j++) {
			char	*z = argv[j];
			if ((k = strlen(z)) > common
			&&  !strncmp(z, name, common))
				continue;
			else {
				char	*t = 0;
				if ((k > common)
				&&  (t = strchr(z+common, '/')))
					*t = EOS;
				s = strrchr(z, '/');
				if (t != 0)
					*t = '/';
#ifdef	apollo
				if (!strcmp(name, "//"))
					s = 0;	/* truncating to "/" is err */
#endif	apollo
				if (s != 0) {
					if ((k = (s + 1 - z)) < common) {
						common = k;
						strcpy(name, z)[k] = EOS;
					}
				} else {	/* give up */
					name[common = 0] = EOS;
					break;
				}
			}
		}

		if (common > 0 && chdir(name) >= 0) {
			abspath(strcpy(new_wd, name));
			for (j = 0; j < numfiles; j++)
				xNAME(j) = txtalloc(xNAME(j) + common);
		}
	}
	if (debug)
		dedwait();
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
		if (!strcmp(xNAME(j), name))
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

	if (j >= 0) {
		name     = xNAME(j);
		flist[j] = *f_;
		xNAME(j) = name;
		return;
	}

	/* append a new entry on the end of the list */
	j = (numfiles | 31) + 1;
	flist = DOALLOC(flist,FLIST,(unsigned)j);

	Zero(&flist[numfiles]);
	flist[numfiles] = *f_;
	xNAME(numfiles) = txtalloc(name);
	xDORD(numfiles) = dir_order++;
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
#ifdef	S_IFLNK
int	len;
char	bfr[BUFSIZ];
#endif	S_IFLNK

	ReZero(f_);

	if (lstat(name, &f_->s) < 0) {
		ReZero(f_);	/* zero all but name-pointer */
		return(-1);
	}
	if (isDIR(f_->s.st_mode))
		return(1);
#ifdef	S_IFLNK
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
#endif	S_IFLNK
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
#ifdef	S_IFLNK
		if (!list && (fb.ltxt != 0)) {
			struct	stat	sb;
			if ((stat(name, &sb) >= 0)
			&&  isDIR(sb.st_mode))
				return(2);		/* link to directory */
		}
#endif	S_IFLNK
		if (!(fb.ltxt || isDIR(fb.s.st_mode)) || list) {
			blip('.');
			append(name, &fb);
		}
		return(isDIR(fb.s.st_mode) ? 1 : 0);	/* directory ? */
	}
	return(-1);					/* not found */
}

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
	int	flag = xFLAG(j);
	int	dord = xDORD(j);

	(void)dedstat(xNAME(j), &flist[j]);
	xFLAG(j) = flag;
	xDORD(j) = dord;
}

/*
 * For 'dedmake()', this adds a temporary entry, moving the former current
 * entry down, so the user can edit the name in-place.
 */
statMAKE(mode)
{
	static	char	*null = "";
	static	FLIST	dummy;
	register int	x;

	if (mode) {
		dummy.s.st_mode = mode;
		dummy.s.st_uid  = getuid();
		dummy.s.st_gid  = getgid();
		append(null, &dummy);
		if ((x = lookup(null)) != curfile) {
			FLIST	save;
			save = flist[x];
			if (x < curfile) {
				while (x++ < curfile)
					flist[x-1] = flist[x];
			} else {
				while (x-- > curfile)
					flist[x+1] = flist[x];
			}
			flist[curfile] = save;
		}
	} else {	/* remove entry */
		if ((x = lookup(null)) >= 0) {
			while (x++ < numfiles)
				flist[x-1] = flist[x];
			numfiles--;
		}
	}
	showFILES();
}
