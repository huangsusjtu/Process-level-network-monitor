#ifndef GET_KERNEL_INFO_H
#define GET_KERNEL_INFO_H
#include <iostream>
#include <unistd.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <QHash>
#include <QString>

using namespace std;

const int NETLINK_TEST=22;

enum{
    IN=0,
    OUT=1
};

struct data_info{
    int protocol;
    int type;
    unsigned short dest_port;
    unsigned short local_port;
    unsigned short data_len;
};

struct kernel_data
{
    struct nlmsghdr hdr;
    struct data_info new_data;
};



class get_kernel_info
{
public:
    get_kernel_info();
    ~get_kernel_info();

public:
    bool recv_from_kernel();
    void flush();
private:
    struct nlmsghdr message;
    struct sockaddr_nl local;
    struct sockaddr_nl kpeer;
    struct in_addr addr;
    int sock_fd;
    int k_peer_len;
public:
    QHash<unsigned short ,struct data_info> kernel_info;
};

#endif // GET_KERNEL_INFO_H
