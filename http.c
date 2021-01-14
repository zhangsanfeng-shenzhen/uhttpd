/*
 * Description: network http manager
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
#include "http.h"
#include "http_parser.h"

int on_url(http_parser *parser, const char *data, size_t len) {
	uh_request *request;

	request = parser->data;
	request->url.at = data;
	request->url.len = len;
	printf("Url: %.*s\n", (int)len, data);
	return 0;
}

int on_body(http_parser *parser, const char *data, size_t len) {
	uh_request *request;

	request = parser->data;
	request->body.at = data;
	request->body.len = len;
	printf("Body: %.*s\n", (int)len, data);
	return 0;
}

static void on_http_parser(uh_request *request, char *data, size_t size)
{
	http_parser_settings parser_set;
	parser_set.on_url = on_url;
    parser_set.on_body = on_body;
	http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
	if (parser == NULL) {
		printf("error:%d\n",__LINE__);
		return;
	}
	parser->data = request;
	http_parser_init(parser, HTTP_REQUEST);
	int parsered = http_parser_execute(parser, &parser_set, data, size);
	if (parsered != size){
		printf("error%d\n",__LINE__);
		return;
	}
	free(parser);
}

void http_404_response(uh_conn *conn)
{
	char msg[1024];
	strcpy(msg, "HTTP/1.1 404 Not Found\n");
	strcat(msg, "Server: uhttpd/1.0.0\n");
	strcat(msg, "Content-Type: text/html;charset=utf-8\n");
	strcat(msg, "\r\n\r\n");
	strcat(msg, "404");
	write(conn->fd, msg, strlen(msg));
}

void on_recv_pkg(uh_conn *conn, void *data, size_t size)
{
	uh_request request;
	on_http_parser(&request, data, size);
	http_404_response(conn);
}

int main(void)
{
	uh_svr *svr;
	svr = (uh_svr *)malloc(sizeof(uh_svr));
	if (svr == NULL) {
		printf("%d\n",__LINE__);
		return -1;
	}
	svr->port = 8000;
	svr->host = strdup("192.168.2.226");
	svr->loop = ev_default_loop(0);
	if (svr->host == NULL) {
		printf("%d\n",__LINE__);
		return -1;
	}
	svr->on_recv_pkg = on_recv_pkg;

	uh_http *http;
	http = (uh_http *)malloc(sizeof(uh_http));
	if (http == NULL) {
		printf("%d\n",__LINE__);
		return -1;
	}
	http->socket = svr;
	uh_server_init(svr);

	return 1;
}