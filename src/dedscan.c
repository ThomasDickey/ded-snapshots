/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * Modified:
 *		01 May 2020, fix coverity warnings
 *		14 Dec 2014, fix coverity warnings
 *		22 Jul 2014, improve support for caseless filenames.
 *		30 Jan 2011, merge -d and DED_DEBUG environment variable, and
 *			     only do a core-dump on level 3.  Fix special case
 *			     which caused exit when piping single filename to
 *			     ded.
 *		25 May 2010, fix clang --analyze warnings.
 *		15 Nov 2009, fix an out-of-bounds indexing in dedscan() when
 *			     doing a ^R pattern on pathnames read from stdin.
 *		07 Mar 2004, remove K&R support, indent'd
 *		15 Jul 2001, fix uninitialized FLIST struct in dedstat() call
 *			     in path_RESOLVE() - U/Win.
 *		29 Jan 2001, support caseless filenames.
 *		15 Feb 1998, corrected ifdef'ing of realpath vs chdir.
 *		02 Feb 1997, add chdir's within path_RESOLVE to make it work
 *			     with relative path (e.g., "./src/") as an argument.
 *		12 Jan 1997, filename-only case still wasn't right, since it
 *			     didn't properly update the new_wd & argv[].
 *		08 Jan 1997, correct missing allocation for special case where
 *			     only a filename is specified.
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
#include	<ded.h>
#include	<cmv_defs.h>
#include	<rcsdefs.h>
#include	<sccsdefs.h>

MODULE_ID("$Id: dedscan.c,v 12.55 2025/01/07 01:19:00 tom Exp $")

#define	N_UNKNOWN	-1	/* name does not exist */
#define	N_FILE		0	/* a file (synonym for 'common==0') */
#define	N_DIR		1	/* a directory */
#define	N_LDIR		2	/* symbolic link to a directory */

static ORDER_T dir_order;

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Find a given name in the display list, returning -1 if not found, or index.
 */
static int
lookup(RING * gbl, const char *name)
{
    unsigned j;

    for_each_file(gbl, j) {
	if (!strcmp(gENTRY(j).z_real_name, name))
	    return (int) (j);
#ifndef MIXEDCASE_FILENAMES
	/*
	 * As implemented on Windows and OSX, case-preserving names are
	 * reasonably stable, i.e., most system calls return the same sense
	 * of the names.  But we first check against the monocase version
	 * just to be sure.
	 */
	if (!strcasecmp(gENTRY(j).z_name, name))
	    return (int) (j);
#endif
    }
    return (-1);
}

/*
 * Clear an FLIST data block.
 */
#define	Zero(p)	(void)memset(p, 0, sizeof(FLIST))

static void
alloc_name(FLIST * f_, const char *name)
{
    if (name != NULL) {
#ifndef MIXEDCASE_FILENAMES
	char bfr[MAXPATHLEN];
	strlwrcpy(bfr, name);
	f_->z_mono_name = txtalloc(bfr);
#endif
	f_->z_name = txtalloc(name);
    }
}

/*
 * Reset an FLIST data block.  Release storage used by symbolic link, but
 * retain the name-string.
 */
static void
ReZero(FLIST * f_)
{
    char *name = f_->z_name;
    Zero(f_);
    alloc_name(f_, name);
}

#define CHUNK (2 << 5)

#define CHUNKED(n) (((n + 1) | (CHUNK - 1)) + 1)

/*
 * For a given name, update/append to the display-list the new FLIST data.
 */
static void
append(RING * gbl, const char *name, FLIST * f_)
{
    FLIST *ptr;
    int have = lookup(gbl, name);
    unsigned need;

    if (have >= 0) {
	gENTRY(have) = *f_;
	alloc_name(&gENTRY(have), name);
	return;
    }

    /* append a new entry on the end of the list */
    need = CHUNKED(gbl->numfiles + 1);
    if (gbl->flist == NULL || (need != CHUNKED(gbl->numfiles))) {
	ptr = DOALLOC(gbl->flist, FLIST, (unsigned) need);
	if (ptr != gbl->flist) {
	    dlog_comment("append numfiles %d, need %d, size %u ->%p\n",
			 gbl->numfiles + 1, need, sizeof(FLIST) * need, ptr);
	    gbl->flist = ptr;
	}
    }

    Zero(&gENTRY(gbl->numfiles));
    gENTRY(gbl->numfiles) = *f_;
    alloc_name(&gENTRY(gbl->numfiles), name);
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
 * the presence of the 'z_ltxt' to do basic testing on whether the file was a
 * symbolic link.
 */
static int
dedstat(RING * gbl, const char *name, FLIST * f_)
{
#ifdef	S_IFLNK
    int len;
    char bfr[MAXPATHLEN];
#endif
    int code;

    ReZero(f_);

    if (lstat(name, &f_->s) < 0) {
	ReZero(f_);		/* zero all but name-pointer */
	return (N_UNKNOWN);
    }
    code = isDIR(f_->s.st_mode) ? N_DIR : N_FILE;
#ifdef	S_IFLNK
    if (isLINK(f_->s.st_mode)) {
	len = (int) readlink(name, bfr, sizeof(bfr) - 1);
	if (len > 0) {
	    bfr[len] = EOS;
	    if (f_->z_ltxt)
		txtfree(f_->z_ltxt);
	    f_->z_ltxt = txtalloc(bfr);
	    if (gbl->AT_opt
		&& (stat(name, &f_->s) >= 0)
		&& isDIR(f_->s.st_mode)) {
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
static int
argstat(RING * gbl, const char *name, int list, int tilde)
{
    FLIST fb;
    char full[MAXPATHLEN];
    int code;

    if (debug) {
	FPRINTF(debug_fp, " stat \"%s\" %slist\r\n", name, list ? "" : "no");
	FFLUSH(debug_fp);
    }

    if (tilde
	&& (*name == '~')
	&& strlen(name) < sizeof(full)) {
	/* permit "~" from Bourne-shell */
	abshome(strcpy(full, name));
	name = full;
    }

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
int
dedscan(RING * gbl)
{
    int argc = gbl->top_argc;
    char **argv = gbl->top_argv;
    DIR *dp;
    DirentT *de;
    int j, k;
    unsigned n;
    int common = -1;
    char name[MAXPATHLEN];
    char *s;

    set_dedblip(gbl);
    gbl->flist = dedfree(gbl->flist, gbl->numfiles);
    dir_order = 0;
    gbl->numfiles = 0;

    if (argc > 1) {
	if (chdir(strcpy(gbl->new_wd, old_wd)) == 0) {
	    for (j = 0; j < argc; j++)
		if (ok_scan(gbl, argv[j])
		    && argstat(gbl, argv[j], TRUE, TRUE) >= 0)
		    common = 0;
	} else {
	    return (0);
	}
    } else {
	abshome(pathcat(gbl->new_wd, old_wd, argv[0]));
	if (!path_RESOLVE(gbl, gbl->new_wd)) {
	    return (0);
	}

	if ((common = argstat(gbl, gbl->new_wd, FALSE, FALSE)) > 0) {
	    /* mark dep's for purge */
	    if (gbl->toscan == NULL)
		ft_remove(gbl->new_wd, gbl->AT_opt, gbl->A_opt);
	    else
		init_scan(gbl);

	    set_dedblip(gbl);

	    if ((dp = opendir(".")) != NULL) {
		size_t len1;
		size_t len2;

		len1 = strlen(strcpy(name, gbl->new_wd));

		if (name[len1 - 1] != '/') {
		    name[len1++] = '/';
		    name[len1] = EOS;
		}
		while ((de = readdir(dp)) != NULL) {
		    s = de->d_name;
		    len2 = strlen(s);
		    if (len1 + len2 + 1 >= sizeof(name))
			continue;
		    if (*s == '.' && !gbl->A_opt)
			continue;
		    if (!ok_scan(gbl, s))
			continue;
		    j = argstat(gbl, strcpy(name + len1, s), TRUE, FALSE);
		    if (!dotname(s)
			&& j > 0
			&& lookup(gbl, s) >= 0) {
			ft_insert(name);
		    }
		}
		(void) closedir(dp);
		/*
		 * If nothing else, force "." to appear in the
		 * list.  This greatly simplifies the handling
		 * of empty directory lists!
		 */
		if (!gbl->numfiles) {
		    (void) argstat(gbl, ".", TRUE, FALSE);
		}
	    } else {
		waitmsg("cannot open directory");
		return (0);
	    }
	    if (gbl->toscan == NULL)
		ft_purge(gbl);	/* remove items not reinserted */
	} else if (common == N_FILE) {
	    s = fleaf(gbl->new_wd);
	    if (s != gbl->new_wd) {
		s[-1] = EOS;
	    }
	    argv[0] = txtalloc(gbl->new_wd);
	    common = (int) strlen(gbl->new_wd);
	    (void) argstat(gbl, s, TRUE, FALSE);
	}
    }

    /*
     * If the user specified in the command arguments a set of files from
     * multiple directories (or even a lot of files in the same directory)
     * find the longest common leading pathname component and readjust
     * everything if it is nonnull.
     */
    if (debug) {
	FPRINTF(debug_fp, "common=%d, numfiles=%u\r\n", common, gbl->numfiles);
	FFLUSH(debug_fp);
    }
    if (common == 0 && gbl->numfiles != 0) {
	unsigned comlen = (unsigned) strlen(strcpy(name, argv[0]));
	for (j = 0; (j < argc) && (comlen != 0); j++) {
	    char *d = argv[j];
	    unsigned slash = 0;

	    for (s = name, k = 0;
		 (d[k] == s[k]) && (d[k] != EOS);) {
		if ((d[k++] == '/')
		    && (d[k] != EOS))	/* need a leaf */
		    slash = (unsigned) k;	/* ...common-length */
	    }
	    if (slash < comlen) {
#ifdef	apollo
		if ((slash == 1)
		    && (name[0] == '/')) {
		    if ((s[1] == '/')
			|| (d[1] == '/'))
			slash = 0;	/* fix truncation */
		}
#endif
		comlen = slash;
	    }
	    dlog_comment("common '%.*s' (%d:%s)\n", comlen, name, slash, d);
	}
	name[comlen] = EOS;

	if (comlen != 0) {
	    dlog_comment("common path = \"%s\" (len=%d)\n",
			 name, comlen);
	    if (chdir(strcpy(gbl->new_wd, old_wd)) < 0)
		failed(old_wd);
	    abshome(strcpy(gbl->new_wd, name));
	    if (!path_RESOLVE(gbl, gbl->new_wd))
		failed(gbl->new_wd);
	    for_each_file(gbl, n)
		alloc_name(&gENTRY(n), gNAME(n) + comlen);
	} else {
	    size_t len = strlen(gbl->new_wd);
	    for (j = 0; j < argc && j < (int) gbl->numfiles; j++) {
		s = argv[j];
		if (s != NULL
		    && strlen(s) > len
		    && s[len] == '/'
		    && !strncmp(gbl->new_wd, s, len)) {
		    alloc_name(&gENTRY(j), s + len + 1);
		}
	    }
	}
    }
    if (debug)
	dedwait(gbl, FALSE);

    gbl->curfile = 0;
    dedsort(gbl);
    gbl->curfile = 0;		/* ensure consistent initial */

    return (int) (gbl->numfiles);
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

void
statSCCS(RING * gbl, const char *name, FLIST * f_)
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
		   && sameleaf(name, rcs_dir(NULL, NULL))) {
	    LAST(rcslast);
#endif /* Z_RCS */
	} else {
	    f_->z_lock =
		f_->z_vers = "";
	    f_->z_time = 0;
	}
    }
}
#endif /* Z_RCS_SCCS */

/*
 * This entrypoint is called to re-stat entries which already have been put
 * into the display-list.
 */
void
statLINE(RING * gbl, unsigned j)
{
    char *path = gNAME(j);
    FLIST *blok = &gENTRY(j);
    int flag = gFLAG(j);
    ORDER_T dord = gDORD(j);

    (void) dedstat(gbl, path, blok);
    gFLAG(j) = (char) flag;
    gDORD(j) = dord;
}

/*
 * For 'dedmake()', this adds a temporary entry, moving the former current
 * entry down, so the user can edit the name in-place.
 */
void
statMAKE(RING * gbl, mode_t mode)
{
    static char null[] = "";
    int x;

    if (mode) {
	FLIST dummy;
	unsigned n;

	Zero(&dummy);
	dummy.s.st_mode = mode;
	dummy.s.st_uid = getuid();
	dummy.s.st_gid = getgid();
	append(gbl, null, &dummy);

	if ((x = lookup(gbl, null)) >= 0
	    && ((n = (unsigned) x) != gbl->curfile)) {
	    FLIST save;
	    save = gENTRY(x);
	    if (n < gbl->curfile) {
		while (n++ < gbl->curfile)
		    gENTRY(n - 1) = gENTRY(n);
	    } else {
		while (n-- > gbl->curfile)
		    gENTRY(n + 1) = gENTRY(n);
	    }
	    cENTRY = save;
	}
    } else {			/* remove entry */
	if ((x = lookup(gbl, null)) >= 0) {
	    unsigned n = (unsigned) x;
	    while (n++ < gbl->numfiles)
		gENTRY(n - 1) = gENTRY(n);
	    gbl->numfiles--;
	}
    }
    showFILES(gbl, FALSE);
}

/*
 * Form a regular expression to match the wildcard pattern which the user
 * gave for a filename.
 */
static char *
make_EXPR(const char *path)
{
    char temp[MAXPATHLEN];
    const char *s = path;
    char *d = temp;

    *d++ = '^';
    while (*s) {
	if (ispunct(UCH(*s))) {
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
    *d = EOS;
    dlog_comment("force scan \"%s\"\n", path);
    return txtalloc(temp);
}

/*
 * Change to the given directory, and then (try to) get an absolute path by
 * using the 'getwd()' function.  Note that the latter may fail if we follow a
 * symbolic link into a directory in which we have execute, but no read-
 * access.  In this case we try to live with the link-text.
 */
int
path_RESOLVE(RING * gbl, char path[MAXPATHLEN])
{
    char temp[MAXPATHLEN];
    char *s;
    int is_dir = 1;
    Stat_t my_sb;
    static int tried;

    if ((strlen(path) < sizeof(temp)) &&
	chdir(strcpy(temp, path)) < 0) {
	is_dir = 0;
	if (errno == ENOTDIR
	    || errno == ENOENT) {
	    /*
	     * Try to find the parent directory, then, and make a
	     * pattern to match the leaf.  We'll do that only once,
	     * to handle unresolved wildcards from the command
	     * line.
	     */
	    s = strrchr(temp, PATH_SLASH);
	    if (s != NULL) {
		s[1] = EOS;
#ifdef	apollo
		if (strcmp(temp, "//"))
#endif /* apollo */
		    if (strcmp(temp, "/"))
			s[0] = EOS;	/* trim trailing '/' */
		/*
		 * If we've already got the parent directory in
		 * the ring, give up, removing this entry.
		 */
		if (ring_get(temp) != NULL) {
		    static char just_dot[] = ".";

		    warn(gbl, gbl->new_wd);
		    dedring(gbl, just_dot, 'Q', 1, FALSE, (char *) 0);
		    return (FALSE);
		}
		if (chdir(temp) < 0)
		    return (FALSE);
		if (first_scan && !tried++)
		    gbl->toscan = make_EXPR(path + (s - temp) + 1);
	    }
	} else {
	    warn(gbl, gbl->new_wd);
	    return (FALSE);
	}
    }
#if defined(HAVE_REALPATH)
    else {
	/* try to recover, just in case */
	if (chdir(old_wd) != 0)
	    return (FALSE);
    }
    s = realpath(path, temp);
    if (!is_dir) {
	strcpy(temp, pathhead(temp, &my_sb));
    }
#else
    s = getwd(temp);
#endif
    if (s != NULL) {
	if (chdir(strcpy(path, temp)) != 0) {
	    return (FALSE);
	}
    } else {			/* for SunOS? */
	FLIST fb;
	int save = gbl->AT_opt;
	int code;
	Zero(&fb);
	gbl->AT_opt = TRUE;
	code = dedstat(gbl, path, &fb);
	gbl->AT_opt = save;
	if (code == N_LDIR && fb.z_ltxt != NULL) {
	    (void) strcpy(path, fb.z_ltxt);
	}
    }
    return (TRUE);
}
