/*
 * Description: network socket manager
 *     History: ou xiao bo, 2021/01/18, create
 */

#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <ev.h>

#include "socket.h"
#include "log.h"

static void connection_destroy(skt_conn *conn) {
    if (conn == NULL) {
		log_error("the conn is fail!");
        return;
    }

    ev_io_stop(conn->svr->loop, &conn->ev_read);
	ev_io_stop(conn->svr->loop, &conn->ev_write);
	ev_timer_stop(conn->svr->loop, &conn->ev_timer);
	free(conn->write_buffer);
	free(conn->read_buffer);
    close(conn->fd);
    free(conn);
}

int setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        return flags;
 
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;
 
    return 0;
}

static void timeout_cb(struct ev_loop *loop, ev_timer *timer, int revents)
{
	skt_conn *conn = timer->data;
	connection_destroy(conn);
}

static void write_cb(struct ev_loop *loop, ev_io *io, int revents) {
	skt_conn *conn = io->data;
	if (conn->flag == SOCKET_CONNECT_READ_FLAGE) {
		connection_destroy(conn);
		conn->flag = SOCKET_CONNECT_WRITE_FLAGE;
	}
}

static void read_cb(struct ev_loop *loop, ev_io *io, int revents) {
    int ret = 0;
	skt_conn *conn = io->data;
    int malloc_len = conn->svr->socket_max_len;

	conn->read_buffer = (char *)malloc(malloc_len);
	if (conn->read_buffer == NULL) {
		log_error("read_cb function malloc fail!");
		connection_destroy(conn);
        return ;
	}

	memset(conn->read_buffer,'\0',malloc_len);
    if (revents & EV_READ) {
        ret = read(conn->fd, conn->read_buffer, malloc_len);
		conn->read_len = ret;
		conn->svr->on_recv_pkg(conn, conn->read_buffer, ret);
		conn->flag = SOCKET_CONNECT_READ_FLAGE;
		ev_io_start(loop, &conn->ev_write);
		write(conn->fd, conn->write_buffer, conn->write_len);
    }

    if (EV_ERROR & revents) {
        log_error("error event in read\n");
        connection_destroy(conn);
        return ;
    }
 
    if (ret < 0) {
        log_error("read error\n");
        ev_io_stop(EV_A_ io);
        connection_destroy(conn);
        return;
    }
 
    if (ret == 0) {
        log_error("conn disconnected.\n");
        ev_io_stop(EV_A_ io);
        connection_destroy(conn);
        return;
    }
}
 

static void accept_cb(struct ev_loop *loop, ev_io *io, int revents) {
    struct sockaddr_in conn_addr;
    socklen_t conn_len = sizeof(conn_addr);
    int conn_fd = accept(io->fd, (struct sockaddr *) &conn_addr, &conn_len);
    if (conn_fd == -1) {
        log_error("the accept connect is fail!");
        return;
    }
 
    skt_conn *conn = malloc(sizeof(skt_conn));
    conn->fd = conn_fd;
	conn->svr = io->data;
    if (setnonblock(conn->fd) < 0)
        log_error("failed to set conn socket to non-blocking");
 
    conn->ev_read.data = conn;
    ev_io_init(&conn->ev_read, read_cb, conn->fd, EV_READ);
    ev_io_start(loop, &conn->ev_read);

	conn->ev_write.data = conn;
    ev_io_init(&conn->ev_write, write_cb, conn->fd, EV_WRITE);
    //ev_io_start(loop, &conn->ev_write);

	conn->ev_timer.data = conn;
	ev_timer_init(&conn->ev_timer, timeout_cb, SOCKET_CONNECTION_TIMEOUT, 0);
	ev_timer_start(loop, &conn->ev_timer);
}


void socket_server_init(skt_svr svr)
{
	struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
		log_error("listen failed");
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1)
		log_error("setsockopt failed");
 
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(svr.server_port);
 
	if (bind(listen_fd, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
		log_error("bind failed");
	if (listen(listen_fd, 5) < 0)
		log_error("listen failed");
	if (setnonblock(listen_fd) < 0)
		log_error("failed to set server socket to non-blocking");
 
 	svr.ev_accept.data = &svr;
	ev_io_init(&svr.ev_accept, accept_cb, listen_fd, EV_READ);
	ev_io_start(svr.loop, &svr.ev_accept);
    ev_loop(svr.loop, 0);
}
