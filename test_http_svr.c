/*
 * Description: network test manager
 *     History: ou xiao bo, 2021/01/18, create
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <ev.h>

#include "socket.h"
#include "http_parser.h"
#include "http.h"

void on_recv_pkg(struct skt_svr *svr, void *data, size_t size)
{
	char *buff;
	buff = data;

	char body[]="Hello World\n";
	http_parser_get_request_value(buff, size);
	http_send_response(svr, body);

	return;
}

int main(int argc, char const *argv[]) 
{
	skt_svr svr;
	svr.loop = ev_default_loop(0);
	svr.server_port = 8000;
	svr.on_recv_pkg = on_recv_pkg;
	socket_server_init(svr);

    return 0;
}