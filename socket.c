/*
 * Description: network socket manager
 *     History: ou xiao bo, 2021/01/09, create
 */

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ev.h>

#include "socket.h"

int MAXLNE = 4096;

static void on_write(struct ev_loop *loop, ev_io *io, int revents)
{
	uh_conn *conn;
	conn = io->data;
	printf("write:%d\n",UH_CONNECT_WRITE);
	if(conn->flags == UH_CONNECT_WRITE) {
		ev_io_stop(loop, &conn->read_watcher);
		ev_io_stop(loop, &conn->write_watcher);
		close(conn->fd);
		free(conn);
	}
}

static void on_read(struct ev_loop *loop, ev_io *io, int revents)
{
	int len;
	char buff[MAXLNE];
	uh_conn *conn;

	conn = io->data;
	len = recv(io->fd, buff, MAXLNE, 0);
	if (len > 0) {
		conn->svr->on_recv_pkg(conn, buff, len);
		ev_io_start(loop, &conn->write_watcher);
		conn->flags = UH_CONNECT_WRITE;
	}
}

static void connection_timeout_cb(struct ev_loop *loop, ev_timer *io, int revents)
{
	uh_conn *conn;
	conn = io->data;

	ev_io_stop(loop, &conn->read_watcher);
	ev_io_stop(loop, &conn->write_watcher);
	close(conn->fd);
	free(conn);
}

static void on_accept(struct ev_loop *loop, ev_io *io, int revents)
{
	int connfd;
	uh_svr *svr;
	svr = io->data;
	if ((connfd = accept(svr->fd, (struct sockaddr *)NULL, NULL)) == -1) {
		printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		return;
	}
	uh_conn *conn;
	conn = (uh_conn *)malloc(sizeof(uh_conn));
	if (conn == NULL) {
		printf("%d\n",__LINE__);
		close(svr->fd);
		return;
	}

	conn->svr = svr;
	conn->fd = connfd;
	conn->read_watcher.data = conn;
	conn->flags = UH_CONNECT_READ;
	ev_io_init(&conn->read_watcher, on_read, connfd, EV_READ);
	ev_io_start(svr->loop, &conn->read_watcher);

	conn->write_watcher.data = conn;
	ev_io_init(&conn->write_watcher, on_write, connfd, EV_WRITE);
	//ev_io_start(svr->loop, &conn->write_watcher);
 
 	conn->timer_watcher.data = conn;
    ev_timer_init(&conn->timer_watcher, connection_timeout_cb, UH_CONNECTION_TIMEOUT, 0);
    ev_timer_start(svr->loop, &conn->timer_watcher);

}

int uh_server_init(uh_svr *svr)
{
    int fd;
    struct sockaddr_in servaddr;
 
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(svr->port);
 
    if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    if (listen(fd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

	svr->fd = fd;
	svr->accept_watcher.data = svr;
	ev_io_init(&svr->accept_watcher, on_accept, fd, EV_READ);
	ev_io_start(svr->loop, &svr->accept_watcher);

	ev_loop(svr->loop, 0);

    return 0;
}

