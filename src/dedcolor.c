/*
 * Title:	dedcolor.c (ded color support)
 * Author:	T.E.Dickey
 * Created:	10 Jul 1994
 * Modified:
 *		09 Aug 1999, allow color names to be mixed case, in any order.
 *		09 Feb 1996, allow ISO 6429 codes to be used on non-Linux.
 *		16 Dec 1995, mods to override curses's sense of default color.
 *
 * Function:	If we've got SYS5-curses and appropriate display hardware,
 *		we can show colors in a curses application. This application
 *		uses the Linux-style /etc/DIR_COLORS file to describe colors
 *		that apply to various filetypes and filenames.
 *
 * Note:	This was written/debugged on a Linux system using ncurses.
 */
#include "ded.h"

MODULE_ID("$Id: dedcolor.c,v 12.15 2001/01/30 01:04:52 tom Exp $")

#if HAVE_HAS_COLORS

enum	ColorBy { ByType, ByMode, BySuffix };

typedef	struct	{
	char	*name;
	enum ColorBy code;
	char	*value;
	} KEYWORD;

#define	KEYATTR	struct	key_pair
	KEYATTR	{
	KEYATTR	*next;
	KEYWORD	key;		/* data used to match file to color */
	int	attr;		/* the attributes and color-pair value */
	};

	int	invert_colors;

static	KEYATTR	*keypairs;
static	char *	color_file;
static	int	initialized;
static	int	default_foreground = COLOR_WHITE;
static	int	default_background = COLOR_BLACK;

/*
 * Initialize a color pair, if there's room in curses' table. Return the
 * index value if successful, otherwise -1.
 */
private	int	CreatePair(
		_ARX(int,	foreground)
		_AR1(int,	background)
			)
		_DCL(int,	foreground)
		_DCL(int,	background)
{
	static	int	used_pairs;	/* # of entries we've used so far */
	int	n;

	if (foreground == default_foreground
	 && background == default_background)
	 	return 0;

	for (n = 1; n <= used_pairs; n++) {
		short	forg	= -1,
			bakg	= -1;
		pair_content(n, &forg, &bakg);
		if (forg == foreground
		 && bakg == background)
		 	return n;
	}
	if (used_pairs < COLOR_PAIRS-1) {
		used_pairs++;
		init_pair(used_pairs, foreground, background);
		return used_pairs;
	}
	return -1;
}

/*
 * The DIR_COLORS file on Linux specifies colors in a manner that is specific
 * to the ISO 6429.  Colors and attributes are given by a series of numbers
 * separated by semicolons.  We translate these codes back to a form that is
 * usable in [n]curses.
 */
private	void	SaveColor(
		_ARX(const KEYWORD *,	name)
		_AR1(char *,		spec)
			)
		_DCL(KEYWORD *,	name)
		_DCL(char *,	spec)
{
	static	const	struct	{
		char	*name;
		int	code;
	} attr_names[] = {
		{"UNDERLINE",	A_UNDERLINE},
		{"REVERSE",	A_REVERSE},
		{"DIM",		A_DIM},
		{"INVIS",	A_INVIS},
		{"BLINK",	A_BLINK},
		{"BOLD",	A_BOLD}
	}, color_names[] = {
		{"BLACK",	COLOR_BLACK},
		{"RED",		COLOR_RED},
		{"GREEN",	COLOR_GREEN},
		{"YELLOW",	COLOR_YELLOW},
		{"BLUE",	COLOR_BLUE},
		{"MAGENTA",	COLOR_MAGENTA},
		{"CYAN",	COLOR_CYAN},
		{"WHITE",	COLOR_WHITE}
	};
	int	code;
	int	attr	= A_NORMAL,
		forg	= default_foreground,
		bakg	= default_background;
	size_t	n;
	int	found = FALSE;
	char	*temp;

	/* patch: how can I get the values for color-pair #0? */

	strlwrcpy(spec,spec);	/* simplify case-independent matches */
	/* build up the attribute+color */
	while (spec != 0 && *spec != EOS) {
		char *next = strchr(spec, ';');
		if (next != 0) {
			*next++ = EOS;
			(void)strclean(spec);
		}
		code = strtol(spec, &temp, 10);
		found = FALSE;
		if (temp != spec) {	/* there's a number */
			found = TRUE;
			switch(code) {
			/* attributes */
			case  1:	attr |= A_BOLD;		break;
			case  2:	attr |= A_DIM;		break;
			case  4:	attr |= A_UNDERLINE;	break;
			case  5:	attr |= A_BLINK;	break;
			case  7:	attr |= A_REVERSE;	break;
			case  8:	attr |= A_INVIS;	break;
			/* text (foreground) color */
			case 30:	forg = COLOR_BLACK;	break;
			case 31:	forg = COLOR_RED;	break;
			case 32:	forg = COLOR_GREEN;	break;
			case 33:	forg = COLOR_YELLOW;	break;
			case 34:	forg = COLOR_BLUE;	break;
			case 35:	forg = COLOR_MAGENTA;	break;
			case 36:	forg = COLOR_CYAN;	break;
			case 37:	forg = COLOR_WHITE;	break;
			/* background color codes */
			case 40:	bakg = COLOR_BLACK;	break;
			case 41:	bakg = COLOR_RED;	break;
			case 42:	bakg = COLOR_GREEN;	break;
			case 43:	bakg = COLOR_YELLOW;	break;
			case 44:	bakg = COLOR_BLUE;	break;
			case 45:	bakg = COLOR_MAGENTA;	break;
			case 46:	bakg = COLOR_CYAN;	break;
			case 47:	bakg = COLOR_WHITE;	break;
			}
		} else {	/* non-number: keywords */
			for (n = 0; n < SIZEOF(attr_names); n++) {
				if (!strucmp(spec, attr_names[n].name)) {
					attr |= attr_names[n].code;
					found = TRUE;
					break;
				}
			}
		}
		if (!found
		 && (*spec == 'f' || *spec == 'b')
		 && (temp = strchr(spec, '=')) != 0) {
			temp++;
			for (n = 0; n < SIZEOF(color_names); n++) {
				if (!strucmp(temp, color_names[n].name)) {
					if (*spec == 'f')
						forg = color_names[n].code;
					else
						bakg = color_names[n].code;
					break;
				}
			}
		}
		spec = next;
	}

	/* build the list so that the later entries will override the first */
	if ((code = CreatePair(forg, bakg)) >= 0) {
		KEYATTR	*p = (KEYATTR *)doalloc((char *)0, sizeof(KEYATTR));
		p->key = *name;
		p->attr = attr | COLOR_PAIR(code);
		p->next = keypairs;
		keypairs = p;
	}
}

/* lookup a predefined keyword for the DIR_COLORS file */
private	const	KEYWORD	*FindKeyword(
		_AR1(char *,	name))
		_DCL(char *,	name)
{
	static	const
	KEYWORD keywords [] = {
	{"NORMAL", ByType,	"?"},	/* global default */
	{"FILE",   ByType,	"-"},	/* normal file */
	{"DIR",    ByType,	"d"},	/* directory */
	{"LINK",   ByType,	"l"},	/* symbolic link */
	{"FIFO",   ByType,	"p"},	/* pipe */
	{"SOCK",   ByType,	"s"},	/* socket */
	{"BLK",    ByType,	"b"},	/* block device driver */
	{"CHR",    ByType,	"c"},	/* character device driver */
	{"EXEC",   ByMode,	"X"},	/* executable */
	/* these are my extensions (cf: 'access_mode()'): */
	{"WRITE",  ByMode,	"W"},	/* writeable */
	{"READ",   ByMode,	"R"},	/* readable */
	{"RW",     ByMode,	"RW"},	/* readable/writeable */
	{"RWX",    ByMode,	"RWX"}	/* readable/writeable */
	};
	size_t	n;

	for (n = 0; n < SIZEOF(keywords); n++)
		if (!strucmp(keywords[n].name, name))
			return &(keywords[n]);
	return 0;
}

/* scan the DIR_COLORS file to get all color information */
private	void	ParseColorFile (_AR0)
{
	FILE	*fp;
	char	bfr[BUFSIZ], *s;
	const	KEYWORD	*p;

	if ((fp = fopen(color_file, "r")) != 0) {
		while (fgets(bfr, sizeof(bfr), fp) != 0) {
			if ((s = strchr(bfr, '#')) != 0)
				*s = EOS;
			if (!strclean(bfr))
				continue;	/* only a comment */
			if ((s = strchr(bfr, ' ')) == 0)
				s = strchr(bfr, '\t');
			if (s != 0)	/* we've got an argument */
				*s++ = EOS;
			else
				continue;	/* ignore this error */
			if ((p = FindKeyword(bfr)) != 0) {
				SaveColor(p, s);
			} else if (*bfr == '.') {
				static	KEYWORD temp;
				temp.name = txtalloc("SUFFIX");
				temp.code = BySuffix;
				temp.value = txtalloc(bfr);
				SaveColor(&temp, s);
			}
			/* else, ignore keyword */
		}
		FCLOSE(fp);
	}
}

/*
 * Look for the color-file in one of the following locations:
 *	~/.ded_colors
 *	~/.dir_colors
 *	/etc/DIR_COLORS
 */
private	int	FindColorFile(
		_ARX(char *,	path)
		_AR1(char *,	leaf)
			)
		_DCL(char *,	path)
		_DCL(char *,	leaf)
{
	Stat_t	sb;
	char	temp[MAXPATHLEN];
	if (stat_file(pathcat(temp, path, leaf), &sb) >= 0) {
		color_file = txtalloc(temp);
		return TRUE;
	}
	color_file = 0;
	return FALSE;
}

private	void	InitializeColors (_AR0)
{
	initialized = TRUE;

	if (invert_colors) {
		default_background = COLOR_WHITE | A_BOLD;
		default_foreground = COLOR_BLACK;
		init_pair(0, default_foreground, default_background);
	}

	/* find the color-definition file */
	if (FindColorFile(gethome(), ".ded_colors")
	 || FindColorFile(gethome(), ".dir_colors")
	 || FindColorFile("/etc", "DIR_COLORS"))
	 	ParseColorFile();
}

/* returns true if the file is at least as accessible as the pattern */
private	int	AtLeastAccessible(
		_ARX(char *,	pattern)
		_AR1(Stat_t *,	sb)
			)
		_DCL(char *,	pattern)
		_DCL(Stat_t *,	sb)
{
	char	temp[8], *match = temp;

	if (ded_access(sb, S_IREAD))	*match++ = 'R';
	if (ded_access(sb, S_IWRITE))	*match++ = 'W';
	if (ded_access(sb, S_IEXEC))	*match++ = 'X';
	*match = EOS;

	/*
	 * Note: I tried to code this using 'access_mode()', but ran into a bug
	 * in gcc (or the linker) on Linux that caused DED to crash with a
	 * segmentation violation (?).
	 */
	for (match = temp; *match != EOS; match++)
		if (*match == *pattern)
			pattern++;
	return (*pattern == EOS);
}

private	int	AttributesOf(
		_AR1(FLIST *,	entry))
		_DCL(FLIST *,	entry)
{
	KEYATTR	*p;
	Stat_t	*sb	= &(entry->s);
	char	*suffix = ftype2(entry->z_name);
	int	attr	= A_NORMAL;

	for (p = keypairs; p != 0; p = p->next) {
		switch (p->key.code) {
		case ByType:
			if (p->key.value[0] == modechar(sb->st_mode))
				return p->attr;
			if (p->key.value[0] == '?')
				attr = p->attr;
			break;
		case ByMode:
			if (isFILE(sb->st_mode)
			 && AtLeastAccessible(p->key.value, sb))
				return p->attr;
			break;
		case BySuffix:
			if (!strcmp(p->key.value, suffix))
				return p->attr;
		}
	}
	return attr;
}

public	void	dedcolor(
		_AR1(FLIST *,	entry))
		_DCL(FLIST *,	entry)
{
	int	attr = A_NORMAL;	/* default, resets color */

	if (!initialized)
		InitializeColors();
	if (entry != 0)		/* set color according to filename & type */
		attr = AttributesOf(entry);

	attrset(attr);
}

public	void	init_dedcolor (_AR0)
{
	(void)start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (use_default_colors() == OK) {
		default_foreground = -1;
		default_background = -1;
	}
#endif
}
#endif	/* HAVE_HAS_COLORS */
