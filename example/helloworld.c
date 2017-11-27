#include <ev.h>
#include <stdio.h>
#include <uhttp.h>

static void signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
    printf("Got signal: %d\n", w->signum);
    ev_break(loop, EVBREAK_ALL);
}

void hook_test(struct uh_connection *con)
{
    struct uh_str *url = uh_get_url(con);
    struct uh_str *path = uh_get_path(con);
    struct uh_str name = uh_get_var(con, "name");
    struct uh_str *header_host = uh_get_header(con, "Host");
    struct uh_str *header_ua = uh_get_header(con, "User-Agent");
    char unescaped_name[128];

    uh_send_head(con, HTTP_STATUS_OK, -1, NULL);
    uh_printf_chunk(con, "<h1>Hello World</h1>");
    uh_printf_chunk(con, "<h1>Libuhttp v%s</h1>", uh_version());
    uh_printf_chunk(con, "<h1>Url: %.*s</h1>", (int)url->len, url->at);
    uh_printf_chunk(con, "<h1>Path: %.*s</h1>", (int)path->len, path->at);
    uh_printf_chunk(con, "<h1>Name: %.*s</h1>", (int)name.len, name.at);

	if (name.at) {
    	uh_unescape(name.at, name.len, unescaped_name, sizeof(unescaped_name));
    	uh_printf_chunk(con, "<h1>Unescaped Name: %s</h1>", unescaped_name);
	}

    if (header_host)
        uh_printf_chunk(con, "<h1>Host: %.*s</h1>", (int)header_host->len, header_host->at);

    if (header_ua)
        uh_printf_chunk(con, "<h1>User-Agent: %.*s</h1>", (int)header_ua->len, header_ua->at);
    
    uh_send_chunk(con, NULL, 0);
}

int main(int argc, char **argv)
{
    struct ev_loop *loop = EV_DEFAULT;
    ev_signal *sig_watcher = NULL;
    struct uh_server *srv = NULL;

    uh_log_info("libuhttp version: %s\n", uh_version());

    sig_watcher = calloc(1, sizeof(ev_signal));
    if (!sig_watcher)
        return -1;
    
    ev_signal_init(sig_watcher, signal_cb, SIGINT);
    ev_signal_start(loop, sig_watcher);

    srv = uh_server_new(loop, "0.0.0.0", 8000);
    if (!srv) {
        uh_log_err("uh_server_new failed\n");
        goto err;
    }

#if (UHTTP_SSL_ENABLED)
    if (uh_ssl_init(srv, "server-cert.pem", "server-key.pem") < 0)
        goto err;
#endif

    uh_register_hook(srv, "/test", hook_test);
    
    uh_log_info("Listen on 8000...\n");
    
    ev_run(loop, 0);
    
err:
    free(sig_watcher);
    uh_server_free(srv);
    
    return 0;
}


