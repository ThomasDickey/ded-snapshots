#ifndef	lint
static	char	what[] = "$Id: sortset.c,v 5.1 1989/12/01 14:54:54 dickey Exp $";
#endif	lint

/*
 * Title:	sortset.c (set sort-parms)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1989 (from ded.c)
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
	register int	j;

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
	}
	return (c);
}
