#ifndef	lint
static	char	sccs_id[] = "@(#)dedsigs.c	1.3 88/08/12 09:10:34";
#endif	lint

/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * Modified:
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
	to_exit(1);
	(void)printf("** quit **\n");
	(void)exit(1);
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
