/*
 * Function:    Scan a list of arguments, to make up a display list.
 * Arguments:   argc, argv passed down from the original invocation, with leading
 *              options parsed off.
 */
#include        "ded.h"
#include        <sys/dir.h>
extern	void	free();

#ifdef	SYSTEM5
#define	DIR	FILE
#define	opendir(n)	fopen(n,"r")
#define	readdir(fp)	(fread(dbfr, sizeof(dbfr), 1, fp) ? &dbfr : (struct direct *)0)
#define	closedir(fp)	fclose(fp)
static	struct	direct	dbfr;
#endif	SYSTEM5

dedscan(argc, argv)
char    *argv[];
{
DIR		*dp;
struct	direct	*de;
register int     j;
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
			(void)statARG(argv[j], TRUE);
	} else {
		if (statARG(argv[0], FALSE) > 0) {
			if (chdir(strcpy(new_wd, argv[0])) < 0) {
				warn(new_wd);
				return(0);
			}
			getcwd(new_wd, sizeof(new_wd)-2);
			if (dp = opendir(".")) {
				while (de = readdir(dp)) {
					(void)statARG(strcpy(name, de->d_name), TRUE);
				}
				closedir(dp);
				if (!numfiles)
					tell("no files found");
			}
		}
	}
	return(numfiles);
}

static
char *
stralloc(d,s)
char	*d, *s;
{
	return (strcpy(doalloc(d, (unsigned)strlen(s)+1), s));
}

static
append(name, f_)
char	*name;
FLIST	*f_;
{
register int j;

	printf("."); fflush(stdout);
	for (j = 0; j < numfiles; j++) {
		if (!strcmp(flist[j].name, name)) {
			name          = flist[j].name;
			flist[j]      = *f_;
			flist[j].name = name;
			return;
		}
	}

	flist = (FLIST *)doalloc(flist, (numfiles+1) * sizeof(FLIST));

	flist[numfiles]      = *f_;
	flist[numfiles].name = stralloc((char *)0, name);
	flist[numfiles].flag = FALSE;
	numfiles++;
}

/*
 * Find the given file's stat-block.  Use the return-code to distinguish the
 * directories from ordinary files.  Save link-text for display, but don't
 * worry about whether a symbolic link really points anywhere real.
 */
static
dedstat (name, f_)
char	*name;
FLIST	*f_;
{
int	len;
char	bfr[BUFSIZ];

	if (lstat(name, &f_->s) < 0) {
		f_->s.st_mode = 0;
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
	return (0);
}

/*
 * Get statistics for a name which is in the original argument list.
 * We handle a special case: if a single argument is given (!list),
 * links are tested to see if they resolve to a directory.
 */
statARG(name, list)
char	*name;
{
FLIST	fb;
struct	stat	sb;

	fb.name = fb.ltxt = 0;
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

/*
 * This entrypoint is called to re-stat entries which already have been put
 * into the display-list.
 */
statLINE(j)
{
	(void)dedstat(flist[j].name, &flist[j]);
}
