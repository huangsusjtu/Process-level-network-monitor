#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string>
#include <algorithm>

#include "inodeport.h"
#include "process.h"
#include "packet_cap.h"

using namespace std;

unsigned long str2ulong (char * ptr) {//辅助函数
	unsigned long retval = 0;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		retval *= 10;
		retval += *ptr - '0';
		ptr++;
	}
	return retval;
}

bool isdigit(const char *name) //辅助函数
{
	size_t i =0;
	char l;
	while( (l=name[i]) != '\0')
	{
		if( l<'0' || l>'9')
			return false;
		i++;
	}
	return true;
}

char* itoa(int i) //辅助函数
{
	static char buf[10];
	int pos=0;
	memset(buf,0,10);
	while(i>0)
	{
		buf[pos++] = '0'+i%10;
		i /= 10;
	}
	i=0;pos--;
	while(i<pos)
	{char c=buf[i];buf[i]=buf[pos];buf[pos]=c;i++;pos--;}
	return buf;
}

process_manager::process_manager()
{}

process_manager::~process_manager()
{}


/*reflush the infomation of all process which has created socket*/
void process_manager::reflush_port(class inodeport& inodeport)  
{
	portprocess.clear();
	portpid.clear();

    DIR* open_process=opendir("/proc");
    struct dirent *h;
   
     while(NULL!=(h=readdir(open_process)))
        {

            if(strcmp(h->d_name,".")==0 || strcmp(h->d_name,".." )==0)
              {
                      continue;
              }
              if( isdigit(h->d_name))
              {                   
                  // cout<<h->d_name<<" !\n";
					
                   get_info_for_pid( h->d_name , inodeport);  //handle one process
              }
        }
         closedir(open_process);
}


/*read the process name by pid  in /proc/pid/cmdline*/
char* process_manager::get_process_name(char * pid) {
	#define prefix sizeof("/proc/")+sizeof("/cmdline")   

	int filenamelen = prefix + strlen(pid) + 1; 
	int bufsize = 80;
	char buffer[NAME_MAX+1];
	char *filename = (char *) malloc (filenamelen);
	snprintf (filename, filenamelen, "/proc/%s/cmdline", pid);
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		//printf ( "Error opening %s\n", filename);
		free (filename);
		exit(3);
		return NULL;
	}
	int length = read (fd, buffer, bufsize);
	if (close (fd)) {
		//std::cout << "Error closing file: "<< std::endl;
		exit(34);
	}
	free (filename);
	if (length < bufsize - 1)
		buffer[length]='\0';

	char * retval = buffer;

	return strdup(retval);
}

/*collect information of one process by read /proc/pid/fd/*/
void process_manager::get_info_for_pid(char * pid, class inodeport& inodeport) {
	size_t dirlen = 10 + strlen(pid);
	char * dirname = (char *) malloc (dirlen * sizeof(char));
	snprintf(dirname, dirlen, "/proc/%s/fd", pid);

	//std::cout << "Getting info for pid " << pid << std::endl;

	DIR * dir = opendir(dirname);

	if (!dir)
	{
		//std::cout << "Couldn't open dir " << dirname << ": "<< "\n";
		free (dirname);
		return;
	}

	/* walk through /proc/%s/fd/... */
	dirent * entry;
	while ((entry = readdir(dir))) {
		if (entry->d_type != DT_LNK)
			continue;
		//std::cout << "Looking at: " << entry->d_name << std::endl;

		int fromlen = dirlen + strlen(entry->d_name) + 1;
		char * fromname = (char *) malloc (fromlen * sizeof(char));
		snprintf (fromname, fromlen, "%s/%s", dirname, entry->d_name);

		//std::cout << "Linking from: " << fromname << std::endl;

		int linklen = 80;
		char linkname[linklen];
		int usedlen = readlink(fromname, linkname, linklen-1);
		if (usedlen == -1)
		{
			free (fromname);
			continue;
		}
		//assert (usedlen < linklen);
		linkname[usedlen] = '\0';
		//std::cout << "Linking to: " << linkname << std::endl;
		get_info_by_linkname (pid, linkname , inodeport);
		free (fromname);
	}
	closedir(dir);
	free (dirname);
}

/*read information in /proc/pid/fd/.  ,thus we know whether this process has created socket and the inode of socket*/
void process_manager::get_info_by_linkname (char * pid, char * linkname, class inodeport& inodeport) {  
	if (strncmp(linkname, "socket:[", 8) == 0) {
		char * ptr = linkname + 8;
		unsigned long inode = str2ulong(ptr);
		int p = atoi(pid);
		unsigned short port;
	
		/*the map from port to pid*/
		
		if( (port=inodeport.get_port_by_inode(inode) )!=0)
		{
			portpid[port] = p;
		}
	} else {
		//std::cout << "Linkname looked like: " << linkname << endl;
	}
}


void process_manager::reflush_packet(struct packet* list_packet)
{
	struct packet* pac = list_packet, *q;
	map<unsigned short, int >::iterator IT;
	map<int , struct process*>::iterator IT2;
	struct process* pro;
	int pid;
	unsigned short port ;

	for(map<int, struct process*>::iterator it=pidprocess.begin();it!=pidprocess.end();it++)
	{
		delete (it->second);
	}
    pidprocess.clear();
	this->lenin = this->lenout = 0;
	
	while(pac)
	{
		//static int c=0;
		//cout<<"Packet"<<c++;
		port = pac->local_port;
		if( (IT=portpid.find(port))!=portpid.end() )
		{
			
			pid = IT->second;
			//printf("Found pid %02x port %02x\n");
			if( (IT2=pidprocess.find(pid))==pidprocess.end())
			{
				pro = new process( get_process_name(itoa(pid)), pid);
				pidprocess[pid] = pro;
				portprocess[port] = pro;
			}else{
				pro = IT2->second;
			}

			if(pac->direct == IN)
			{	
				pro->len_in += pac->len;
				this->lenin += pac->len; 
			}
			else
			{
				pro->len_out += pac->len;
				this->lenout += pac->len;
			}
		}else{
			//printf("Not Found port %02x\n",port);
		}

		q = pac;
		pac	= pac->next;
		delete q;
	}
}

bool myfunction (struct process* i,struct process* j) { return (i->len_in+i->len_out)>(j->len_in+j->len_out); }
const vector<struct process*> process_manager::get_process_vec()
{
	vector<struct process*> res;
	map< unsigned short, struct process* >::iterator it1;
	for(it1=portprocess.begin();it1!=portprocess.end();it1++)
	{
		res.push_back(it1->second);
	} 	
	sort(res.begin(),res.end(),myfunction);
	return res;
}


void process_manager::test(inodeport& inodeport)
{
	reflush_port( inodeport);

	map<unsigned short, struct process* >::iterator it; 
	for(it=portprocess.begin();it!=portprocess.end();it++)
	{
		//cout<<"Port: "<< it->first <<" Process: "<< it->second << endl;
	} 
	map< int, struct process* >::iterator it1; 
	for(it1=pidprocess.begin();it1!=pidprocess.end();it1++)
	{
		//cout<<"Inode: "<<it1->first<<" Process: "<<it1->second<<endl;
	} 	

}




