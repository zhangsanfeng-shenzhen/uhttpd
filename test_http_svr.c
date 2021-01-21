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

void on_recv_pkg(void *conn, void *data, size_t size)
{
	char *buff;
	buff = data;

	http_request *request;
	request = (http_request *)malloc(sizeof(http_request));
	if (request == NULL) {
		printf("error\n");
		return;
	}
	char body[1024];
	http_parser_get_request_value(request, buff, size);
	sprintf(body, "Hello World\nmethod is : %ld\n", request->method);
	http_send_response(conn, body);
	free(request);

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