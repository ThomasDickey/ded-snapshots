/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		05 Nov 1995, prevent tilde-expansion on names read via readdir.
 *		03 Sep 1995, modify path_RESOLVE to ensure that parent isn't
 *			     already in ring (if so, return failure).
 *		30 Aug 1995, make A_opt apply to all dot-files
 *		03 Aug 1994, split out 'lastrev()'
 *		23 Jul 1994, force "." into empty filelists.
 *		23 Nov 1993, new blip-code.
 *		29 Oct 1993, ifdef-ident, port to HP/UX.
 *		28 Sep 1993, gcc warnings
 *		01 Apr 1992, convert most global variables to RING-struct.
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
#include	"cmv_defs.h"
#include	"rcsdefs.h"
#include	"sccsdefs.h"

MODULE_ID("$Id: dedscan.c,v 12.24 1995/11/05 22:49:45 tom Exp $")

#define	def_doalloc	FLIST_alloc
	/*ARGSUSED*/
	def_DOALLOC(FLIST)

#define	N_UNKNOWN	-1	/* name does not exist */
#define	N_FILE		0	/* a file */
#define	N_DIR		1	/* a directory */
#define	N_LDIR		2	/* symbolic link to a directory */

static	int	dir_order;

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Find a given name in the display list, returning -1 if not found, or index.
 */
private	int	lookup (
	_ARX(RING *,	gbl)
	_AR1(char *,	name)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
{
	register int j;

	for (j = 0; j < gbl->numfiles; j++) {
		if (!strcmp(gNAME(j), name))
			return (j);
	}
	return (-1);
}

/*
 * Clear an FLIST data block.
 */
#define	Zero(p)	(void)memset(p, 0, sizeof(FLIST))

/*
 * Reset an FLIST data block.  Release storage used by symbolic link, but
 * retain the name-string.
 */
private	void	ReZero (
	_AR1(FLIST *,	f_))
	_DCL(FLIST *,	f_)
{
	char	*name = f_->name;
	if (f_->ltxt)	txtfree(f_->ltxt);
	Zero(f_);
	f_->name = name;
}

/*
 * For a given name, update/append to the display-list the new FLIST data.
 */
private	void	append(
	_ARX(RING *,	gbl)
	_ARX(char *,	name)
	_AR1(FLIST *,	f_)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
	_DCL(FLIST *,	f_)
{
	register int j = lookup(gbl, name);

	if (j >= 0) {
		name     = gNAME(j);
		gENTRY(j) = *f_;
		gNAME(j) = name;
		return;
	}

	/* append a new entry on the end of the list */
	j = (gbl->numfiles | 31) + 1;
	gbl->flist = DOALLOC(gbl->flist,FLIST,(unsigned)j);

	Zero(&gENTRY(gbl->numfiles));
	gENTRY(gbl->numfiles) = *f_;
	gNAME(gbl->numfiles) = txtalloc(name);
	gDORD(gbl->numfiles) = dir_order++;
	gbl->numfiles++;
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
private	int	dedstat (
	_ARX(RING *,	gbl)
	_ARX(char *,	name)
	_AR1(FLIST *,	f_)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
	_DCL(FLIST *,	f_)
{
#ifdef	S_IFLNK
	int	len;
	char	bfr[MAXPATHLEN];
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
			if (gbl->AT_opt
			&&  (stat(name, &f_->s) >= 0)
			&&  isDIR(f_->s.st_mode)) {
				ft_insert(name);
				code = N_LDIR;
			}
		}
	}
#endif
#ifdef	Z_RCS_SCCS
	statSCCS(gbl, name, f_);
#endif
	return (code);
}

/*
 * Get statistics for a name which is in the original argument list.  We handle
 * a special case: if a single argument is given (!list), links are tested to
 * see if they resolve to a directory.
 */
private	int	argstat(
	_ARX(RING *,	gbl)
	_ARX(char *,	name)
	_ARX(int,	list)
	_AR1(int,	tilde)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
	_DCL(int,	list)
	_DCL(int,	tilde)
{
	FLIST	fb;
	char	full[MAXPATHLEN];
	int	code;

	if (debug) {
		PRINTF(" stat \"%s\" %slist\r\n", name, list ? "" : "no");
		FFLUSH(stdout);
	}

	if (tilde && (*name == '~'))	/* permit "~" from Bourne-shell */
		abshome(name = strcpy(full, name));

	Zero(&fb);
	if ((code = dedstat(gbl, name, &fb)) != N_UNKNOWN) {
		put_dedblip(code == N_LDIR ? '@' : '.');
		if (list)
			append(gbl, name, &fb);
	} else
		put_dedblip('?');
	return (code);
}

/************************************************************************
 *	dedscan(@)							*
 *----------------------------------------------------------------------*
 * Function:	Scan a list of arguments, to make up a display list.	*
 * Arguments:   argc, argv passed down from the original invocation,	*
 *		with leading options parsed off.			*
 ************************************************************************/
public	int	dedscan (
	_AR1(RING *,	gbl))
	_DCL(RING *,	gbl)
{
	auto	int	argc	= gbl->top_argc;
	auto	char **	argv	= gbl->top_argv;
	auto	DIR	*dp;
	auto	DirentT *de;
	register int	j, k;
	auto	 int	common = -1;
	char	name[MAXPATHLEN];
	char	*s;

	set_dedblip(gbl);
	gbl->flist = dedfree(gbl->flist, gbl->numfiles);
	dir_order = 0;
	gbl->numfiles = 0;

	if (argc > 1) {
		(void)chdir(strcpy(gbl->new_wd,old_wd));
		for (j = 0; j < argc; j++)
			if (ok_scan(gbl, argv[j])
			&&  argstat(gbl, argv[j], TRUE, TRUE) >= 0)
				common = 0;
	} else {
		abshome(pathcat(gbl->new_wd, old_wd, argv[0]));
		if (!path_RESOLVE(gbl, gbl->new_wd)) {
			return(0);
		}

		if ((common = argstat(gbl, gbl->new_wd, FALSE, FALSE)) > 0) {
				/* mark dep's for purge */
			if (gbl->toscan == 0)
				ft_remove(gbl->new_wd, gbl->AT_opt, gbl->A_opt);
			else
				init_scan(gbl);

			set_dedblip(gbl);
			if ((dp = opendir(".")) != NULL) {
			int	len = strlen(strcpy(name, gbl->new_wd));
				if (name[len-1] != '/') {
					name[len++] = '/';
					name[len]   = EOS;
				}
				while ((de = readdir(dp)) != NULL) {
					s = de->d_name;
					if (*s == '.' && !gbl->A_opt)
						continue;
					if (!ok_scan(gbl, s))
						continue;
					j = argstat(gbl, strcpy(name+len, s), TRUE, FALSE);
					if (!dotname(s)
					&&  j > 0
					&&  (k = lookup(gbl, s)) >= 0) {
						ft_insert(name);
					}
				}
				(void)closedir(dp);
				/*
				 * If nothing else, force "." to appear in the
				 * list.  This greatly simplifies the handling
				 * of empty directory lists!
				 */
				if (!gbl->numfiles) {
					(void)argstat(gbl, ".", TRUE, FALSE);
				}
			} else {
				waitmsg("cannot open directory");
				return(0);
			}
			if (gbl->toscan == 0)
				ft_purge(gbl); /* remove items not reinserted */
		} else if (common == N_FILE)
			gbl->numfiles = 1;
	}

	/*
	 * If the user specified in the command arguments a set of files from
	 * multiple directories (or even a lot of files in the same directory)
	 * find the longest common leading pathname component and readjust
	 * everything if it is nonnull.
	 */
	if (debug)
		PRINTF("common=%d, numfiles=%d\r\n", common, gbl->numfiles);
	if (common == 0 && gbl->numfiles != 0) {
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
			if (chdir(strcpy(gbl->new_wd,old_wd)) < 0)
				failed(old_wd);
			abshome(strcpy(gbl->new_wd, name));
			if (!path_RESOLVE(gbl, gbl->new_wd))
				failed(gbl->new_wd);
			for (j = 0; j < gbl->numfiles; j++)
				gNAME(j) = txtalloc(gNAME(j) + common);
		} else {
			size_t	len = strlen(gbl->new_wd);
			for (j = 0; j < argc; j++) {
				if (strlen(s = argv[j]) > len
				&&  s[len] == '/'
				&&  !strncmp(gbl->new_wd,s,len))
					gNAME(j) = txtalloc(gNAME(j) + len + 1);
			}
		}
	}
	if (debug)
		dedwait(gbl, FALSE);

	gbl->curfile = 0;
	dedsort(gbl);
	gbl->curfile = 0;	/* ensure consistent initial */

	return(gbl->numfiles);
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
#define	LAST(p)	p(gbl->new_wd, name, &(f_->z_vers), &(f_->z_time), &(f_->z_lock))

public	void	statSCCS(
	_ARX(RING *,	gbl)
	_ARX(char *,	name)
	_AR1(FLIST *,	f_)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
	_DCL(FLIST *,	f_)
{
	if (gbl->Z_opt) {
		if (isFILE(f_->s.st_mode)) {
#ifdef CMV_PATH
			purge_cmv_dir(gbl->new_wd, name);
#endif
			lastrev(gbl->new_wd,
				name,
				&(f_->z_vers),
				&(f_->z_time),
				&(f_->z_lock));
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
public	void	statLINE (
	_ARX(RING *,	gbl)
	_AR1(int,	j)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	j)
{
	char *	path = gNAME(j);
	FLIST * blok = &gENTRY(j);
	int	flag = gFLAG(j);
	int	dord = gDORD(j);

	(void)dedstat(gbl, path, blok);
	gFLAG(j) = flag;
	gDORD(j) = dord;
}

/*
 * For 'dedmake()', this adds a temporary entry, moving the former current
 * entry down, so the user can edit the name in-place.
 */
public	void	statMAKE (
	_ARX(RING *,	gbl)
	_AR1(int,	mode)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	mode)
{
	static	char	*null = "";
	register int	x;

	if (mode) {
		FLIST	dummy;
		memset(&dummy, 0, sizeof(FLIST));
		dummy.s.st_mode = mode;
		dummy.s.st_uid  = getuid();
		dummy.s.st_gid  = getgid();
		append(gbl, null, &dummy);
		if ((x = lookup(gbl, null)) != gbl->curfile) {
			FLIST	save;
			save = gENTRY(x);
			if (x < gbl->curfile) {
				while (x++ < gbl->curfile)
					gENTRY(x-1) = gENTRY(x);
			} else {
				while (x-- > gbl->curfile)
					gENTRY(x+1) = gENTRY(x);
			}
			cENTRY = save;
		}
	} else {	/* remove entry */
		if ((x = lookup(gbl, null)) >= 0) {
			while (x++ < gbl->numfiles)
				gENTRY(x-1) = gENTRY(x);
			gbl->numfiles--;
		}
	}
	showFILES(gbl,FALSE);
}

/*
 * Form a regular expression to match the wildcard pattern which the user
 * gave for a filename.
 */
private	char *	make_EXPR (
	_AR1(char *,	path))
	_DCL(char *,	path)
{
	char	temp[MAXPATHLEN],
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
	return txtalloc(temp);
}

/*
 * Change to the given directory, and then (try to) get an absolute path by
 * using the 'getwd()' function.  Note that the latter may fail if we follow a
 * symbolic link into a directory in which we have execute, but no read-
 * access.  In this case we try to live with the link-text.
 */
public	int	path_RESOLVE (
	_ARX(RING *,	gbl)
	_AR1(char *,	path)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	path)
{
	char	temp[MAXPATHLEN];
	char	*s;
	static	int	tried;

	if (chdir(strcpy(temp, path)) < 0) {
		if (errno == ENOTDIR
		 || errno == ENOENT) {
			/*
			 * Try to find the parent directory, then, and make a
			 * pattern to match the leaf.  We'll do that only once,
			 * to handle unresolved wildcards from the command
			 * line.
			 */
			s = strrchr(temp, PATH_SLASH);
			if (s != 0) {
				s[1] = EOS;
#ifdef	apollo
				if (strcmp(temp, "//"))
#endif	/* apollo */
				if (strcmp(temp, "/"))
					s[0] = EOS;	/* trim trailing '/' */
				/*
				 * If we've already got the parent directory in
				 * the ring, give up, removing this entry.
				 */
				if (ring_get(temp) != 0) {
					warn(gbl, gbl->new_wd);
					dedring(gbl, ".", 'Q', 1, FALSE, (char *)0);
					return (FALSE);
				}
				if (chdir(temp) < 0)
					return (FALSE);
				if (first_scan && !tried++)
					gbl->toscan = make_EXPR(path + (s - temp) + 1);
			}
		} else {
			warn(gbl, gbl->new_wd);
			return(FALSE);
		}
	}

#if HAVE_REALPATH
	s = realpath(path, temp);
#else
	s = getwd(temp);
#endif
	if (s != 0) {
		(void) strcpy(path, temp);
	} else {	/* for SunOS? */
		FLIST	fb;
		int	save = gbl->AT_opt;
		int	code;
		gbl->AT_opt = TRUE;
		code	= dedstat(gbl, path, &fb);
		gbl->AT_opt = save;
		if (code == N_LDIR)
			(void)strcpy (path, fb.ltxt);
	}
	return (TRUE);
}
