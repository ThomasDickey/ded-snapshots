#ifndef	lint
static	char	Id[] = "$Id: dedsigs.c,v 12.2 1993/09/28 12:21:22 dickey Exp $";
#endif

/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * Modified:
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

#define	SIG_PTYPES
#include	"ded.h"
#include	<signal.h>

static	int	caught;		/* counts number of interrupts */
static	int	init	= -1;	/* last-flag, to prevent redundant 'signal()' */

/*
 * Catch "intr" signals.
 */
static
SIGNAL_FUNC(catch)
{
	(void)signal (sig,  catch);
	beep();
	caught++;
}

static
SIGNAL_FUNC(dedquit)
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
int	dedsigs _ONE(int,flag)
{
	int	code	= caught;

	caught = 0;		/* reset interrupt-count */
	if (flag != init) {
		if (init < 0) {
			(void)signal (SIGHUP,  dedquit);
			(void)signal (SIGTERM, dedquit);
		}
		init = flag;
		(void)signal (SIGINT,  catch);
		if (flag)
			(void)signal(SIGQUIT, dedquit);
		else
			(void)signal(SIGQUIT, SIG_IGN);
	}
	return (code);		/* return number of interrupts we had */
}
