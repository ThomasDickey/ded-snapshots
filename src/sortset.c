#if	!defined(NO_IDENT)
static	char	Id[] = "$Id: sortset.c,v 12.5 1993/12/01 16:30:56 dickey Exp $";
#endif

/*
 * Title:	sortset.c (set sort-parms)
 * Author:	T.E.Dickey
 * Created:	01 Dec 1989 (from ded.c)
 * Modified:
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		01 Apr 1992, convert most global variables to RING-struct.
 *		18 Oct 1991, converted to ANSI
 *		17 Jul 1991, added '@', 'D' sorts
 *		28 Jun 1991, added 'P' (apollo sr10)
 *		11 Dec 1989, corrected call on 'dlog_char()'
 *		08 Dec 1989, added ':' special-sort to allow user to scroll
 *			     among all sort options before selecting.
 *
 * Function:	Set sort-argument for ded, encapsulating knowledge of the
 *		particular sort-keys available (see also 'dedsort.c')
 */

#include	"ded.h"

public	char	sortc[128];

static	char	*sort_msg[] = {
	 ". - lengths of dot-separated items"
#ifdef	S_IFLNK
	,"@ - symbolic-link targets"
#endif	/* S_IFLNK */
	,"c - ctime (chmod time)"
	,"d - directory-order"
	,"D - device-code"
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

#define	LOOP(j)	for (j = 0; j < SIZEOF(sort_msg); j++)

public	int	sortset(
	_ARX(RING *,	gbl)
	_ARX(int,	ord)
	_AR1(int,	opt)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	ord)
	_DCL(int,	opt)
{
	if (*sortc == EOS) {
		register int	j, k = 0;
		LOOP(j)
			sortc[k++] = *sort_msg[j];
		sortc[k] = EOS;
	}
	if (strchr(sortc, opt) != 0) {
		gbl->dateopt = (opt == 'c') ? 1 : (opt == 'r' ? 0 : 2);
		gbl->sortopt = opt;
		gbl->sortord = (ord == 'r');
		return(TRUE);
	}
	return(FALSE);
}

/*
 * This function is called from the main command-loop to permit the user
 * to review the current sorting mode (by '?'), or to re-sort with the
 * present direction (with newline or return).
 */
public	int	sortget(
	_ARX(RING *,	gbl)
	_AR1(int,	c)
		)
	_DCL(RING *,	gbl)
	_DCL(int,	c)
{
	auto	char	bfr[80];
	register int	j, k;

	if (c == '?') {
		LOOP(j)
			if (*sort_msg[j] == gbl->sortopt) {
				FORMAT(bfr, "sort option: %s", sort_msg[j]);
				dedmsg(gbl, bfr);
				break;
			}
		c = 0;
	} else if (c == '\n') {
		c = gbl->sortopt;
	} else if (c == ':') {
		auto	int	y,x,
				done = FALSE,
				find, found;

		to_work(gbl,TRUE);
		PRINTW("Sort:> ");
		getyx(stdscr,y,x);
		find = gbl->sortopt;
		j = 0;
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
				dedmsg(gbl, "up/down keys=scroll, return=select");
				beep();
			}
			move(y,x-2);
			k = strlen(sortc) - 1;
			switch (found = dlog_char((int *)0, 0)) {
			case ARO_UP: 	if (--j < 0)	j = k;
					find = *sort_msg[j];	break;
			case ARO_DOWN:	if (++j > k)	j = 0;
					find = *sort_msg[j];	break;
			case 'q':
					c = 0;
					to_work(gbl,TRUE);
					showC(gbl);
					/* fall-thru */
			case '\n':	done = TRUE;		break;
			default:	find = found;
			}
		}
	}
	return (c);
}
