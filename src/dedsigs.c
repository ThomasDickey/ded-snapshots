#ifndef	lint
static	char	what[] = "$Header: /users/source/archives/ded.vcs/src/RCS/dedsigs.c,v 5.0 1989/05/10 14:51:14 ste_cm Rel $";
#endif	lint

/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * $Log: dedsigs.c,v $
 * Revision 5.0  1989/05/10 14:51:14  ste_cm
 * BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *
 *		Revision 4.0  89/05/10  14:51:14  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.0  89/05/10  14:51:14  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.1  89/05/10  14:51:14  dickey
 *		compiled on sun/sparc
 *		
 *		Revision 2.0  89/03/15  08:34:45  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
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
SIGS_T
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
