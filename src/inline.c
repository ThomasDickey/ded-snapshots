#ifndef	lint
static	char	Id[] = "$Id: inline.c,v 11.5 1992/08/12 16:21:36 dickey Exp $";
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

#define	C_LAST	-3
#define	C_QUIT	-2
#define	C_TRIM	-1
#define	C_NEXT	EOS

/************************************************************************
 *	local procedures						*
 ************************************************************************/
#ifdef	DEBUG
static	void	show(
	_ARX(char *,	tag)
	_ARX(int,	c)
	_ARX(size_t,	n)
	_AR1(char *,	text)
		)
	_DCL(char *,	tag)
	_DCL(int,	c)
	_DCL(size_t,	n)
	_DCL(char *,	text)
{
	char	temp_c[20],
		temp_t[BUFSIZ];
	char	*c2s, *text2s;

	switch (c) {
	case EOS:	c2s = "EOS ";	break;
	case C_TRIM:	c2s = "TRIM";	break;
	case C_QUIT:	c2s = "QUIT";	break;
	case C_LAST:	c2s = "LAST";	break;
	default:	encode_logch(c2s = temp_c, (int *)0, c & 0xff);
	}

	for (text2s = temp_t; *text; text++) {
		encode_logch(text2s, (int *)0, *text);
		text2s += strlen(text2s);
	}
	*text2s = EOS;

	dlog_comment("%s %-4.4s %d:%s\n", tag, c2s, n, temp_t);
}
#define	SHOW(tag,c,n,text)	show(tag,c,n,text);
#else
#define	SHOW(tag,c,n,text)
#endif

/************************************************************************
 *	public procedures						*
 ************************************************************************/

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
	static	DYN	*save;
	static	size_t	in_edit;
	register char	*lastedit;

	if (re_edit <= 0)
		refresh();

	save = dyn_alloc(save, in_edit+2);
	lastedit = dyn_string(save);

	SHOW("<", c, in_edit, lastedit)
	if (c != EOS) {
		switch (c) {
		case C_LAST:	/* report the last end-character */
			c = *lastedit;
			break;
		case C_QUIT:	/* remove all data (quit/abend) */
			in_edit = 1;
		case C_TRIM:	/* remove prior-data (e.g., for retry/append) */
			if (in_edit != 0) {
				c = lastedit[--in_edit];
				lastedit[in_edit] = EOS;
				save->cur_length  = in_edit;
			}
			break;
		default:	/* (re)start an editing-string */
			if (lastedit[0] != c) {
				lastedit[0] = c;
				lastedit[1] = EOS;
				save->cur_length = 1;
			}
			in_edit = 1;
		}
	} else {
		if (re_edit && lastedit[in_edit] != EOS) {
			c = lastedit[in_edit++];
		} else {
			c = dlog_char((int *)0,0);
			lastedit[in_edit++] = c;
			lastedit[in_edit]   = EOS;
			save->cur_length    = in_edit;
		}
	}

	SHOW(">", c, in_edit, lastedit)
	return (c & 0xff);
}
