#ifndef	lint
static	char	Id[] = "$Id: inline.c,v 11.1 1992/08/11 17:03:28 dickey Exp $";
#endif

/*
 * Title:	inline.c (directory-editor inline editor-strings)
 * Author:	T.E.Dickey
 * Created:	11 Aug 1992 (from 'dedline.c')
 * Modified:
 *
 * Function:	Procedures which manage in-line editing of particular fields
 *		of the file-list.
 */

#include	"ded.h"

static	int	re_edit;		/* flag for 'edittext()' */

/*
 * Disable refresh temporarily while replaying inline-editing.
 */
public	void	hide_inline(
	_AR1(int,	flag))
	_DCL(int,	flag)
{
	re_edit = flag;
}


/*
 * Initiate/conclude repetition of inline editing.
 */
public	int	edit_inline _ONE(int,flag)
{
	return ((re_edit = flag) ? ReplayEndC() : 0);
}

/*
 * Store/retrieve field-editing commands.  The first character of the buffer
 * 'lastedit[]' is reserved to tell us what the command was.
 */
public	int	get_inline _ONE(int,c)
{
	static	char	lastedit[BUFSIZ];
	static	int	in_edit;

	if (re_edit <= 0)
		refresh();

	if (c != EOS) {
		switch (c) {
		case -3:	/* report the last end-character */
			c = *lastedit;
			break;
		case -2:	/* remove all data (quit/abend) */
			in_edit = 1;
		case -1:	/* remove prior-data (e.g., for retry/append) */
			if (in_edit > 0) {
				c = lastedit[--in_edit];
				lastedit[in_edit] = EOS;
			}
			break;
		default:
			lastedit[0] = c;
			in_edit = 1;
		}
	} else {
		if (re_edit && lastedit[in_edit] != EOS) {
			c = lastedit[in_edit++];
		} else {
			c = dlog_char((int *)0,0);
			if (c == '\r') c = '\n';
			lastedit[in_edit++] = c;
			lastedit[in_edit]   = EOS;
		}
	}

	return (c & 0xff);
}
