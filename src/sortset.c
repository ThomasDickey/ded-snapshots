#ifndef	lint
static	char	what[] = "$Id: sortset.c,v 9.3 1991/07/11 12:44:39 dickey Exp $";
#endif

/*
 * Title:	sortset.c (set sort-parms)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1989 (from ded.c)
 * Modified:
 *		28 Jun 1991, added 'P' (apollo sr10)
 *		11 Dec 1989, corrected call on 'dlog_char()'
 *		08 Dec 1989, added ':' special-sort to allow user to scroll
 *			     among all sort options before selecting.
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
#ifdef	apollo_sr10
	,"P - protection code, with extended-acl flag"
#endif
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
	if (strchr(sortc, opt) != 0) {
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

		to_work(TRUE);
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
					to_work(TRUE);
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
