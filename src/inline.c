/*
 * Title:	inline.c (directory-editor inline editor-strings)
 * Author:	T.E.Dickey
 * Created:	11 Aug 1992 (from 'dedline.c')
 * Modified:
 *		07 Mar 2004, remove K&R support, indent'd
 *		29 Oct 1993, ifdef-ident
 *		28 Sep 1993, gcc warnings
 *
 * Function:	Procedures which manage in-line editing of particular fields
 *		of the file-list.
 */

#include	"ded.h"

MODULE_ID("$Id: inline.c,v 12.9 2004/09/08 01:03:31 tom Exp $")

#define	ITEM	struct	_item
ITEM {
    ITEM *link;
    int topc;			/* top-character, if nested */
    int endc;			/* inline-edit toggle character */
    size_t play;		/* index into 'text' of playback */
    DYN *text;			/* the text to play/record */
    HIST *hist;			/* prior copies of 'text' */
};

static DYN *edited;
static int re_edit,		/* flag for replay/editing */
  the_age,			/* index into history */
  my_topc,			/* optional key for items */
  my_endc;			/* required key for items */

/************************************************************************
 *	local procedures						*
 ************************************************************************/
int
dyn_trim1(DYN * p)
{
    int c;
    if (dyn_length(p)) {
	char *temp = dyn_string(p);
	p->cur_length -= 1;
	c = temp[p->cur_length];
	temp[p->cur_length] = EOS;
    } else
	c = EOS;
    return c;
}

#ifdef	DEBUG
static void
show_text(int c, int cmd, int play, char *text)
{
    char temp_c[20], *c2s;

    switch (cmd) {
    case C_ENDC:
	c2s = "ENDC";
	break;
    case C_DONE:
	c2s = "DONE";
	break;
    case C_FIND:
	c2s = "FIND";
	break;
    case C_INIT:
	c2s = "INIT";
	break;
    case C_NEXT:
	c2s = "NEXT";
	break;
    case C_QUIT:
	c2s = "QUIT";
	break;
    case C_TOPC:
	c2s = "TOPC";
	break;
    case C_TRIM:
	c2s = "TRIM";
	break;
    default:
	c2s = isascii(cmd) ? "PLAY" : "?";
    }
    c2s = strcat(strcpy(temp_c, c2s), " ");
    encode_logch(c2s + strlen(c2s), (int *) 0, c & 0xff);

    dlog_comment("%c%-8.8s %d:%s\n",
		 re_edit == TRUE
		 ? '+'
		 : ((re_edit == -TRUE)
		    ? '-'
		    : ' '),
		 c2s, play, text);
}

static void
show_item(int c, int cmd, ITEM * item)
{
    char temp_t[BUFSIZ], *text2s = temp_t, *text = dyn_string(item->text);

    if (item->topc)
	*text2s++ = item->topc;
    if (item->endc)
	*text2s++ = item->endc;
    *text2s++ = ':';

    while (*text) {
	encode_logch(text2s, (int *) 0, *text++);
	text2s += strlen(text2s);
    }
    *text2s = EOS;

    show_text(c, cmd, item->play, temp_t);
}

#define	SHOW(c,cmd,item)	show_item(c,cmd,item)
#define	SHOW2(c,cmd)		show_text(c,cmd,0,"")
#else
#define	SHOW(c,cmd,item)
#define	SHOW2(c,cmd)
#endif

static ITEM *
find_item(void)
{
    static ITEM *items;
    ITEM *p;

    if (!my_endc)
	failed("get_inline: no endc defined");

    for (p = items; p != 0; p = p->link)
	if (p->topc == my_topc
	    && p->endc == my_endc)
	    break;

    if (p == 0) {
	p = ALLOC(ITEM, 1);
	p->link = items;
	p->topc = my_topc;
	p->endc = my_endc;
	p->play = 0;
	p->text = dyn_alloc((DYN *) 0, 1);
	p->hist = 0;
	items = p;
    }
    return p;
}

static int
redo_item(char *s)
{
    ITEM *p = find_item();
    p->text = dyn_copy(p->text, s);
    p->play = 0;
    re_edit = TRUE;
    return TRUE;
}

/************************************************************************
 *	public procedures						*
 ************************************************************************/

/*
 * Disable refresh temporarily while replaying inline-editing.
 */
void
hide_inline(int flag)
{
    re_edit = flag;
}

/*
 * Initiate/conclude repetition of inline editing.
 */
int
edit_inline(int flag)
{
    return ((re_edit = flag) ? my_endc : 0);
}

/*
 * Push/pop history for inline editing that cannot be done via 'dlog_string()'
 */
#define	IGNORE	{ beep(); return FALSE; }

int
up_inline(void)
{
    ITEM *p = find_item();
    char *s, *t = dyn_string(p->text);

    (void) dyn_trim1(p->text);

    if (!the_age)
	edited = dyn_copy(edited, t);

    if ((s = get_history(p->hist, the_age)) != NULL) {
	if (strcmp(s, t)) ;	/* cannot skip */
	else if ((s = get_history(p->hist, the_age + 1)) != NULL)
	    the_age++;
	else
	    IGNORE		/* last and only item */
		the_age++;
    } else
	IGNORE

	    return redo_item(s);
}

int
down_inline(void)
{
    ITEM *p = find_item();
    char *s;

    (void) dyn_trim1(p->text);

    if (the_age <= 0)
	IGNORE

	    if ((s = get_history(p->hist, the_age - 2)) != NULL) {
	    the_age--;
	    if (the_age == 1 && !strcmp(s, dyn_string(edited)))
		the_age = 0;
	} else if (the_age == 1) {
	    the_age = 0;
	    s = dyn_string(edited);
	} else
	    IGNORE

		return redo_item(s);
}

/*
 * Store/retrieve field-editing commands.  The first character of the buffer
 * is reserved to tell us what the command was.
 */
int
get_inline(RING * gbl, int c, int cmd)
{
    ITEM *p;

    if (re_edit <= 0)
	refresh();

    switch (cmd) {
    case C_TOPC:
	SHOW2(c, cmd);
	my_topc = c;
	return EOS;
    case C_FIND:
    case C_INIT:
	my_endc = c;
	the_age = 0;
    }

    p = find_item();
    p->text = dyn_alloc(p->text, p->play + 2);

    switch (cmd) {
    case C_FIND:
	break;

    case C_INIT:
	dyn_init(&(p->text), 1);
	break;

    case C_DONE:		/* save buffer in history */
	edited = dyn_copy(edited, dyn_string(p->text));
	(void) dyn_trim1(edited);
	put_history(&(p->hist), dyn_string(edited));
	break;

    case C_ENDC:		/* report the last end-character */
	c = p->endc;
	break;

    case C_QUIT:		/* remove all data (quit/abend) */
	p->play = 0;
	/* fall-thru */

    case C_TRIM:		/* remove prior-data (e.g., for retry/append) */
	c = dyn_trim1(p->text);
	p->play = dyn_length(p->text);
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
	    c = dlog_char(gbl, (int *) 0, 0);
	    p->text = dyn_append_c(p->text, c);
	    p->play = dyn_length(p->text);
	}
	break;

    default:			/* (re)start an editing-string */
	if (my_endc != c) {
	    my_endc = c;
	    p = find_item();
	}
	p->play = 0;
    }

    SHOW(c, cmd, find_item());
    return (c & 0xff);
}

DYN **
inline_text(void)
{
    ITEM *p = find_item();
    return &(p->text);
}

HIST **
inline_hist(void)
{
    ITEM *p = find_item();
    return &(p->hist);
}

#ifdef	DEBUG
int
inline_hidden(void)
{
    return re_edit;
}
#endif
