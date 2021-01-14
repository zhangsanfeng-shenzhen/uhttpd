/*
 * Description: network http manager
 *     History: ou xiao bo, 2021/01/09, create
 */

# ifndef _HTTP_H_
# define _HTTP_H_

# include "socket.h"

typedef struct uh_value {
    const char *at;
    size_t len;
}uh_value;

typedef struct uh_request {
    struct uh_value url;
    struct uh_value path;
    struct uh_value query;
    struct uh_value body;
}uh_request;

typedef struct uh_http
{
	uh_svr *socket;
}uh_http;

# endif