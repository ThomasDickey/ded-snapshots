#ifndef	lint
static	char	Id[] = "$Id: dedsigs.c,v 8.1 1991/05/16 07:58:48 dickey Exp $";
#endif

/*
 * Title:	dedsigs.c (catch/ignore signals)
 * Author:	T.E.Dickey
 * Created:	03 Aug 1988
 * $Log: dedsigs.c,v $
 * Revision 8.1  1991/05/16 07:58:48  dickey
 * mods to accommodate apollo sr10.3
 *
 *		Revision 8.0  89/10/31  09:00:55  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.0  89/10/31  09:00:55  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  89/10/31  09:00:55  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.1  89/10/31  09:00:55  dickey
 *		fixed types for 'sun3'
 *		
 *		Revision 5.0  89/05/10  14:51:14  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
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

#define	SIG_PTYPES
#include	"ded.h"
#include	<signal.h>

static	int	caught;		/* counts number of interrupts */
static	int	init	= -1;	/* last-flag, to prevent redundant 'signal()' */

/*
 * Catch "intr" signals.
 */
static
void	catch(sig)
{
	(void)signal (sig,  catch);
	beep();
	caught++;
}

static
void	dedquit()
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
		if (flag)
			(void)signal(SIGQUIT, dedquit);
		else
			(void)signal(SIGQUIT, SIG_IGN);
	}
	return (code);		/* return number of interrupts we had */
}
