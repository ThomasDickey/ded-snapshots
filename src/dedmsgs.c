#ifndef	lint
static	char	Id[] = "$Id: dedmsgs.c,v 12.0 1992/04/07 07:41:26 ste_cm Rel $";
#endif

/*
 * Title:	dedmsgs.c (directory-editor messages)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *
 * Function:	Display/maintain message in the last line of the screen
 */

#include	"ded.h"

/*
 * Clear the message-line
 */
public	void	clearmsg(_AR0)
{
	move(LINES-1,0);
	clrtoeol();		/* clear off the waiting-message */
}

/*
 * Print an error/warning message, optionally pausing
 */
private	void	show_message(
	_ARX(RING *,	gbl)
	_ARX(char *,	tag)
	_AR1(char *,	msg)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	tag)
	_DCL(char *,	msg)
{
	if (in_screen) {
		move(LINES-1,0);
		PRINTW("** %.*s", COLS-4, msg);
		clrtoeol();
		if (gbl == 0) {
			/* pause beside error message */
			/* ...and clear it after pause */
			move(LINES-1,0);
			refresh();
			beep();
			(void)dlog_char((int *)0,-1);
			clrtoeol();
		} else
			showC(gbl);
	} else {
		FPRINTF(stderr, "?? %s\n", msg);
	}
	dlog_comment("(%s) %s\n", tag, msg);
}

private	char *	err_msg _ONE(char *,msg)
{
	static	char	bfr[BUFSIZ];
	if (msg == 0)	msg = "?";
	FORMAT(bfr, "%s: %s", msg, sys_errlist[errno]);
	return (bfr);
}

public	void	dedmsg(
	_ARX(RING *,	gbl)
	_AR1(char *,	msg)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	msg)
{
	show_message(gbl, "dedmsg", msg);
}

public	void	warn(
	_ARX(RING *,	gbl)
	_AR1(char *,	msg)
		)
	_DCL(RING *,	gbl)
	_DCL(char *,	msg)
{
	show_message(gbl, "warn", err_msg(msg));
}

/*
 * Wait for the user to hit a key before the next screen is shown.  This is
 * used when we have put a message up and may be going back to the
 * directory tree display.
 */
waitmsg		_ONE(char *,msg) { show_message((RING *)0, "waitmsg", msg); }
wait_warn	_ONE(char *,msg) { waitmsg(err_msg(msg)); }