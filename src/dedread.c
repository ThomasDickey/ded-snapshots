#ifndef	lint
static	char	sccs_id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/dedread.c,v 2.2 1989/05/31 09:09:21 dickey Exp $";
#endif	lint

/*
 * Title:	dedread.c (modify read-list expression)
 * Author:	T.E.Dickey
 * Created:	26 May 1989
 * Modified:
 *
 * Function:	Modifies the 'toscan' value, which controls the selection of
 *		files for display.
 *
 * patch:	should accept a blank-separated list for multiple expressions
 */
#include	"ded.h"
extern	char	*txtalloc();

dedread()
{
	register int	j,k;
	auto	char	text[BUFSIZ], *expr;

	to_work();
	PRINTW("Pattern: ");
	getyx(stdscr,j,k);
	clrtobot();
	move(j,k);
	refresh();

	if (toscan)
		(void)strcpy(text, toscan);
	else
		*text = EOS;

	dlog_string(text,sizeof(text),FALSE);
	if (!strcmp(text, toscan)) {
		showC();
		return (FALSE);	/* no change */
	} else if (!*text) {
		toscan = 0;
		showC();
		OLD_REGEX(scan_expr);
		return (TRUE);
	} else if (NEW_REGEX(expr,text)) {
		showC();
		toscan = txtalloc(text);
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
