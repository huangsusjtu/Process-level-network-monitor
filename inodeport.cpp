#include "inodeport.h"

#include <cstdio>
#include <iostream>
#include <cstdlib>
using namespace std;


/*read /proc/net/{tcp,tcp6} to get (inode--port)*/
void inodeport::refreshinodeport()
{
	
	if (! add_procinfo ("/proc/net/tcp"))
	{
		//std::cout << "Error: couldn't open /proc/net/tcp\n";
		exit(0);
	}
	add_procinfo ("/proc/net/tcp6");
}

/* opens /proc/net/tcp[6] and adds its contents line by line */
int inodeport::add_procinfo (const char * filename) 
{
	FILE * procinfo = fopen (filename, "r");
	char buffer[4096];

	if (procinfo == NULL)
		return 0;
	
	fgets(buffer, sizeof(buffer), procinfo);

	do
	{
		if (fgets(buffer, sizeof(buffer), procinfo))
			add_inodeport(buffer);
	} while (!feof(procinfo));
	fclose(procinfo);
	return 1;
}

/*handle one line in file*/
void inodeport::add_inodeport (char * buffer)
{
	
	unsigned int local_port, rem_port;
	unsigned long inode;

	int matches = sscanf(buffer, "%*d: %*64[0-9A-Fa-f]:%04X %*64[0-9A-Fa-f]:%04X %*X %*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
		 &local_port, &rem_port, &inode);

	if (matches != 3) {
		//printf("Unexpected buffer: '%s'\n",buffer);
		exit(0);
	}
	
	if (inode == 0) {
		/* connection is in TIME_WAIT state. We rely on 
		 * the old data still in the table. */
		return;
	}

	inode_port[inode] = (unsigned short)local_port;
}

void inodeport::test()
{
	typedef std::map <unsigned long, unsigned short>::iterator IT;
	
	
	refreshinodeport();	
	IT it = inode_port.begin();
	while(it!=inode_port.end())
	{
		//cout<<it->first<<" "<<it->second<<endl;
		it++;
	}
	
}
