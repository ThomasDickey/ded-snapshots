#ifndef	lint
static	char	*Id = "$Id: history.c,v 11.3 1992/08/25 15:57:10 dickey Exp $";
#endif

/*
 * Title:	history.c
 * Author:	T.E.Dickey
 * Created:	07 Aug 1992
 *
 * Function:	save/restore strings used for history of various ded commands.
 *		A history table is a linked list of strings, with no consecutive
 *		duplicates.  The overall length of the list is limited.
 */

#include "ded.h"

#define	def_alloc	HIST_alloc
	/*ARGSUSED*/
	def_ALLOC(HIST)

#define	MAX_AGE	10

#ifdef	DEBUG
private	void	show_history(
	_ARX(HIST *,	table)
	_AR1(char *,	tag)
		)
	_DCL(HIST *,	table)
	_DCL(char *,	tag)
{
	int	number	= 0;

	dlog_comment("history:%s\n",tag);
	while (table != 0) {
		char	temp_t[BUFSIZ],
			*text2s	= temp_t,
			*text	= table->text;

		while (*text) {
			encode_logch(text2s, (int *)0, *text++);
			text2s += strlen(text2s);
		}
		*text2s = EOS;
		dlog_comment("[%d] %s\n", number++, temp_t);
		table = table->next;
	}
}
#define	SHOW_HISTORY(table,tag)	show_history(table,tag)
#else
#define	SHOW_HISTORY(table,tag)
#endif

private	int	same_history(
	_ARX(HIST *,	table)
	_AR1(char *,	text)
		)
	_DCL(HIST *,	table)
	_DCL(char *,	text)
{
	if (table != 0)
		if (!strcmp(text, table->text))
			return TRUE;
	return FALSE;
}

public	void	put_history(
	_ARX(HIST **,	table)
	_AR1(char *,	text)
		)
	_DCL(HIST **,	table)
	_DCL(char *,	text)
{
	if (table != 0
	 && text  != 0
	 && *text != EOS
	 && !same_history(*table, text) ) {
		int	age = MAX_AGE;
		HIST	*new = ALLOC(HIST,1),
			*old = *table;

		new->next = old;
		new->text = stralloc(text);
		*table = new;

		/* don't let the table grow past maximum-age */
		while ((--age > 0) && new->next)
			new = new->next;

		if (old = new->next) {
			new->next = 0;
			dofree(old->text);
			dofree((char *)old);
		}
	}
	SHOW_HISTORY(*table,"put");
}


/*
 * Returns the string corresponding to the 'age' (indexed from 0), or null if
 * none exists.
 */
public	char *	get_history(
	_ARX(HIST *,	table)
	_AR1(int,	age)
		)
	_DCL(HIST *,	table)
	_DCL(int,	age)
{
	if (age < 0)
		return 0;
	while (age-- > 0) {
		if (table == 0)
			break;
		table = table->next;
	}
	return table ? table->text : 0;
}
