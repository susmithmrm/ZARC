#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#else
#define _WIN32_WINNT 0x0600
#endif
#include "loopback.h"
#include <string.h>
#ifdef _WIN32
#include <windows.h>
typedef int address_length;
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
typedef socklen_t address_length;
#endif
bool zarc_net_init(void){
#ifdef _WIN32
 WSADATA data;return WSAStartup(MAKEWORD(2,2),&data)==0;
#else
 return signal(SIGPIPE,SIG_IGN)!=SIG_ERR;
#endif
}
void zarc_net_cleanup(void){
#ifdef _WIN32
 (void)WSACleanup();
#endif
}
void zarc_net_close(zarc_socket s){
 if(s==ZARC_BAD_SOCKET)return;
#ifdef _WIN32
 (void)closesocket(s);
#else
 (void)close(s);
#endif
}
static struct sockaddr_in address(uint16_t port){
 struct sockaddr_in a;memset(&a,0,sizeof a);a.sin_family=AF_INET;a.sin_port=htons(port);a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);return a;
}
static bool send_timeout(zarc_socket s){
#ifdef _WIN32
 DWORD ms=200;return setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,(const char *)&ms,(int)sizeof ms)==0;
#else
 struct timeval t={0,200000};return setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&t,(socklen_t)sizeof t)==0;
#endif
}
zarc_socket zarc_net_listen(uint16_t *port){
 if(!port)return ZARC_BAD_SOCKET;
 zarc_socket s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(s==ZARC_BAD_SOCKET)return s;
 struct sockaddr_in a=address(*port);address_length n=(address_length)sizeof a;
 if(bind(s,(const struct sockaddr *)&a,n)!=0||listen(s,2)!=0||getsockname(s,(struct sockaddr *)&a,&n)!=0){
  zarc_net_close(s);return ZARC_BAD_SOCKET;
 }
 *port=ntohs(a.sin_port);return s;
}
zarc_socket zarc_net_accept(zarc_socket s){
 zarc_socket p=accept(s,NULL,NULL);if(p!=ZARC_BAD_SOCKET&&!send_timeout(p)){zarc_net_close(p);return ZARC_BAD_SOCKET;}return p;
}
zarc_socket zarc_net_connect(uint16_t port){
 zarc_socket s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(s==ZARC_BAD_SOCKET)return s;
 struct sockaddr_in a=address(port);
 if(connect(s,(const struct sockaddr *)&a,(address_length)sizeof a)!=0||!send_timeout(s)){
  zarc_net_close(s);return ZARC_BAD_SOCKET;
 }
 return s;
}
int zarc_net_readable(zarc_socket s,unsigned ms){
 if(s==ZARC_BAD_SOCKET)return -1;
#ifndef _WIN32
 if(s>=FD_SETSIZE)return -1;
#endif
 fd_set set;FD_ZERO(&set);FD_SET(s,&set);
 struct timeval t;t.tv_sec=(long)(ms/1000u);t.tv_usec=(long)((ms%1000u)*1000u);
#ifdef _WIN32
 return select(0,&set,NULL,NULL,&t);
#else
 return select(s+1,&set,NULL,NULL,&t);
#endif
}
int zarc_net_read(zarc_socket s,uint8_t *p,size_t n){
 if(!p||!n||n>4096u)return -1;
#ifdef _WIN32
 return recv(s,(char *)p,(int)n,0);
#else
 return (int)recv(s,p,n,0);
#endif
}
bool zarc_net_send(zarc_socket s,const zarc_frame *f){
 uint8_t b[ZARC_FRAME_HEADER+ZARC_PAYLOAD_MAX];size_t n;
 if(!zarc_frame_encode(f,b,sizeof b,&n))return false;
 size_t offset=0;
 while(offset<n){
#ifdef _WIN32
  int sent=send(s,(const char *)b+offset,(int)(n-offset),0);
#else
  int sent=(int)send(s,b+offset,n-offset,0);
#endif
  if(sent<=0)return false;
  offset+=(size_t)sent;
 }
 return true;
}
uint64_t zarc_net_milliseconds(void){
#ifdef _WIN32
 return (uint64_t)GetTickCount64();
#else
 struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t)!=0)return 0;
 return (uint64_t)t.tv_sec*UINT64_C(1000)+(uint64_t)t.tv_nsec/UINT64_C(1000000);
#endif
}
