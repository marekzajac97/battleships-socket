#ifndef _MULTICAST_H_
#define _MULTICAST_H_

#define SA      struct sockaddr
#define IPV6 1


unsigned int _if_nametoindex(const char *);

int snd_udp_socket(const char *, int , SA **, socklen_t *);

int family_to_level(int );

int mcast_join(int , const SA *, socklen_t ,  const char *, u_int );

int sockfd_to_family(int );

int mcast_set_loop(int , int );

void recv_multicast(int, socklen_t);

void send_multicast(int, SA *, socklen_t);

#endif