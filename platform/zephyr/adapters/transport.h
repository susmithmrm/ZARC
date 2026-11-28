#ifndef ZARC_NATIVE_TRANSPORT_H
#define ZARC_NATIVE_TRANSPORT_H
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/atomic.h>
#include "zarc/connect.h"
#define ZARC_NATIVE_FRAME_MAX (ZARC_FRAME_HEADER + ZARC_PAYLOAD_MAX)
struct zarc_native_bytes { size_t len; uint8_t data[ZARC_NATIVE_FRAME_MAX]; };
/* RX queue elements are copied by callbacks; overflow invalidates the link.
 * Owner must close admission and reconcile session on any fault.
 * State and buffers must outlive driver registration and all callbacks. */
#if defined(CONFIG_UART_ASYNC_API)
struct zarc_native_uart {
 const struct device *dev;
 struct k_msgq *rx;
 atomic_t tx_busy, fault, rx_active, stopping;
 uint8_t rx_buf[2][ZARC_NATIVE_FRAME_MAX];
 uint8_t tx_buf[ZARC_NATIVE_FRAME_MAX];
 atomic_t rx_owned[2];
};
int zarc_native_uart_open(struct zarc_native_uart *,const struct device *,struct k_msgq *);
int zarc_native_uart_send(struct zarc_native_uart *,const zarc_frame *);
int zarc_native_uart_stop(struct zarc_native_uart *);
#endif
#if defined(CONFIG_CAN)
#include <zephyr/drivers/can.h>
struct zarc_native_can {
 const struct device *dev;
 struct k_msgq *rx;
 int filter_id;
 atomic_t fault;
 bool active;
};
int zarc_native_can_open(struct zarc_native_can *,const struct device *,struct k_msgq *,const struct can_filter *);
int zarc_native_can_send(struct zarc_native_can *,const struct can_frame *);
int zarc_native_can_close(struct zarc_native_can *);
#endif
#if defined(CONFIG_IPC_SERVICE)
#include <zephyr/ipc/ipc_service.h>
struct zarc_native_ipc {
 const struct device *dev;
 struct ipc_ept endpoint;
 struct ipc_ept_cfg config;
 struct k_msgq *rx;
 atomic_t bound,fault;
 bool registered;
};
int zarc_native_ipc_open(struct zarc_native_ipc *,const struct device *,const char *,struct k_msgq *);
int zarc_native_ipc_send(struct zarc_native_ipc *,const zarc_frame *);
int zarc_native_ipc_close(struct zarc_native_ipc *);
#endif
#if defined(CONFIG_NET_SOCKETS)
#include <zephyr/net/socket.h>
#define ZARC_NATIVE_TCP_TIMEOUT_MAX_MS 2147483647u
/* Single-owner, caller-polled TCP transport for the existing framed protocol.
 * It does not select a network interface, configure TLS/authentication, or
 * claim Ethernet PHY behavior. The owner must handle every error by reconciling
 * the session; receive returns stream bytes and never assumes frame boundaries. */
struct zarc_native_tcp {
 int fd;
};
void zarc_native_tcp_init(struct zarc_native_tcp *);
int zarc_native_tcp_connect(struct zarc_native_tcp *,const struct sockaddr_in *,uint32_t timeout_ms);
int zarc_native_tcp_send(struct zarc_native_tcp *,const zarc_frame *,uint32_t timeout_ms);
int zarc_native_tcp_receive(struct zarc_native_tcp *,struct zarc_native_bytes *,uint32_t timeout_ms);
int zarc_native_tcp_close(struct zarc_native_tcp *);
#endif
#endif
