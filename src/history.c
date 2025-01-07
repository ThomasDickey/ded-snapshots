/*
 * Title:	history.c
 * Author:	T.E.Dickey
 * Created:	07 Aug 1992
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		09 Feb 1996, increase history limit.
 *		23 Jul 1994, added 'show_history()'
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *		28 Aug 1992, if caller puts history item which is repeated,
 *			     simply move it to the front of the list.
 *
 * Function:	save/restore strings used for history of various ded commands.
 *		A history table is a linked list of strings, with no duplicates.
 *		The overall length of the list is limited.
 */

#include "ded.h"

MODULE_ID("$Id: history.c,v 12.9 2025/01/07 01:22:02 tom Exp $")

#define	MAX_AGE	20

#ifdef	DEBUG
static void
dump_history(HIST * table, char *tag)
{
    int number = 0;

    dlog_comment("history:%s\n", tag);
    while (table != 0) {
	char temp_t[BUFSIZ], *text2s = temp_t, *text = table->text;

	while (*text) {
	    encode_logch(text2s, (int *) 0, *text++);
	    text2s += strlen(text2s);
	}
	*text2s = EOS;
	dlog_comment("[%d] %s\n", number++, temp_t);
	table = table->next;
    }
}

#define	DUMP_HISTORY(table,tag)	dump_history(table,tag)
#else
#define	DUMP_HISTORY(table,tag)
#endif

static int
same_history(HIST * table, char *text)
{
    if (table != NULL)
	if (!strcmp(text, table->text))
	    return TRUE;
    return FALSE;
}

void
put_history(HIST ** table, char *text)
{
    if (table != NULL
	&& text != NULL
	&& *text != EOS) {
	HIST *p, *q;

	for (p = *table, q = NULL; p != NULL; q = p, p = p->next)
	    if (same_history(p, text))
		break;

	if (p != NULL) {	/* relink the entry to make it first */

	    if (q != NULL) {
		q->next = p->next;
		p->next = *table;
		*table = p;
	    } else if (p != *table) {
		p->next = *table;
		*table = p;
	    }

	} else {		/* allocate a new entry */
	    int age = MAX_AGE;

	    p = ALLOC(HIST, 1);
	    q = *table;

	    p->next = q;
	    p->text = stralloc(text);
	    *table = p;

	    /* don't let the table grow past maximum-age */
	    while ((--age > 0) && p->next)
		p = p->next;

	    if ((q = p->next) != NULL) {
		p->next = NULL;
		dofree(q->text);
		dofree((char *) q);
	    }
	}
	DUMP_HISTORY(*table, "put");
    }
}

/*
 * Returns the string corresponding to the 'age' (indexed from 0), or null if
 * none exists.
 */
char *
get_history(HIST * table, int age)
{
    if (age < 0)
	return NULL;
    while (age-- > 0) {
	if (table == NULL)
	    break;
	table = table->next;
    }
    return table ? table->text : NULL;
}

/*
 * Displays the command-history for a given filelist.  The first item is always
 * the command that's cached with the RING structure.
 */
void
show_history(RING * gbl, int depth)
{
    HIST *table = cmd_history;
    char temp[20];
    int shown = 1;

    dedshow(gbl, "Command=", dyn_string(gbl->cmd_sh));
    while ((table != NULL) && (shown < depth)) {
	if (strcmp(table->text, dyn_string(gbl->cmd_sh))) {
	    FORMAT(temp, "%d %c ", ++shown, gbl->clr_sh ? '%' : '!');
	    dedshow2(temp);
	    dedshow2(table->text);
	    dedshow2("\n");
	}
	table = table->next;
    }
    showC(gbl);
}
