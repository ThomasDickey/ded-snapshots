#ifndef	NO_IDENT
static char Id[] = "$Id: dedcmds.c,v 12.2 2004/03/07 23:25:18 tom Exp $";
#endif

/*
 * Title:	dedcmds.c (ded commands)
 * Author:	T.E.Dickey
 * Created:	19 Jul 1994
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *
 * Function:	This module contains support for internal commands that
 *		are issued to the directory editor.
 */
#include "ded.h"

#if 0
#define	C_EDIT	1
#define	C_LIST	2
#define	C_TREE	4
#define	C_BOTH	C_LIST|C_TREE

static void
do_bind(void)
{
}

static DEDCMDS commands[] =
{
    {"backward-delete-word", C_EDIT, CTL('W'), nofunc},
    {"beginning-of-line", C_EDIT, CTL('B'), nofunc},
    {"bind", C_BOTH, 0, do_bind},
    {"edit-in-process", C_BOTH, 'E', edit_in_process},
    {"edit-new-process", C_BOTH, 'e', edit_new_process},
    {"end-of-line", C_EDIT, CTL('F'), nofunc},
    {"execute-dot", C_LIST, '.', execute_dot},
    {"quit", C_BOTH, 'q', do_quit},
    {"reset", C_BOTH, 0, do_reset},
    {"set", C_BOTH, 0, do_set},
    {"toggle-hidden-files", C_BOTH, '&', toggle_hidden},
    {"unset", C_BOTH, 0, do_unset},
};

void
dedcmds(void)
{
}
#endif
