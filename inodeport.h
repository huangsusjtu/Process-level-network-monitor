/*
	This class matains a map from inode to port, which used to construct relation between process and port.
*/

#ifndef CONNECTION
#define CONNECTION
#include <map>

class inodeport{
	inodeport(){}
	~inodeport(){}
	inodeport& operator=(inodeport&);
	inodeport(const inodeport& p);

	/*map from inode to port*/
	std::map <unsigned long, unsigned short> inode_port;  
	
	void add_inodeport (char * buffer);
	int add_procinfo (const char * filename);
public:
	static inodeport& GetInstance(){static inodeport s; return s;}
	void refreshinodeport();

	unsigned short get_port_by_inode(unsigned long a)
	{
		if(inode_port.find(a)!=inode_port.end()) 
			return inode_port[a];
		else 
			return 0;
	}
	void test();
private:

};



#endif
