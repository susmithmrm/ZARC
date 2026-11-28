#include "transport.h"
#include <zephyr/posix/fcntl.h>
#include <errno.h>
#include <string.h>

static bool valid_timeout(uint32_t timeout_ms)
{
 return timeout_ms>0u&&timeout_ms<=ZARC_NATIVE_TCP_TIMEOUT_MAX_MS;
}

static void fail_closed(struct zarc_native_tcp *tcp)
{
 if(tcp->fd>=0){(void)zsock_close(tcp->fd);tcp->fd=-1;}
}

static int wait_ready(struct zarc_native_tcp *tcp,short events,int64_t deadline)
{
 for(;;){
  int64_t remaining=deadline-k_uptime_get();
  if(remaining<=0)return -ETIMEDOUT;
  struct zsock_pollfd fd={.fd=tcp->fd,.events=events};
  int ret=zsock_poll(&fd,1,(int)remaining);
  if(ret<0){if(errno==EINTR)continue;return -errno;}
  if(ret==0)return -ETIMEDOUT;
  if(fd.revents&events)return 0;
  if(fd.revents&(ZSOCK_POLLERR|ZSOCK_POLLHUP|ZSOCK_POLLNVAL)){
   int error=0;socklen_t length=sizeof(error);
   if(zsock_getsockopt(tcp->fd,ZSOCK_SOL_SOCKET,ZSOCK_SO_ERROR,&error,&length)<0)return -errno;
   return error? -error : -ECONNRESET;
  }
 }
}

void zarc_native_tcp_init(struct zarc_native_tcp *tcp)
{
 if(tcp)tcp->fd=-1;
}

int zarc_native_tcp_connect(struct zarc_native_tcp *tcp,const struct sockaddr_in *remote,uint32_t timeout_ms)
{
 if(!tcp||!remote)return -EINVAL;
 if(tcp->fd>=0)return -EALREADY;
 if(remote->sin_family!=AF_INET||remote->sin_port==0||!valid_timeout(timeout_ms))return -EINVAL;
 int fd=zsock_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 if(fd<0)return -errno;
 tcp->fd=fd;
 if(zsock_fcntl(fd,F_SETFL,O_NONBLOCK)<0){int error=errno;fail_closed(tcp);return -error;}
 int ret=zsock_connect(fd,(const struct sockaddr *)remote,sizeof(*remote));
 if(ret<0){
  if(errno!=EINPROGRESS&&errno!=EALREADY&&errno!=EWOULDBLOCK){int error=errno;fail_closed(tcp);return -error;}
  int64_t deadline=k_uptime_get()+timeout_ms;
  ret=wait_ready(tcp,ZSOCK_POLLOUT,deadline);
  if(ret<0){fail_closed(tcp);return ret;}
  int error=0;socklen_t length=sizeof(error);
  if(zsock_getsockopt(fd,ZSOCK_SOL_SOCKET,ZSOCK_SO_ERROR,&error,&length)<0){error=errno;fail_closed(tcp);return -error;}
  if(error){fail_closed(tcp);return -error;}
 }
 return 0;
}

int zarc_native_tcp_send(struct zarc_native_tcp *tcp,const zarc_frame *frame,uint32_t timeout_ms)
{
 if(!tcp||!frame)return -EINVAL;
 if(tcp->fd<0)return -ENOTCONN;
 if(!valid_timeout(timeout_ms))return -EINVAL;
 uint8_t bytes[ZARC_NATIVE_FRAME_MAX];size_t length=0;
 if(!zarc_frame_encode(frame,bytes,sizeof(bytes),&length))return -EINVAL;
 int64_t deadline=k_uptime_get()+timeout_ms;size_t sent=0;
 while(sent<length){
  int ret=wait_ready(tcp,ZSOCK_POLLOUT,deadline);
  if(ret<0){fail_closed(tcp);return ret;}
  ssize_t count=zsock_send(tcp->fd,bytes+sent,length-sent,ZSOCK_MSG_DONTWAIT);
  if(count>0){sent+=(size_t)count;continue;}
  if(count==0){fail_closed(tcp);return -EPIPE;}
  if(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK)continue;
  int error=errno;fail_closed(tcp);return -error;
 }
 return 0;
}

int zarc_native_tcp_receive(struct zarc_native_tcp *tcp,struct zarc_native_bytes *bytes,uint32_t timeout_ms)
{
 if(!tcp||!bytes)return -EINVAL;
 if(tcp->fd<0)return -ENOTCONN;
 if(!valid_timeout(timeout_ms))return -EINVAL;
 bytes->len=0;
 int ret=wait_ready(tcp,ZSOCK_POLLIN,k_uptime_get()+timeout_ms);
 if(ret<0){if(ret!=-ETIMEDOUT)fail_closed(tcp);return ret;}
 ssize_t count=zsock_recv(tcp->fd,bytes->data,sizeof(bytes->data),ZSOCK_MSG_DONTWAIT);
 if(count>0){bytes->len=(size_t)count;return 0;}
 if(count==0){fail_closed(tcp);return -ECONNRESET;}
 if(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK)return -EAGAIN;
 ret=-errno;fail_closed(tcp);return ret;
}

int zarc_native_tcp_close(struct zarc_native_tcp *tcp)
{
 if(!tcp)return -EINVAL;
 if(tcp->fd<0)return 0;
 int fd=tcp->fd;tcp->fd=-1;
 return zsock_close(fd)<0?-errno:0;
}
