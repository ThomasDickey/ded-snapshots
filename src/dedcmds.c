#ifndef	NO_IDENT
static	char	Id[] = "$Id: dedcmds.c,v 12.1 1994/07/19 23:34:44 tom Exp $";
#endif

/*
 * Title:	dedcmds.c (ded commands)
 * Author:	T.E.Dickey
 * Created:	19 Jul 1994
 *
 * Function:	This module contains support for internal commands that
 *		are issued to the directory editor.
 */
#include "ded.h"

#define	C_EDIT	1
#define	C_LIST	2
#define	C_TREE	4
#define	C_BOTH	C_LIST|C_TREE

private	void	do_bind(_AR0)
{
}

static	DEDCMDS	commands[] = {
{"backward-delete-word",	C_EDIT,	CTL('W'),	nofunc},
{"beginning-of-line",		C_EDIT,	CTL('B'),	nofunc},
{"bind",			C_BOTH,	0,		do_bind},
{"edit-in-process",		C_BOTH,	'E',		edit_in_process},
{"edit-new-process",		C_BOTH,	'e',		edit_new_process},
{"end-of-line",			C_EDIT,	CTL('F'),	nofunc},
{"execute-dot",			C_LIST,	'.',		execute_dot},
{"quit",			C_BOTH,	'q',		do_quit},
{"reset",			C_BOTH,	0,		do_reset},
{"set",				C_BOTH,	0,		do_set},
{"toggle-hidden-files",		C_BOTH,	'&',		toggle_hidden},
{"unset",			C_BOTH,	0,		do_unset},
};

public	void	dedcmds(_AR0)
{
}
