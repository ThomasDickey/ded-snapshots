/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * Modified:
 *		26 Jun 1994, catch INTR only when flag argument is TRUE.
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		18 Oct 1991, converted to ANSI.  Show signal-number in 'dedquit'
 *		11 Jun 1991, lint (apollo sr10.2)
 *		16 May 1991, mods to accommodate apollo sr10.3
 *		31 Oct 1989, fixed types for 'sun3'
 *		10 May 1989, compiled on sun/sparc
 *		15 Mar 1989, use 'dlog' module
 *		12 Aug 1988, in 'catch()' re-catch signal so this works on
 *			     system5.  Also, make 'SIGTERM' go to 'to_exit()'.
 *
 * Function:	Process signals for 'ded'.
 */

#include	"ded.h"

MODULE_ID("$Id: dedsigs.c,v 12.11 1995/07/04 14:07:26 tom Exp $")

static	int	caught;		/* counts number of interrupts */
static	int	init	= -1;	/* last-flag, to prevent redundant 'signal()' */

/*
 * Catch "intr" signals.
 */
private	SIGNAL_FUNC(catch)
{
	(void)signal (sig,  catch);
	beep();
	caught++;
}

private	SIGNAL_FUNC(dedquit)
{
	static	char	msg[] = "** quit **";
	auto	char	temp[BUFSIZ];

	(void)strcpy(temp, msg);
	if (sig != SIGQUIT)
		FORMAT(temp + strlen(temp), " (signal %d)", sig);
	(void)strcat(temp, "\n");

	to_exit(1);
	FPRINTF(stderr, temp);
	dlog_comment(temp);

	dlog_exit(FAIL);
}

/*
 * Process signals: we may catch interrupts, but try to clean up and exit if
 * we get a quit-signal.
 */
public	int	dedsigs (
	_AR1(int,	flag))
	_DCL(int,	flag)
{
	int	code	= caught;

	caught = 0;		/* reset interrupt-count */
	if (flag != init) {
		if (init < 0) {
			(void)signal (SIGHUP,  dedquit);
			(void)signal (SIGTERM, dedquit);
		}
		init = flag;
		if (flag) {
			(void)signal(SIGINT,  catch);
			(void)signal(SIGQUIT, dedquit);
		} else {
			(void)signal(SIGINT,  SIG_DFL);
			(void)signal(SIGQUIT, SIG_IGN);
		}
		enable_winch(flag);
	}
	return (code);		/* return number of interrupts we had */
}
