#ifndef	lint
static	char	what[] = "$Id: sortset.c,v 8.0 1990/03/06 08:24:51 ste_cm Rel $";
#endif	lint

/*
 * Title:	sortset.c (set sort-parms)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1989 (from ded.c)
 * $Log: sortset.c,v $
 * Revision 8.0  1990/03/06 08:24:51  ste_cm
 * BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *
 *		Revision 7.0  90/03/06  08:24:51  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.0  90/03/06  08:24:51  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.4  90/03/06  08:24:51  dickey
 *		lint
 *		
 *		Revision 5.3  89/12/11  09:15:20  dickey
 *		corrected call on 'dlog_char()'
 *		
 *		Revision 5.2  89/12/08  10:23:00  dickey
 *		added ':' special-sort to allow user to scroll among all
 *		sort options before selecting.
 *		
 *
 * Function:	Set sort-argument for ded, encapsulating knowledge of the
 *		particular sort-keys available (see also 'dedsort.c')
 */

#include	"ded.h"

char	sortc[128];

static	char	*sort_msg[] = {
	 ". - lengths of dot-separated items"
	,"c - ctime (chmod time)"
	,"d - directory-order"
	,"g - group"
	,"G - group (numeric)"
	,"i - inode"
	,"l - number of links"
	,"n - name"
	,"N - name (excluding path, if any)"
#ifdef	apollo_sr10
	,"o - object type-uid (symbolic)"
	,"O - object type-uid (numeric)"
#endif	/* apollo_sr10 */
	,"p - protection code"
	,"r - atime (access time)"
	,"s - size (bytes)"
	,"t - suffix/type (after first '.')"
	,"T - suffix/type (after last '.')"
	,"u - user"
	,"U - user (numeric)"
	,"w - by mtime (modification)"
#ifdef	Z_RCS_SCCS
	,"y - RCS/SCCS lock-owner"
	,"v - RCS/SCCS tip-version"
	,"z - RCS/SCCS checkin date"
	,"Z - RCS/SCCS checkin date over modification dates"
#endif	/* Z_RCS_SCCS */
	};

#define	LOOP(j)	for (j = 0; j < sizeof(sort_msg)/sizeof(sort_msg[0]); j++)

sortset(ord,opt)
{
	if (*sortc == EOS) {
		register int	j, k = 0;
		LOOP(j)
			sortc[k++] = *sort_msg[j];
		sortc[k] = EOS;
	}
	if (strchr(sortc, (size_t)opt) != 0) {
		dateopt = opt == 'c'  ? 1 : (opt == 'r' ? 0 : 2);
		sortopt = opt;
		sortord = (ord == 'r');
		return(TRUE);
	}
	return(FALSE);
}

/*
 * This function is called from the main command-loop to permit the user
 * to review the current sorting mode (by '?'), or to re-sort with the
 * present direction (with newline or return).
 */
sortget(c)
{
	auto	char	bfr[80];
	register int	j, k;

	if (c == '?') {
		LOOP(j)
			if (*sort_msg[j] == sortopt) {
				FORMAT(bfr, "sort option: %s", sort_msg[j]);
				dedmsg(bfr);
				break;
			}
		c = 0;
	} else if (c == '\r' || c == '\n') {
		c = sortopt;
	} else if (c == ':') {
		auto	int	y,x,
				done = FALSE,
				find, found;

		to_work();
		PRINTW("Sort:> ");
		getyx(stdscr,y,x);
		find = sortopt;
		while (!done) {
			found = FALSE;
			LOOP(k) {
				if (*sort_msg[k] == find) {
					c = *strcpy(bfr, sort_msg[k]);
					found = TRUE;
					break;
				}
			}
			if (found) {
				j = k;
				move(y,x);
				PRINTW("%s", bfr);
				clrtobot();
			} else {
				dedmsg("up/down keys=scroll, return=select");
				beep();
			}
			move(y,x-2);
			refresh();
			k = strlen(sortc) - 1;
			switch (found = dlog_char((int *)0, 0)) {
			case ARO_UP: 	if (--j < 0)	j = k;
					find = *sort_msg[j];	break;
			case ARO_DOWN:	if (++j > k)	j = 0;
					find = *sort_msg[j];	break;
			case 'q':
					c = 0;
					to_work();
					showC();
					/* fall-thru */
			case '\r':
			case '\n':	done = TRUE;		break;
			default:	find = found;
			}
		}
	}
	return (c);
}
