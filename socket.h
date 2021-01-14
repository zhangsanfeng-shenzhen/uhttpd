/*
 * Description: network socket manager
 *     History: ou xiao bo, 2021/01/09, create
 */
# ifndef _SOCKET_H_
# define _SOCKET_H_

#define UH_CONNECTION_TIMEOUT 4

#define UH_CONNECT_READ 1
#define UH_CONNECT_WRITE 2
#define UH_CONNECT_CLOSE 3
#define UH_CONNECT_ACCEPT 4

typedef struct uh_svr
{
	char *host;
	int port;
	int fd;
	ev_io accept_watcher;
	struct ev_loop *loop;
	void (*on_recv_pkg)(void *conn, void *data, size_t size);
}uh_svr;

typedef struct uh_conn
{
	int fd;
	unsigned char flags;
    ev_io read_watcher;
    ev_io write_watcher;
    ev_timer timer_watcher;
	uh_svr *svr;
}uh_conn;

int uh_server_init(uh_svr *svr);

#endif