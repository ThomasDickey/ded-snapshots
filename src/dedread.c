#ifndef	lint
static	char	Id[] = "$Id: dedread.c,v 9.1 1991/07/11 12:52:51 dickey Exp $";
#endif

/*
 * Title:	dedread.c (modify read-list expression)
 * Author:	T.E.Dickey
 * Created:	26 May 1989
 * Modified:
 *		11 Jul 1991, interface to 'to_work()'
 *		18 Apr 1991, added flag to control whether identical pattern
 *			     returns true or false (so that if nothing is found,
 *			     we can force re-invocation of this procedure).
 *		23 May 1990, make the pattern to be set an argument
 *		
 *
 * Function:	Modifies the 'toscan' value, which controls the selection of
 *		files for display.
 *
 * patch:	should accept a blank-separated list for multiple expressions
 */
#include	"ded.h"
extern	char	*txtalloc();

dedread(pattern_, change_needed)
char	**pattern_;
{
	register int	j,k;
	auto	char	text[BUFSIZ], *expr;

	to_work(TRUE);
	PRINTW("Pattern: ");
	getyx(stdscr,j,k);
	clrtobot();
	move(j,k);
	refresh();

	if (*pattern_)
		(void)strcpy(text, *pattern_);
	else
		*text = EOS;

	dlog_string(text,sizeof(text),FALSE);
	if ((*pattern_ != 0) && !strcmp(text, *pattern_)) {
		showC();
		return (change_needed);
	} else if (!*text) {
		*pattern_ = 0;
		showC();
		OLD_REGEX(scan_expr);
		return (TRUE);
	} else if (NEW_REGEX(expr,text)) {
		showC();
		*pattern_ = txtalloc(text);
		OLD_REGEX(scan_expr);
		scan_expr = expr;
		return (TRUE);
	} else {
		BAD_REGEX(expr);
		showC();
		return (FALSE);
	}
}

/*
 * Initialize the match for regular-expression selection of files.  We need this
 * entrypoint because the BSD-style code does not save the compiled-expr.
 */
init_scan()
{
	if (toscan != 0) {
		dlog_comment("scan for \"%s\"\n", toscan);
		OLD_REGEX(scan_expr);
		if (!NEW_REGEX(scan_expr,toscan)) {	/* shouldn't happen */
			BAD_REGEX(scan_expr);
			toscan = 0;
		}
	}
}

/*
 * Returns true if the given name was selectable by the current read-expression
 */
ok_scan(name)
char	*name;
{
	if (toscan != 0)
		return (GOT_REGEX(scan_expr,name) != 0);
	return (TRUE);
}
