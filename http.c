/*
 * Description: network http manager
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

static int on_url_cb(struct http_parser *parser, const char *data, int len)
{
	http_request *request;
	request = parser->data;
	request->url.len = len;
	request->url.data = data;

    return 0;
}

static int on_body_cb(struct http_parser *parser, const char *data, int len)
{
	http_request *request;
	request = parser->data;
	request->body.len = len;
	request->body.data = data;

    return 0;
}

static int on_message_complete_cb(struct http_parser *parser)
{
	http_request *request;
	request = parser->data;
	request->http_major = parser->http_major;
	request->http_minor = parser->http_minor;
	request->method = parser->method;

	return 0;
}

static struct http_parser_settings settings = {
    .on_url = on_url_cb,
    .on_body = on_body_cb,
	.on_message_complete = on_message_complete_cb,
};

void http_parser_get_request_value(http_request *request, char *data, size_t size)
{
	struct http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);
	parser.data = request;
	http_parser_execute(&parser, &settings, data, size);
}

const char *get_status_description(uint32_t status)
{
    switch (status) {
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 102:
        return "Processing";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi-Status";
    case 208:
        return "Already Reported";
    case 226:
        return "IM Used";
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 421:
        return "Misdirected Request";
    case 422:
        return "Unprocessable Entity";
    case 423:
        return "Locked";
    case 424:
        return "Failed Dependency";
    case 426:
        return "Upgrade Required";
    case 428:
        return "Precondition Required";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 451:
        return "Unavailable For Legal Reasons";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    case 506:
        return "Variant Also Negotiates";
    case 507:
        return "Insufficient Storage";
    case 508:
        return "Loop Detected";
    case 510:
        return "Not Extended";
    case 511:
        return "Network Authentication Required";
    }

    return "Unknown";
}

static void http_set_header(char *header, int code)
{
	char context[] = "HTTP/1.1 %d %s\n" \
	"Server: nginx/0.7.69\n" \
	"Content-Type: text/html\n" \
	"Connection: keep-alive\n" \
	"Accept-Ranges: bytes\r\n\r\n";

	sprintf(header,context,code,get_status_description(code));

	return;
}

void http_send_response(void *conn, char *body)
{
	struct skt_conn *skt = conn;

	char *response;
	response = (char *)malloc(skt->svr->socket_max_len);
	if (response == NULL) {
		printf("malloc http_send_response fail!\n");
		return;
	}

	http_set_header(response, 200);
	strcat(response, body);
	skt->write_buffer = strdup(response);
	skt->write_len = strlen(response);
	free(response);
}
