/*
 * Description: network socket manager
 *     History: ou xiao bo, 2021/01/18, create
 */
# ifndef _SOCKET_H_
# define _SOCKET_H_

typedef struct skt_svr {
	int server_port;
	struct ev_loop *loop;
	ev_io ev_accept;
	int write_len;
	char *write_buffer;
	int read_len;
	char *read_buffer;
	void (*on_recv_pkg)(struct skt_svr *svr, void *data, size_t size);
}skt_svr;

typedef struct skt_conn {
    int fd;
    ev_io ev_read;
	ev_io ev_write;
	int flag;
	skt_svr *svr;
} skt_conn;

void socket_server_init(skt_svr svr);

#endif