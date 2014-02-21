#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <vector>
#include <map>
using std::map;
using std::vector;


struct connection
{
    u_int16_t remote_port ;
    u_int16_t local_port;
};                           //记录进程的连接的实例

struct process
{
    int pid;
    u_int32_t  len_in;
    u_int32_t  len_out;

    char *name;
    process(char* n,int p):pid(p),len_in(0),len_out(0){
		if(n)
		{
		//	name = new char [strlen(n)+1];
		//	strcpy(name,n);	
			name = n;
		}else
			name = NULL;
		pid =  p;
	}	
    ~process(){
		if(name)
			delete []name;
	}
};   // 标志每个有网络链接的进程



class process_manager
{
public:
	/*flush info of all process, construct all process has sockets,create the map from port to process,*/
    void reflush_port(class inodeport&);
	/*single instance*/
    static process_manager& GetInstance(){ static class process_manager s; return s;}
	/*return map of pid_process*/	
	const vector<struct process*> get_process_vec();
	/*flush the packet infomation into process*/
	void reflush_packet(struct packet* );

	
    void test(class inodeport& );


	u_int32_t lenin;
	u_int32_t lenout;
private:	
    ~process_manager();
    process_manager();
	process_manager& operator=(process_manager&); //disallowed
	process_manager(const process_manager&);//disallowed
	
	//get name of process by pid
    char* get_process_name(char * pid); 
	void get_info_for_pid(char * pid , class inodeport&); 
	void get_info_by_linkname ( char *pid, char* linkname, class inodeport&);
	
	
	/*pid to process*/
	map<int , struct process*> pidprocess;
	/*port to pid*/
	map<unsigned short, int> portpid;
	/*port to process*/
	map<unsigned short, struct process* >portprocess; 

};

#endif // PROCESS_H
