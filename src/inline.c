#ifndef	lint
static	char	Id[] = "$Id: inline.c,v 11.11 1992/08/13 14:35:05 dickey Exp $";
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

#define	ITEM	struct	_item
typedef	ITEM	{
	ITEM *	link;
	int	topc,	/* top-character, if nested */
		endc;	/* inline-edit toggle character */
	size_t	play;	/* index into 'text' of playback */
	DYN *	text;	/* the text to play/record */
	HIST *	hist;	/* prior copies of 'text' */
	};

#define	def_alloc	ITEM_alloc
	/*ARGSUSED*/
	def_ALLOC(ITEM)

static	int	re_edit;		/* flag for 'edittext()' */

/************************************************************************
 *	local procedures						*
 ************************************************************************/
#ifdef	DEBUG
static	void	show_text(
	_ARX(int,	c)
	_ARX(int,	cmd)
	_ARX(int,	play)
	_AR1(char *,	text)
		)
	_DCL(int,	c)
	_DCL(int,	cmd)
	_DCL(int,	play)
	_DCL(char *,	text)
{
	char	temp_c[20],
		*c2s;

	switch (cmd) {
	case C_FIND:	c2s = "FIND";	break;
	case C_INIT:	c2s = "INIT";	break;
	case C_NEXT:	c2s = "NEXT";	break;
	case C_TRIM:	c2s = "TRIM";	break;
	case C_QUIT:	c2s = "QUIT";	break;
	case C_TOPC:	c2s = "TOPC";	break;
	case C_ENDC:	c2s = "ENDC";	break;
	default:	c2s = isascii(cmd) ? "PLAY" : "?";
	}
	c2s = strcat(strcpy(temp_c, c2s), " ");
	encode_logch(c2s + strlen(c2s), (int *)0, c & 0xff);

	dlog_comment("%c%-8.8s %d:%s\n",
		re_edit == TRUE
			? '+'
			: ((re_edit == -TRUE)
				? '-'
				: ' '),
		c2s, play, text);
}

private	void	show_item(
	_ARX(int,	c)
	_ARX(int,	cmd)
	_AR1(ITEM *,	item)
		)
	_DCL(int,	c)
	_DCL(int,	cmd)
	_DCL(ITEM *,	item)
{
	char	temp_t[BUFSIZ],
		*text2s	= temp_t,
		*text	= dyn_string(item->text);

	if (item->topc)	*text2s++ = item->topc;
	if (item->endc)	*text2s++ = item->endc;
	*text2s++ = ':';

	while (*text) {
		encode_logch(text2s, (int *)0, *text++);
		text2s += strlen(text2s);
	}
	*text2s = EOS;

	show_text(c,cmd, item->play, temp_t);
}

#define	SHOW(c,cmd,item)	show_item(c,cmd,item);
#define	SHOW2(c,cmd)		show_text(c,cmd,0,"");
#else
#define	SHOW(c,cmd,item)
#define	SHOW2(c,cmd)
#endif

static
ITEM *	find_item(
	_ARX(int,	topc)
	_AR1(int,	endc)
		)
	_DCL(int,	topc)
	_DCL(int,	endc)
{
	static	ITEM	*items;
	register ITEM	*p;

	for (p = items; p != 0; p = p->link)
		if (p->topc == topc
		 && p->endc == endc)
			break;

	if (p == 0) {
		p = ALLOC(ITEM,1);
		p->link = items;
		p->topc = topc;
		p->endc = endc;
		p->play = 0;
		p->text = dyn_alloc((DYN *)0, 1);
		p->hist = 0;
		items = p;
	}
	return p;
}

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
 * is reserved to tell us what the command was.
 */
public	int	get_inline (
	_ARX(int,	c)
	_AR1(int,	cmd)
		)
	_DCL(int,	c)
	_DCL(int,	cmd)
{
	static	int	my_topc, my_endc;
	register ITEM *	p;

	if (re_edit <= 0)
		refresh();

	switch (cmd) {
	case C_TOPC:
		SHOW2(c,cmd)
		my_topc = c;
		return;
	case C_FIND:
	case C_INIT:
		my_endc = c;
	}

	if (!my_endc && !isascii(cmd)) {
		failed("get_inline: no endc defined");
	}

	p = find_item(my_topc, my_endc);
	p->text = dyn_alloc(p->text, p->play+2);

	switch (cmd) {
	case C_FIND:
		break;

	case C_INIT:
		dyn_init(&(p->text), 1);
		break;

	case C_ENDC:	/* report the last end-character */
		c = p->endc;
		break;

	case C_QUIT:	/* remove all data (quit/abend) */
		p->play = 0;
		/* fall-thru */

	case C_TRIM:	/* remove prior-data (e.g., for retry/append) */
		if (p->play != 0)
			p->play -= 1;

		p->text->cur_length = p->play;
		c  = dyn_string(p->text)[p->play];
		dyn_string(p->text)[p->play] = EOS;
		break;

	case C_NEXT:
		if (re_edit
		 && p->play < dyn_length(p->text)) {
			c = dyn_string(p->text)[p->play];
			p->play += 1;
		} else {
			if (re_edit == TRUE) {
				re_edit = FALSE;
				refresh();
			}
			c = dlog_char((int *)0,0);
			p->text = dyn_append_c(p->text, c);
			p->play = dyn_length(p->text);
		}
		break;

	default:	/* (re)start an editing-string */
		if (my_endc != c)
			p = find_item(my_topc, my_endc = c);
		p->play = 0;
	}

	SHOW(c, cmd, find_item(my_topc, my_endc))
	return (c & 0xff);
}
