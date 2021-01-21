/*
 * Description: network http manager
 *     History: ou xiao bo, 2021/01/18, create
 */
# ifndef _HTTP_H_
# define _HTTP_H_

typedef struct http_value {
    const char *data;
    size_t len;
}http_value;

typedef struct uh_request {
    http_value url;
    http_value body;
	int http_major;
	int http_minor;
	long method;
}http_request;
void http_parser_get_request_value(http_request *request, char *data, size_t size);
void http_send_response(void *conn, char *body);

#endif