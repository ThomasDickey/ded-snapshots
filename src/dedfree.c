/*
 * Title:	dedfree.c (free ded's main structure)
 * Author:	T.E.Dickey
 * Created:	02 May 1988
 * Modified:
 *		29 Jan 2001, support caseless filenames.
 *		29 Oct 1993, ifdef-ident
 *		18 Oct 1991, converted to ANSI
 *		15 May 1991, apollo sr10.3 cpp complains about tag on #endif
 *
 * Function:	Free storage used in the 'flist[]' array (or
 *		whatever copy of it that the called passes here).
 */

#include	"ded.h"

MODULE_ID("$Id: dedfree.c,v 12.5 2001/01/30 01:59:11 tom Exp $")

FLIST *
dedfree(
_ARX(FLIST *,	fp)
_AR1(unsigned,	num)
	)
_DCL(FLIST *,	fp)
_DCL(unsigned,	num)
{
	register unsigned j;

	if (fp != 0) {	/* we are rescanning display-list */
		for (j = 0; j < num; j++) {
			if (fp[j].z_name)	txtfree(fp[j].z_name);
			if (fp[j].z_ltxt)	txtfree(fp[j].z_ltxt);
#ifndef MIXEDCASE_FILENAMES
			if (fp[j].z_mono_name)	txtfree(fp[j].z_mono_name);
#endif
		}
		FREE((char *)fp);
	}
	return (0);
}
