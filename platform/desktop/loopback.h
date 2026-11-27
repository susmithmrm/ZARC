#ifndef ZARC_LOOPBACK_H
#define ZARC_LOOPBACK_H
#include "zarc/connect.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
typedef SOCKET zarc_socket;
#define ZARC_BAD_SOCKET INVALID_SOCKET
#else
typedef int zarc_socket;
#define ZARC_BAD_SOCKET (-1)
#endif
/* Desktop laboratory fixture only. Bind and connect exclusively to loopback. */
bool zarc_net_init(void);
void zarc_net_cleanup(void);
void zarc_net_close(zarc_socket);
zarc_socket zarc_net_listen(uint16_t *port);
zarc_socket zarc_net_accept(zarc_socket);
zarc_socket zarc_net_connect(uint16_t port);
int zarc_net_readable(zarc_socket, unsigned timeout_ms);
int zarc_net_read(zarc_socket, uint8_t *, size_t);
bool zarc_net_send(zarc_socket, const zarc_frame *);
uint64_t zarc_net_milliseconds(void);
#endif
