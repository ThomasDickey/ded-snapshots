#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: dedread.c,v 12.5 1994/06/30 23:41:45 tom Exp $";
#endif

/*
 * Title:	dedread.c (modify read-list expression)
 * Author:	T.E.Dickey
 * Created:	26 May 1989
 * Modified:
 *		23 Nov 1993, new blip-code.
 *		29 Oct 1993, ifdef-ident, port to HP/UX.
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		18 Oct 1991, converted to ANSI
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

public	int	dedread(
	_ARX(RING *,	gbl)
	_ARX(char **,	pattern_)
	_AR1(int,	change_needed)
		)
	_DCL(RING *,	gbl)
	_DCL(char **,	pattern_)
	_DCL(int,	change_needed)
{
	register char	*s;
	static	DYN	*text;
	static	HIST	*History;
	auto	REGEX_T	expr;

	set_dedblip(gbl);

	if (*pattern_)
		text = dyn_copy(text, *pattern_);
	else
		dyn_init(&text, BUFSIZ);

	if (!(s = dlog_string(gbl, "Pattern: ", &text,(DYN **)0, &History, EOS, 0)))
		s = "";
	if ((*pattern_ != 0) && !strcmp(s, *pattern_)) {
		showC(gbl);
		return (change_needed);
	} else if (!*s) {
		*pattern_ = 0;
		showC(gbl);
		if (gbl->used_expr) {
			OLD_REGEX(gbl->scan_expr);
			gbl->used_expr = FALSE;
		}
		return (TRUE);
	} else if (NEW_REGEX(expr,s)) {
		showC(gbl);
		*pattern_ = txtalloc(s);
		if (gbl->used_expr)
			OLD_REGEX(gbl->scan_expr);
		gbl->scan_expr = expr;
		gbl->used_expr = TRUE;
		return (TRUE);
	} else {
		BAD_REGEX(expr);
		showC(gbl);
		return (FALSE);
	}
}

/*
 * Initialize the match for regular-expression selection of files.  We need this
 * entrypoint because the BSD-style code does not save the compiled-expr.
 */
public	void	init_scan _ONE(RING *,gbl)
{
	if (gbl->toscan != 0) {
		dlog_comment("scan for \"%s\"\n", gbl->toscan);
		if (gbl->used_expr)
			OLD_REGEX(gbl->scan_expr);
		if (!(gbl->used_expr = NEW_REGEX(gbl->scan_expr,gbl->toscan))) {
			/* shouldn't happen */
			gbl->toscan = 0;
		}
	}
}

/*
 * Returns true if the given name was selectable by the current read-expression
 */
public	int	ok_scan (
	_ARX(RING *,	gbl)
	_AR1(char *,	name)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	name)
{
	if (gbl->toscan != 0)
		return (GOT_REGEX(gbl->scan_expr,name) != 0);
	return (TRUE);
}
