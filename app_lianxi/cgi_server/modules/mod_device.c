/**
 * mod_device.c - 设备管理模块
 *
 * 路由前缀: /api/device
 */
#include "../route.h"
#include "../common/cgi_util.h"
#include <stdio.h>
#include <string.h>

/* GET /api/device/list */
static void handle_device_list(struct mg_connection *c,
                               struct mg_http_message *hm, void *ud) {
    (void)hm; (void)ud;
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":["
                  "{\"id\":\"dev-001\",\"type\":\"sensor\",\"online\":true},"
                  "{\"id\":\"dev-002\",\"type\":\"camera\",\"online\":false}"
                  "]}");
}

/* GET /api/device/status?id=xxx */
static void handle_device_status(struct mg_connection *c,
                                 struct mg_http_message *hm, void *ud) {
    (void)ud;
    char id_buf[64] = {0};
    int id_len = mg_http_get_var(&hm->query, "id", id_buf, sizeof(id_buf));
    if (id_len <= 0) {
        mg_http_reply(c, 400, HEADER_JSON,
                      "{\"code\":-1,\"msg\":\"missing param: id\"}");
        return;
    }
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":{\"id\":\"%s\",\"online\":true,"
                  "\"uptime\":86400}}", id_buf);
}

/* POST /api/device/control  (body: {"id":"dev-001","cmd":"restart"}) */
static void handle_device_control(struct mg_connection *c,
                                  struct mg_http_message *hm, void *ud) {
    (void)ud;
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"msg\":\"command sent\",\"echo\":%.*s}",
                  (int)hm->body.len, hm->body.buf);
}

/* ---- 路由表 ---- */
static const cgi_route_t device_routes[] = {
    { "GET",  "/list",    handle_device_list    },
    { "GET",  "/status",  handle_device_status  },
    { "POST", "/control", handle_device_control },
};

const cgi_module_t mod_device = {
    .prefix      = "/api/device",
    .routes      = device_routes,
    .route_count = sizeof(device_routes) / sizeof(device_routes[0]),
    .module_data = NULL,
};
