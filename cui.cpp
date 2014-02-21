#include <ncurses.h>
#include "cui.h"

Cui::Cui()
{
	WINDOW * screen = initscr();
	raw();
	noecho();
	cbreak();
	nodelay(screen, TRUE);
	caption = "PLNM";
}

Cui::~Cui()
{
	clear();
	endwin();
}

void Cui::show_title()
{
	clear();
	mvprintw (0, 0, "%s", caption);
	attron(A_REVERSE);
	mvprintw (2, 0, "  PID     PROGRAM                       SENT      RECEIVED       ");
	attroff(A_REVERSE);
}
void Cui::show_line(const struct line& l, int row)
{
	mvprintw(3+row, 3, "%d  ", l.pid);
	mvprintw(3+row, 3+6,  "  %s  ", l.name);
	mvprintw(3+row, 6+22+9, "  %s  ", l.kbyteout);
	mvprintw(3+row, 6+22+9+9, "  %s  ", l.kbytein);
}

void Cui::show(const vector<struct process*>& pvec)
{
	int c = pvec.size();
	for(int i = 0;i<c;i++)
	{
		struct line l( *pvec[i] );
		show_line(l, i);
	}
	refresh();
}






