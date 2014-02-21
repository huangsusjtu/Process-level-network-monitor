#ifndef CUI
#define CUI

#include <cstdlib>
#include <cstring>
#include <vector>
#include "process.h"
using std::vector;

struct line{
	int pid;
	char*  kbytein;
	char*  kbyteout;
	char *name;
	line(struct process &p)
	{
		pid = p.pid;
		kbytein = new char [32];
		kbyteout = new char [32];
		sprintf(kbytein, "%g kb/s",(double)p.len_in/1024);
		sprintf(kbyteout, "%g kb/s",(double)p.len_out/1024);
		name = strdup(p.name);
	}
	~line(){
		free(name);
		free(kbytein);
		free(kbyteout);
	}
};

class Cui
{
	char *caption;
	private:
		Cui(Cui& T){}
		Cui&  operator=(const Cui& T)const{}

		void show_line(const struct line& l, int row);
	public:
		static Cui& GetInstance(){
			static class Cui instance;
			return instance;
		}
		virtual void show(const vector<struct process*>& pvec );
		virtual void show_title();
		Cui();
		virtual ~Cui();
};

#endif
