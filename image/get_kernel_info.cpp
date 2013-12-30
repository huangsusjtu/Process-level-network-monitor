#include <get_kernel_info.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

get_kernel_info::get_kernel_info()
{
    //system("rmmod filter.ko");
   // system("insmod ../netlink/filter.ko");

    kernel_info.clear();
    char a[20];
    sock_fd=socket(AF_NETLINK,SOCK_RAW,NETLINK_TEST);
    if(!sock_fd)
        {
            cout<<"create socket errro"<<endl;
            return ;
        }
    memset(&local,0,sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = getpid();
    local.nl_groups = 0;
    if(bind(sock_fd,(struct sockaddr *)&local,sizeof(local)) != 0)
       {
           cout<<"bind socket errro"<<endl;
           return ;
       }

    memset(&kpeer,0,sizeof(kpeer));
    kpeer.nl_family = AF_NETLINK;
    kpeer.nl_pid = 0;
    kpeer.nl_groups = 0; //not in multicast

    memset(&message,0,sizeof(message));
    message.nlmsg_len = NLMSG_LENGTH(0);
    message.nlmsg_flags = 0;
    message.nlmsg_pid = local.nl_pid;

     // cout<<"begin send\n";
     //send to kernel
     sendto(sock_fd,&message,message.nlmsg_len,0,(struct sockaddr *)&kpeer,sizeof(kpeer));
     k_peer_len=sizeof(socklen_t);

     //cout<<"begin recv\n";
     if(0>=recvfrom(sock_fd,a,10,0,(struct sockaddr *)&kpeer,(socklen_t *)&k_peer_len))
     {
         cout<<"netlink rcv errro"<<endl;
     }else{
         cout<<a<<" response from kernel"<<endl;
     }
     if (-1 == fcntl(sock_fd, F_SETFL, O_NONBLOCK))
      {
         cout<<"fcntl socket error!\n";
      }

}

get_kernel_info::~get_kernel_info()
{
    kernel_info.clear();
    //system("rmmod filter.ko");
}

bool get_kernel_info::recv_from_kernel()
{
    struct kernel_data *new_kernel = new kernel_data;
    memset(new_kernel,'\0',sizeof(struct kernel_data));

    struct timeval timeout={1,0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock_fd,&fds);
    select(sock_fd+1,&fds,NULL,NULL,&timeout);
    if(FD_ISSET(sock_fd,&fds))
    {

        //recvfrom(sock_fd,new_data,sizeof(struct data_info),0,(struct sockaddr *)&kpeer,(socklen_t *)&k_peer_len);
        recvfrom(sock_fd,new_kernel,sizeof(struct kernel_data),0,(struct sockaddr *)&kpeer,(socklen_t *)&k_peer_len);


        new_kernel->new_data.data_len = ntohs(new_kernel->new_data.data_len);
        new_kernel->new_data.dest_port = ntohs(new_kernel->new_data.dest_port);
        new_kernel->new_data.local_port = ntohs(new_kernel->new_data.local_port);
        cout<<new_kernel->new_data.dest_port<<endl;

       if(this->kernel_info.contains(new_kernel->new_data.local_port))
          {
             new_kernel->new_data.data_len += this->kernel_info.value(new_kernel->new_data.local_port).data_len;
          }
       this->kernel_info.insert(new_kernel->new_data.local_port,new_kernel->new_data);
       return true;
   }else{
        delete new_kernel;
        return false;
    }


}

void get_kernel_info::flush()
{
     kernel_info.clear();
}
