#include	"ded.h"
extern	char	*re_comp();
extern	int	re_exec();

dedfind(key)
{
int	j,k,
	found	= FALSE,
	next	= 0;
char	text[BUFSIZ], *s;
static	int	order;		/* saves last legal search order */

	if (key == '/' || key == '?') {

		to_work();
		printw("Target: ");
		getyx(stdscr,j,k);
		clrtobot();
		move(j,k);
		refresh();

		resetty();
		getstr(text);
		rawterm();
		if ((s = re_comp(text)) == 0) {
			if (key == '/')	order = 1;
			if (key == '?') order = -1;
		} else {
			tell(s);
			return;
		}
		next = order;
	} else if (order) {
		if (key == 'n')	next = order;
		if (key == 'N')	next = -order;
	}

	if (next) {	/* we can do a search */
		for (j = curfile + next; ; j += next) {
			if (j < 0) {
				j = numfiles;
			} else if (j >= numfiles) {
				j = -1;
			} else {
				found = re_exec(flist[j].name);
				if (flist[j].ltxt != 0)
					found = re_exec(flist[j].ltxt);
				if (found)	break;
			}
			if (j == curfile)	break;
		}
		if (found) {
			markC(FALSE);
			curfile = j;
			if (to_file())
				showFILES();
			else
				showC();
		} else {
			tell("not found");
			return;
		}
	} else {
		beep();
		showC();
	}
}
