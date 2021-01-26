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
#include <sys/file.h>
#include <sys/resource.h>

#include "socket.h"
#include "http_parser.h"
#include "http.h"
#include "log.h"

int set_file_limit(size_t limit)
{
    struct rlimit rlim;
    memset(&rlim, 0, sizeof(rlim));
    if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
        return -1;
    }
    if (rlim.rlim_cur >= limit)
        return 0;
    rlim.rlim_cur = limit;
    rlim.rlim_max = limit;
    if (setrlimit(RLIMIT_NOFILE, &rlim) < 0) {
        return -1;
    }

    return 0;
}

typedef struct log_cfg {
    char   *path;
    char   *flag;
    int     shift;
    int     max;
    int     num;
    int     keep;
} log_cfg;

static int init_log(void)
{
	log_cfg log;
	log.path = strdup("/tmp/fuck/logger.log");
	log.flag = strdup("fatal,error,warn,info,debug,trace");
	log.shift = 1;
	log.max = 100 * 1000 * 1000;
	log.num = 100;
	log.keep = 7;

    default_dlog = dlog_init(log.path, log.shift, log.max, log.num, log.keep);
    if (default_dlog == NULL)
        return -__LINE__;
    default_dlog_flag = dlog_read_flag(log.flag);

    return 0;
}

void on_recv_pkg(void *conn, void *data, size_t size)
{
	char *buff;
	buff = data;

	http_request *request;
	request = (http_request *)malloc(sizeof(http_request));
	if (request == NULL) {
		log_error("on_recv_pkg malloc fail!");
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
	set_file_limit(1001000);
	init_log();

	skt_svr svr;
	svr.loop = ev_default_loop(0);
	svr.server_port = 8000;
	svr.on_recv_pkg = on_recv_pkg;
	svr.socket_max_len = 1024;
	socket_server_init(svr);

	log_vip("server start");
	log_stderr("server start");

    return 0;
}