#include "packet_cap.h"

const size_t SnapLen = max(sizeof(ppp_header),sizeof(ethhdr) ) + max(sizeof(ip6_hdr),sizeof(ip)) + max(sizeof(tcphdr),sizeof(udphdr));

pcap::pcap()
{
	pthread_mutex_init(&lock_packet,NULL);
	this->list_packet = NULL;
}

pcap::~pcap()
{
	for(unsigned int i=0;i<handles.size();i++)
	{	
		if(handles[i].handle)
			pcap_close(handles[i].handle);
	}	
	packet *p;

	pthread_mutex_lock(&lock_packet);
	while(list_packet)
	{
		p = list_packet;
		list_packet = list_packet->next;
		delete p;
	}
	pthread_mutex_unlock(&lock_packet);
	pthread_mutex_destroy(&lock_packet);
}


struct packet* pcap::get_packet_list()
{
	pthread_mutex_lock(&lock_packet);
	return list_packet;
}           //operations of struct packet* list_packet
void pcap::set_packet_list(struct packet* p)
{
 	list_packet = p;
	if(!p)
		lenin= lenout = 0;
	pthread_mutex_unlock(&lock_packet);
}



bool pcap::init()
{
	open_devices();
	return get_local_ip();	
}


void pcap::open_devices()
{
	int res;
	pcap_if_t *it,*tmp;
	pcap_t *handle;
	struct HANDLE h;
	this->handles.clear();
	res=pcap_findalldevs(&it,errbuf);
	
	if(res < 0)
	{
		//printf("Find devs err : %s\n",errbuf);
		exit(-1);
	}
	tmp = it;
	while(tmp)
	{
		if(strcmp("any",tmp->name)==0 || strcmp("nflog",tmp->name)==0 || strcmp("nfqueue",tmp->name)==0 || strcmp("lo",tmp->name)==0)
		{
			tmp = tmp->next;
			continue;
		}

		handle = pcap_open_live(tmp->name, SnapLen, 0, 100, errbuf);
	 	if (handle == NULL) {
			//fprintf(stderr, "Couldn't open device %s: %s\n", tmp->name, errbuf);
	 	}else{
			//if(pcap_setnonblock(handle,1,errbuf) <0)
				//fprintf(stderr, "Couldn't setnonblock %s\n",  errbuf);
	
			h.handle = handle;			
		    h.linktype = pcap_datalink(handle);	
			handles.push_back(h);  
			//cout<<handle<<endl;
		}		
		tmp = tmp->next;
	}
	pcap_freealldevs(it);
}


int pcap::do_something( callback_fun func )  
{
	unsigned int i;
	int res;

	for(i=0; i<handles.size(); i++)
	{
		/*which handler is better, loop or dispatch ? */
		res = pcap_dispatch(handles[i].handle, -1,func, (u_char*)&handles[i].linktype);
		//res = pcap_loop(handles[i].handle, 0,func, (u_char*)&handles[i].linktype);
		if( res < 0)
		{
			//cout<<"Dispatch error :"<<i<<pcap_geterr(handles[i].handle)<<endl;
			return res;
		}else{
			//cout<<"Success handle "<<res<<endl;
		}
		
	}

	return handles.size();
}

bool pcap::get_local_ip()
{
	struct ifaddrs *ifaddr, *ifa;  

	IP4s.clear();
	IP6s.clear();  	
	if (getifaddrs(&ifaddr) == -1) {  
       		//fprintf(stderr,"Err in getifaddrs\n");
        	return false;
   	 }  
	
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{  
        if (ifa->ifa_addr == NULL)  
            continue;  
		
		switch (ifa->ifa_addr->sa_family){
			case (AF_INET):
				struct in_addr addr;
        			addr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
				IP4s.push_back(addr);
				break;
			case (AF_INET6):
				struct in6_addr addr6;
				addr6 = ((struct sockaddr_in6	*)ifa->ifa_addr)->sin6_addr;
				IP6s.push_back(addr6);
				break;
			default:
				break;
		}
	}
	freeifaddrs(ifaddr);
    return true;
}

// Is a address in our local addresses?
bool pcap::IP4_contain(const struct in_addr &t)   
{
	for(unsigned int i=0; i<IP4s.size();i++)
		if(IP4s[i].s_addr == t.s_addr)
			return true;
	return false;	
}

// Is a address in our local addresses?
bool pcap::IP6_contain(const struct in6_addr &t)
{
	for(unsigned int i=0; i<IP6s.size();i++)
		if( memcmp(t.s6_addr, IP6s[i].s6_addr, sizeof(struct in6_addr))==0 )
			return true;
	return false;	
}


void pcap::test()
{
	open_devices();
	get_local_ip();
	while(1)
	{
		sleep(1);
		do_something(NULL);
	}	
	
}
