#ifndef	lint
static	char	sccs_id[] = "@(#)dedsigs.c	1.1 88/08/03 08:42:20";
#endif	lint

/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * Modified:
 *
 * Function:	Process signals for 'ded'.
 */

#include	<signal.h>

static	int	caught;		/* counts number of interrupts */
static	int	init	= -1;	/* last-flag, to prevent redundant 'signal()' */

/*
 * Catch "intr" signals.
 */
static
int
catch()
{
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
		init = flag;
		(void)signal (SIGINT,  catch);
		(void)signal (SIGQUIT, flag ? dedquit : catch);
	}
	return (code);		/* return number of interrupts we had */
}
