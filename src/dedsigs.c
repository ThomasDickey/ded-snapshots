#ifndef	lint
static	char	sccs_id[] = "$Header: /users/source/archives/ded.vcs/src/RCS/dedsigs.c,v 2.0 1989/03/15 08:34:45 ste_cm Exp $";
#endif	lint

/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * $Log: dedsigs.c,v $
 * Revision 2.0  1989/03/15 08:34:45  ste_cm
 * BASELINE Thu Apr  6 13:14:13 EDT 1989
 *
 *		Revision 1.5  89/03/15  08:34:45  dickey
 *		sccs2rcs keywords
 *		
 *		15 Mar 1989, use 'dlog' module
 *		12 Aug 1988, in 'catch()' re-catch signal so this works on
 *			     system5.  Also, make 'SIGTERM' go to 'to_exit()'.
 *
 * Function:	Process signals for 'ded'.
 */

#include	"ded.h"
#include	<signal.h>

static	int	caught;		/* counts number of interrupts */
static	int	init	= -1;	/* last-flag, to prevent redundant 'signal()' */

/*
 * Catch "intr" signals.
 */
static
int
catch(sig)
{
	(void)signal (sig,  catch);
	beep();
	caught++;
}

static
int
dedquit()
{
	static	char	msg[] = "** quit **\n";
	to_exit(1);
	FPRINTF(stderr, msg);
	dlog_comment(msg);
	dlog_exit(FAIL);
}

/*
 * Process signals: we may catch interrupts, but try to clean up and exit if
 * we get a quit-signal.
 */
dedsigs(flag)
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
		(void)signal (SIGQUIT, flag ? dedquit : SIG_IGN);
	}
	return (code);		/* return number of interrupts we had */
}
