#ifndef	lint
static	char	Id[] = "$Id: dedscan.c,v 10.2 1992/01/02 09:10:03 dickey Exp $";
#endif

/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		02 Jan 1992, make this work properly if only a filename is
 *			     specified.
 *		18 Oct 1991, converted to ANSI.
 *			     ** hack 'statLINE()' to avoid apollo cc 6.7 bug.
 *		11 Jul 1991, modified interface to 'showFILES()'
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *		18 Apr 1991, modified debug-trace to make it easier to watch
 *		04 Apr 1991, guard against 'getwd()' failure.
 *		13 Aug 1990, lint
 *		27 Jul 1990, modified 'argstat()' and 'dedstat()' so that
 *			     'dedstat()' does all of the stat-work.  Did this
 *			     so that it calls 'statSCCS()' for all directory
 *			     arguments, making 'l' command work properly.  Also,
 *			     this eliminates a redundant 'stat()' in
 *			     'argstat()'.
 *		16 May 1990, added code to strip prefixes which are common to
 *			     the new-wd, but not among the other arguments.
 *		25 Apr 1990, corrected code which tries to circumvent unreadable
 *			     old_wd (had broken the 'R' command in that fix...)
 *			     Refined pathname-conversion using 'abshome()'
 *			     followed by 'getwd()' when needed, rather than
 *			     'abspath()' to avoid confusion with symbolic links.
 *		24 Apr 1990, modified 'argstat()' so that if shell (e.g.,
 *			     Bourne) passes a "~" argument, we expand it
 *			     properly.
 *		23 Apr 1990, modify initial 'chdir()' so we try to recover from
 *			     unreadable directory (e.g., when invoking "su"
 *			     from a protected directory)
 *		18 Apr 1990, invoke 'rcslast()' to pick up information about
 *			     permit-file (e.g., "RCS,v").
 *		16 Oct 1989, suppress 'ft_insert()' for "." and ".."
 *		12 Oct 1989, only fall-thru to 'sccslast()' if we found
 *			     *nothing* of rcs.
 *		06 Oct 1989, modified interface to 'showFILES()'
 *		04 Oct 1989, added A_opt code (permit dot-names)
 *		06 Jun 1989, made read-pattern apply to explicit lists as well.
 *			     Last change broke '@'-toggle; fixed.
 *		05 Jun 1989, simplified interface to ftree-module
 *		31 May 1989, added 'init_scan()' to fix regular-expression
 *			     kludge.
 *		26 May 1989, don't purge ftree if we are using read-expression
 *			     Use read-selection (CTL/R command) to suppress
 *			     unwanted names.
 *		14 Mar 1989, corrections to common-path algorithm
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
 *		09 May 1988, corrected full-name computation for ftree.
 *		22 Apr 1988, use external 'txtalloc()', integrated with ftree.
 *
 * Function:	Scan a list of arguments, to make up a display list.
 * Arguments:   argc, argv passed down from the original invocation, with
 *		leading options parsed off.
 */

#define		DIR_PTYPES	/* includes directory-stuff */
#include	"ded.h"
#include	"rcsdefs.h"
#include	"sccsdefs.h"

#define	def_doalloc	FLIST_alloc
	/*ARGSUSED*/
	def_DOALLOC(FLIST)

#define	N_UNKNOWN	-1	/* name does not exist */
#define	N_FILE		0	/* a file */
#define	N_DIR		1	/* a directory */
#define	N_LDIR		2	/* symbolic link to a directory */

static	int	dir_order;

/************************************************************************
 *	dedscan(@)							*
 *----------------------------------------------------------------------*
 * Function:	Scan a list of arguments, to make up a display list.	*
 * Arguments:   argc, argv passed down from the original invocation,	*
 *		with leading options parsed off.			*
 ************************************************************************/
dedscan(
_ARX(int,	argc)
_AR1(char **,	argv)
	)
_DCL(int,	argc)
_DCL(char **,	argv)
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

	if (argc > 1) {
		(void)chdir(strcpy(new_wd,old_wd));
		for (j = 0; j < argc; j++)
			if (ok_scan(argv[j])
			&&  argstat(argv[j], TRUE) >= 0)
				common = 0;
	} else {
		abshome(pathcat(new_wd, old_wd, argv[0]));
		if (!path_RESOLVE(new_wd)) {
			warn(new_wd);
			return(0);
		}
		if ((common = argstat(new_wd, FALSE)) > 0) {
				/* mark dep's for purge */
			if (toscan == 0)
				ft_remove(new_wd,AT_opt);
			else
				init_scan();

			if (dp = opendir(".")) {
			int	len = strlen(strcpy(name, new_wd));
				if (name[len-1] != '/') {
					name[len++] = '/';
					name[len]   = EOS;
				}
				while (de = readdir(dp)) {
					if (dotname(s = de->d_name))
						if (!A_opt)
							continue;
					if (!ok_scan(s))
						continue;
					j = argstat(strcpy(name+len, s), TRUE);
					if (!dotname(s)
					&&  j > 0
					&&  (k = lookup(s)) >= 0) {
						ft_insert(name);
					}
				}
				(void)closedir(dp);
				if (!numfiles)
					waitmsg("no files found");
			} else {
				waitmsg("cannot open directory");
				return(0);
			}
			if (toscan == 0)
				ft_purge(); /* remove items not reinserted */
		} else if (common == N_FILE)
			numfiles = 1;
	}

	/*
	 * If the user specified in the command arguments a set of files from
	 * multiple directories (or even a lot of files in the same directory)
	 * find the longest common leading pathname component and readjust
	 * everything if it is nonnull.
	 */
	if (debug)
		PRINTF("common=%d, numfiles=%d\r\n", common, numfiles);
	if (common == 0 && numfiles != 0) {
		common = strlen(strcpy(name,argv[0]));
		for (j = 0; (j < argc) && (common > 0); j++) {
			register char	*d = argv[j];
			register int	slash = 0;

			for (s = name, k = 0;
				(d[k] == s[k]) && (d[k] != EOS);) {
				if ((d[k++] == '/')
				&&  (d[k]   != EOS))	/* need a leaf */
					slash = k;	/* ...common-length */
			}
			if (slash < common) {
#ifdef	apollo
				if ((slash == 1)
				&&  (name[0] == '/')) {
					if ((s[1] == '/')
					||  (d[1] == '/'))
						slash = 0; /* fix truncation */
				}
#endif
				common = slash;
			}
			dlog_comment("common '%.*s' (%d:%s)\n", common, name, slash, d);
		}
		name[common] = EOS;

		if (common > 0) {
			dlog_comment("common path = \"%s\" (len=%d)\n",
				name, common);
			if (chdir(strcpy(new_wd,old_wd)) < 0)
				failed(old_wd);
			abshome(strcpy(new_wd, name));
			if (!path_RESOLVE(new_wd))
				failed(new_wd);
			for (j = 0; j < numfiles; j++)
				xNAME(j) = txtalloc(xNAME(j) + common);
		} else {
			size_t	len = strlen(new_wd);
			for (j = 0; j < argc; j++) {
				if (strlen(s = argv[j]) > len
				&&  s[len] == '/'
				&&  !strncmp(new_wd,s,len))
					xNAME(j) = txtalloc(xNAME(j) + len + 1);
			}
		}
	}
	if (debug)
		dedwait(FALSE);
	return(numfiles);
}

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Find a given name in the display list, returning -1 if not found, or index.
 */
static
lookup _ONE(char *, name)
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
append(
_ARX(char *,	name)
_AR1(FLIST *,	f_)
	)
_DCL(char *,	name)
_DCL(FLIST *,	f_)
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
Zero _ONE(FLIST *,	f_)
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
ReZero _ONE(FLIST *,	f_)
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
dedstat (
_ARX(char *,	name)
_AR1(FLIST *,	f_)
	)
_DCL(char *,	name)
_DCL(FLIST *,	f_)
{
#ifdef	S_IFLNK
	int	len;
	char	bfr[BUFSIZ];
#endif
	int	code;

	ReZero(f_);

	if (lstat(name, &f_->s) < 0) {
		ReZero(f_);	/* zero all but name-pointer */
		return(N_UNKNOWN);
	}
	code = isDIR(f_->s.st_mode) ? N_DIR : N_FILE;
#ifdef	S_IFLNK
	if (isLINK(f_->s.st_mode)) {
		len = readlink(name, bfr, sizeof(bfr));
		if (len > 0) {
			bfr[len] = EOS;
			if (f_->ltxt)	txtfree(f_->ltxt);
			f_->ltxt = txtalloc(bfr);
			if (AT_opt
			&&  (stat(name, &f_->s) >= 0)
			&&  isDIR(f_->s.st_mode)) {
				ft_insert(name);
				code = N_LDIR;
			}
		}
	}
#endif
#ifdef	Z_RCS_SCCS
	statSCCS(name, f_);
#endif
	return (code);
}

/*
 * Get statistics for a name which is in the original argument list.
 * We handle a special case: if a single argument is given (!list),
 * links are tested to see if they resolve to a directory.
 */
static
argstat(
_ARX(char *,	name)
_AR1(int,	list)
	)
_DCL(char *,	name)
_DCL(int,	list)
{
	FLIST	fb;
	char	full[BUFSIZ];
	int	code;

	if (debug)
		PRINTF(" stat \"%s\" %slist\r\n", name, list ? "" : "no");

	if (*name == '~')	/* permit "~" from Bourne-shell */
		abshome(name = strcpy(full, name));

	Zero(&fb);
	if ((code = dedstat(name, &fb)) != N_UNKNOWN) {
		blip(code == N_LDIR ? '@' : '.');
		if (list)
			append(name, &fb);
	} else
		blip('?');
	return (code);
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

statSCCS(
_ARX(char *,	name)
_AR1(FLIST *,	f_)
	)
_DCL(char *,	name)
_DCL(FLIST *,	f_)
{
	if (Z_opt) {
		if (isFILE(f_->s.st_mode)) {
#ifdef	Z_RCS
			LAST(rcslast);
#ifdef	Z_SCCS
			if (f_->z_time == 0
			&&  f_->z_vers[0] == '?'
			&&  f_->z_lock[0] == '?')	/* fall-thru ? */
#endif
#endif
#ifdef	Z_SCCS
			LAST(sccslast);
#endif
#ifdef	Z_RCS
		} else if (isDIR(f_->s.st_mode)
			&& sameleaf(name,rcs_dir())) {
			LAST(rcslast);
#endif	/* Z_RCS */
		} else {
			f_->z_lock =
			f_->z_vers = "";
			f_->z_time = 0;
		}
	}
}
#endif	/* Z_RCS_SCCS */

/*
 * This entrypoint is called to re-stat entries which already have been put
 * into the display-list.
 */
statLINE _ONE(int,j)
{
	char *	path = xNAME(j);
	FLIST * blok = &flist[j];
	int	flag = xFLAG(j);
	int	dord = xDORD(j);

	(void)dedstat(path, blok);
	xFLAG(j) = flag;
	xDORD(j) = dord;
}

/*
 * For 'dedmake()', this adds a temporary entry, moving the former current
 * entry down, so the user can edit the name in-place.
 */
statMAKE _ONE(int,mode)
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
	showFILES(FALSE,TRUE);
}

/*
 * Form a regular expression to match the wildcard pattern which the user
 * gave for a filename.
 */
static
make_EXPR _ONE(char *,path)
{
	char	temp[BUFSIZ],
		*s = path,
		*d = temp;
	*d++ = '^';
	while (*s) {
		if (ispunct(*s)) {
			if (*s == '?') {
				*d++ = '.';
				s++;
				continue;
			} else if (*s == '*') {
				*d++ = '.';
				*d++ = '*';
				s++;
				continue;
			}
			*d++ = '\\';
		}
		*d++ = *s++;
	}
	*d++ = '$';
	*d   = EOS;
	dlog_comment("force scan \"%s\"\n", path);
	toscan = txtalloc(temp);
}

/*
 * Change to the given directory, and then (try to) get an absolute path by
 * using the 'getwd()' function.  Note that the latter may fail if we follow
 * a symbolic link into a directory in which we have execute, but no read-
 * access. In this case we try to live with the link-text.
 */
path_RESOLVE _ONE(char *,path)
{
	char	temp[BUFSIZ];
	static	int	tried;

	if (chdir(strcpy(temp, path)) < 0) {
		if (errno == ENOTDIR
		 || errno == ENOENT) {
			char *s = strrchr(temp, '/');
			if (s != 0) {
				s[1] = EOS;
#ifdef	apollo
				if (strcmp(temp, "//"))
#endif	/* apollo */
				if (strcmp(temp, "/"))
					s[0] = EOS;	/* trim trailing '/' */
				if (chdir(temp) < 0)
					return (FALSE);
				if (!tried++)
					make_EXPR(path + (s - temp) + 1);
			}
		} else
			return(FALSE);
	}

	if (getwd(temp))
		(void) strcpy(path, temp);
	else {	/* for SunOS? */
		FLIST	fb;
		int	save = AT_opt;
		int	code;
		AT_opt	= TRUE;
		code	= dedstat(path, &fb);
		AT_opt	= save;
		if (code == N_LDIR)
			(void)strcpy (path, fb.ltxt);
	}
	return (TRUE);
}
