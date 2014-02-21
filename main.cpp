/*
HuangSu
*/
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
using namespace std;
#include "packet_cap.h"
#include "inodeport.h"
#include "process.h"
#include "cui.h"
#include "mempool.h"
//#include "maindialog.h"

static int stop = 0;

/*signal handle for process flush*/
static void moniter_process(int signo)  
{
	/*first we get the packet list which represents the packets captured*/
	struct packet* list_packet = NULL;
	class pcap& Packet_instance = pcap::GetInstance();
	list_packet = Packet_instance.get_packet_list();
	Packet_instance.set_packet_list(NULL);

	class Cui& cui_instance = Cui::GetInstance();	
	cui_instance.show_title();


	if(!stop)
			alarm(1);
	else
		exit(0);
	/*if no packet, no work to do, set alarm and return ,  otherwise, flush packet into process*/
	if(NULL==list_packet){
		return ;
	}

	/*we need  map of inode to port, the map of port to process*/
	class inodeport &Inode_instance = inodeport::GetInstance();     
	class process_manager &Process_manager = process_manager::GetInstance(); 
	Inode_instance.refreshinodeport();  
	Process_manager.reflush_port(Inode_instance);     
	
	/*packet to process*/
	Process_manager.reflush_packet( list_packet );       

	/*flush window*/
/*	maindialog &win = maindialog::GetInstance();
	win.reflush( (const vector<struct process*>&)Process_manager.get_process_vec() );
*/
	
	cui_instance.show( (const vector<struct process*>&)Process_manager.get_process_vec() );
}

/*thread handle for capture packet*/
static void* packet_handle(void* args)
{
	class pcap& Packet_instance = pcap::GetInstance();
	Packet_instance.init();
	while(!stop)
	{			
		Packet_instance.do_something(callback);	
	}
	return (void*)(0);
}  


int main(int argc, char *argv[])
{
 //   QApplication app(argc,argv);

	class Cui&  cui_instance = Cui::GetInstance();	
	cui_instance.show_title();
	
	if( signal(SIGALRM,moniter_process) == SIG_ERR )
	{
		return -1;
	}	
	alarm(1);	

	int err;
	void *perr = &err;
	pthread_t pcap_thread;
	err =  pthread_create(&pcap_thread, NULL, packet_handle, NULL);
	if(0!=err)
		exit(-1);
	
	if((char)getchar() == 'q')
		exit(0);
	pthread_join(pcap_thread, &perr);
	//	maindialog &win = maindialog::GetInstance();
  //  win.show();
//	return app.exec();
	return 0;
}




/*what we want to do when a packet arrives,  we need pay attention to the network byte order and host byte order*/
void callback(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
	//cout<<"In callback"<<endl;
	int linktype;     //data link type
	u_int16_t IPtype;   //ip layer type
	u_int8_t Transfer_p;	 //transfer layer type
	u_char* buf;            //buf  to captured data
	int direction = 0;     // packet in or out
	u_int16_t len;    // payload of the packet
		
	struct ethhdr *ethernet;
	struct ppp_header *ppp;
	struct ip * ip;
	struct ip6_hdr * ip6;
	struct packet *list_packet;
	pcap& instance = pcap::GetInstance();
	linktype = *(int*)args;
	
	
	//parse data link layer
	switch (linktype) 
	{
		case DLT_EN10MB:
			ethernet = (struct ethhdr *)packet;
			IPtype = ethernet->h_proto;
			buf =  (u_char*)packet + sizeof(struct ethhdr);
			break;
		case DLT_PPP:
			ppp = (struct ppp_header *) packet;
			IPtype = ppp->packettype;
			buf =  (u_char*)packet + sizeof(struct ppp_header);
			break;
		default:
			//fprintf(stdout, "Unknown linktype %d", linktype);
			return ;
	}

	//parse ip layer
	switch (IPtype){
		case (0x0008):  //ipv4
			ip = (struct ip *) buf;
			Transfer_p = ip->ip_p;
			buf = buf + sizeof(struct ip);
			
			if( instance.IP4_contain(ip->ip_src) )
				direction = OUT;
			else if( instance.IP4_contain(ip->ip_dst) )
				direction = IN;
			else
 				return;
			len = ntohs(ip->ip_len);
			break;
		case (0xDD86): //ipv6
			ip6 = (struct ip6_hdr *) buf;
			Transfer_p = (ip6->ip6_ctlun).ip6_un1.ip6_un1_nxt;
			buf = buf + sizeof(struct ip6_hdr);
			if( instance.IP6_contain(ip6->ip6_src) )
				direction = OUT;
			else if( instance.IP6_contain(ip6->ip6_dst) )
				direction = IN;
			else
				return;
			len = ntohs(ip6->ip6_ctlun.ip6_un1.ip6_un1_plen);
			break;
		default:
			// TODO: maybe support for other protocols apart from IPv4 and IPv6 
			return ;
	}
	
	/*statistic*/
	if(IPPROTO_TCP == Transfer_p){
		tcphdr *tcp = (tcphdr*)buf;
		struct packet* p;	
		list_packet = instance.get_packet_list();
		if(IN==direction)
		{
			p = new struct packet(list_packet, len, ntohs(tcp->dest), ntohs(tcp->source), IN);
			instance.lenin += len;
		}
        else
        {
		    p = new struct packet(list_packet, len, ntohs(tcp->source), ntohs(tcp->dest) , OUT);
			instance.lenout += len;
		}
		instance.set_packet_list(p);		
	}
}

