/**
 * mod_system.c - 系统管理模块
 *
 * 路由前缀: /api/sys
 */
#include "../route.h"
#include "../common/cgi_util.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* GET /api/sys/info */
static void handle_sys_info(struct mg_connection *c,
                            struct mg_http_message *hm, void *ud) {
    (void)hm; (void)ud;
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":{"
                  "\"version\":\"1.0.0\","
                  "\"build\":\"" __DATE__ " " __TIME__ "\""
                  "}}");
}

/* GET /api/sys/time */
static void handle_sys_time(struct mg_connection *c,
                            struct mg_http_message *hm, void *ud) {
    (void)hm; (void)ud;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":{\"time\":\"%s\",\"ts\":%lld}}",
                  buf, (long long)now);
}

/* GET /api/sys/health */
static void handle_sys_health(struct mg_connection *c,
                              struct mg_http_message *hm, void *ud) {
    (void)hm; (void)ud;
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":{\"status\":\"ok\"}}");
}

/* ---- 路由表 ---- */
static const cgi_route_t system_routes[] = {
    { "GET", "/info",   handle_sys_info   },
    { "GET", "/time",   handle_sys_time   },
    { "GET", "/health", handle_sys_health },
};

const cgi_module_t mod_system = {
    .prefix      = "/api/sys",
    .routes      = system_routes,
    .route_count = sizeof(system_routes) / sizeof(system_routes[0]),
    .module_data = NULL,
};
