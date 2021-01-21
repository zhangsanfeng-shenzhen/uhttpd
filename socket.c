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

static void free_res(struct ev_loop *loop, ev_io *io) {
    skt_conn *conn = io->data;
    if (conn == NULL) {
        fprintf(stderr, "line:%d -- the conn is  NULL:%s !\n",__LINE__,strerror(errno) );
        return;
    }
    ev_io_stop(loop, &conn->ev_read);
	ev_io_stop(loop, &conn->ev_write);
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

static void write_cb(struct ev_loop *loop, ev_io *io, int revents) {
	skt_conn *conn = io->data;
	if (conn->flag == 1) {
		free_res(loop, io);
		conn->flag = 2;
	}
}

static void read_cb(struct ev_loop *loop, ev_io *io, int revents) {
    int ret = 0;
	skt_conn *conn = io->data;
    int malloc_len = conn->svr->socket_max_len;

	conn->read_buffer = (char *)malloc(malloc_len);
	if (conn->read_buffer == NULL) {
		fprintf(stderr, "malloc error\n");
		free_res(loop, io);
        return ;
	}

	memset(conn->read_buffer,'\0',malloc_len);
    if (revents & EV_READ) {
        ret = read(conn->fd, conn->read_buffer, malloc_len);
		conn->read_len = ret;
		conn->svr->on_recv_pkg(conn, conn->read_buffer, ret);
		conn->flag = 1;
		ev_io_start(loop, &conn->ev_write);
		write(conn->fd, conn->write_buffer, conn->write_len);
    }

    if (EV_ERROR & revents) {
        fprintf(stderr, "error event in read\n");
        free_res(loop, io);
        return ;
    }
 
    if (ret < 0) {
        fprintf(stderr, "read error\n");
        ev_io_stop(EV_A_ io);
        free_res(loop, io);
        return;
    }
 
    if (ret == 0) {
        fprintf(stderr, "conn disconnected.\n");
        ev_io_stop(EV_A_ io);
        free_res(loop, io);
        return;
    }
}
 

static void accept_cb(struct ev_loop *loop, ev_io *io, int revents) {
    struct sockaddr_in conn_addr;
    socklen_t conn_len = sizeof(conn_addr);
    int conn_fd = accept(io->fd, (struct sockaddr *) &conn_addr, &conn_len);
    if (conn_fd == -1) {
        fprintf(stderr, "line:%d -- the accept return -1:%s !\n",__LINE__,strerror(errno) );
        return;
    }
 
    skt_conn *conn = malloc(sizeof(skt_conn));
    conn->fd = conn_fd;
	conn->svr = io->data;
    if (setnonblock(conn->fd) < 0)
        err(1, "failed to set conn socket to non-blocking");
 
    conn->ev_read.data = conn;

    ev_io_init(&conn->ev_read, read_cb, conn->fd, EV_READ);
    ev_io_start(loop, &conn->ev_read);

	conn->ev_write.data = conn;
    ev_io_init(&conn->ev_write, write_cb, conn->fd, EV_WRITE);
    //ev_io_start(loop, &conn->ev_write);
}


void socket_server_init(skt_svr svr)
{
	struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
		err(1, "listen failed");
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1)
		err(1, "setsockopt failed");
 
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(svr.server_port);
 
	if (bind(listen_fd, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
		err(1, "bind failed");
	if (listen(listen_fd, 5) < 0)
		err(1, "listen failed");
	if (setnonblock(listen_fd) < 0)
		err(1, "failed to set server socket to non-blocking");
 
 	svr.ev_accept.data = &svr;
	ev_io_init(&svr.ev_accept, accept_cb, listen_fd, EV_READ);
	ev_io_start(svr.loop, &svr.ev_accept);
    ev_loop(svr.loop, 0);
}
