#ifndef	lint
static	char	Id[] = "$Id: dedscan.c,v 8.0 1990/08/13 13:44:29 ste_cm Rel $";
#endif	lint

/*
 * Title:	dedscan.c (stat & scan argument lists)
 * Author:	T.E.Dickey
 * Created:	09 Nov 1987
 * $Log: dedscan.c,v $
 * Revision 8.0  1990/08/13 13:44:29  ste_cm
 * BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *
 *		Revision 7.3  90/08/13  13:44:29  dickey
 *		lint
 *		
 *		Revision 7.2  90/07/27  08:35:25  dickey
 *		modified 'argstat()' and 'dedstat()' so that 'dedstat()' does
 *		all of the stat-work.  Did this so that it calls 'statSCCS()'
 *		for all directory arguments, making 'l' command work properly.
 *		Also, this eliminates a redundant 'stat()' in 'argstat()'.
 *		
 *		Revision 7.1  90/05/16  08:14:49  dickey
 *		added code to strip prefixes which are common to the new-wd,
 *		but not among the other arguments.
 *		
 *		Revision 7.0  90/04/25  13:35:39  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.5  90/04/25  13:35:39  dickey
 *		corrected code which tries to circumvent unreadable old_wd
 *		(had broken the 'R' command in that fix...)
 *		
 *		Revision 6.4  90/04/25  08:18:54  dickey
 *		refined pathname-conversion using 'abshome()' followed by
 *		'getwd()' when needed, rather than 'abspath()' to avoid
 *		confusion with symbolic links.
 *		
 *		Revision 6.3  90/04/24  11:34:25  dickey
 *		modified 'argstat()' so that if shell (e.g., Bourne) passes
 *		a "~" argument, we expand it properly.
 *		
 *		Revision 6.2  90/04/23  13:55:06  dickey
 *		modify initial 'chdir()' so we try to recover from unreadable
 *		directory (e.g., when invoking "su" from a protected directory)
 *		
 *		Revision 6.1  90/04/18  07:40:54  dickey
 *		invoke 'rcslast()' to pick up information about permit-file
 *		(e.g., "RCS,v").
 *		
 *		Revision 6.0  89/10/16  08:28:32  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.0  89/10/16  08:28:32  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.4  89/10/16  08:28:32  dickey
 *		suppress 'ft_insert()' for "." and ".."
 *		
 *		Revision 4.3  89/10/12  09:21:02  dickey
 *		only fall-thru to 'sccslast()' if we found *nothing* of rcs.
 *		
 *		Revision 4.2  89/10/06  09:38:03  dickey
 *		modified interface to 'showFILES()'
 *		
 *		Revision 4.1  89/10/04  17:09:47  dickey
 *		added A_opt code (permit dot-names)
 *		
 *		Revision 4.0  89/06/06  09:01:28  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/06/06  09:01:28  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.6  89/06/06  09:01:28  dickey
 *		made read-pattern apply to explicit lists as well.
 *		
 *		Revision 2.5  89/06/06  08:32:48  dickey
 *		last change broke '@'-toggle; fixed.
 *		
 *		Revision 2.4  89/06/05  15:40:48  dickey
 *		simplified interface to ftree-module
 *		
 *		Revision 2.3  89/05/31  09:03:58  dickey
 *		added 'init_scan()' to fix regular-expression kludge
 *		
 *		Revision 2.2  89/05/26  14:20:57  dickey
 *		don't purge ftree if we are using read-expression
 *		
 *		Revision 2.1  89/05/26  13:14:42  dickey
 *		use read-selection (CTL/R command) to suppress unwanted names
 *		
 *		Revision 2.0  89/03/24  08:37:17  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
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
extern	FLIST	*dedfree();
extern	char	*pathcat();
extern	char	*txtalloc();

extern	int	debug;

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

	if (argc > 1) {
		(void)chdir(strcpy(new_wd,old_wd));
		for (j = 0; j < argc; j++)
			if (ok_scan(argv[j])
			&&  argstat(argv[j], TRUE) >= 0)
				common = 0;
	} else {
		abshome(pathcat(new_wd, old_wd, argv[0]));
		if (chdir(new_wd) < 0 || !getwd(new_wd)) {
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
					if (debug)
						PRINTF(" file \"%s\"\r\n", s);
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
#endif	apollo
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
			if (chdir(new_wd) < 0 || !getwd(new_wd))
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
#endif	S_IFLNK
#ifdef	Z_RCS_SCCS
	statSCCS(name, f_);
#endif	Z_RCS_SCCS
	return (code);
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
	char	full[BUFSIZ];
	int	code;

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
statSCCS(name, f_)
char	*name;
FLIST	*f_;
{
	if (Z_opt) {
		if (isFILE(f_->s.st_mode)) {
#ifdef	Z_RCS
			LAST(rcslast);
#ifdef	Z_SCCS
			if (f_->z_time == 0
			&&  f_->z_vers[0] == '?'
			&&  f_->z_lock[0] == '?')	/* fall-thru ? */
#endif	Z_SCCS
#endif	Z_RCS
#ifdef	Z_SCCS
			LAST(sccslast);
#endif	Z_SCCS
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
	showFILES(FALSE);
}
