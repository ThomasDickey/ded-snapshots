/*
 * Title:	dedmsgs.c (directory-editor messages)
 * Author:	T.E.Dickey
 * Created:	07 Apr 1992, from 'ded.c'
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *
 * Function:	Display/maintain message in the last line of the screen
 */

#include	"ded.h"

MODULE_ID("$Id: dedmsgs.c,v 12.12 2010/07/04 19:41:22 tom Exp $")

/*
 * Clear the message-line
 */
void
clearmsg(void)
{
    move(LINES - 1, 0);
    clrtoeol();			/* clear off the waiting-message */
}

/*
 * Print an error/warning message, optionally pausing
 */
static void
show_message(RING * gbl, const char *tag, const char *msg)
{
    if (in_screen) {
	move(LINES - 1, 0);
	PRINTW("** %.*s", COLS - 4, msg);
	clrtoeol();
	if (gbl == 0) {
	    /* pause beside error message */
	    /* ...and clear it after pause */
	    move(LINES - 1, 0);
	    beep();
	    (void) dlog_char(gbl, (int *) 0, -1);
	    clrtoeol();
	} else
	    showC(gbl);
    } else {
	FPRINTF(stderr, "?? %s\n", msg);
    }
    dlog_comment("(%s) %s\n", tag, msg);
}

static const char *
err_msg(const char *msg)
{
    static char *bfr;
    char *text = strerror(errno);

    if (bfr == 0)
	bfr = malloc(BUFSIZ);
    if (bfr == 0)
	abort();
    if (msg == 0)
	msg = "?";
    FORMAT(bfr, "%s: %s", msg, text);
    return (bfr);
}

void
dedmsg(RING * gbl, const char *msg)
{
    show_message(gbl, "dedmsg", msg);
}

void
warn(RING * gbl, const char *msg)
{
    show_message(gbl, "warn", err_msg(msg));
}

/*
 * Wait for the user to hit a key before the next screen is shown.  This is
 * used when we have put a message up and may be going back to the
 * directory tree display.
 */
void
waitmsg(const char *msg)
{
    show_message((RING *) 0, "waitmsg", msg);
}

void
wait_warn(const char *msg)
{
    waitmsg(err_msg(msg));
}
