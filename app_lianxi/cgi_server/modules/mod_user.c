/**
 * mod_user.c - 用户管理模块
 *
 * 路由前缀: /api/user
 */
#include "../route.h"
#include "../common/cgi_util.h"
#include <stdio.h>
#include <string.h>

/* GET /api/user/list */
static void handle_user_list(struct mg_connection *c,
                             struct mg_http_message *hm, void *ud) {
    (void)hm; (void)ud;
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":[{\"id\":1,\"name\":\"alice\"},"
                  "{\"id\":2,\"name\":\"bob\"}]}");
}

/* GET /api/user/detail?id=xxx */
static void handle_user_detail(struct mg_connection *c,
                               struct mg_http_message *hm, void *ud) {
    (void)ud;
    char id_buf[32] = {0};
    int id_len = mg_http_get_var(&hm->query, "id", id_buf, sizeof(id_buf));
    if (id_len <= 0) {
        mg_http_reply(c, 400, HEADER_JSON,
                      "{\"code\":-1,\"msg\":\"missing param: id\"}");
        return;
    }
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"data\":{\"id\":%s,\"name\":\"alice\"}}",
                  id_buf);
}

/* POST /api/user/create  (body: JSON) */
static void handle_user_create(struct mg_connection *c,
                               struct mg_http_message *hm, void *ud) {
    (void)ud;
    /* 简单示例: 回显请求体 */
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"msg\":\"created\",\"echo\":%.*s}",
                  (int)hm->body.len, hm->body.buf);
}

/* DELETE /api/user/delete?id=xxx */
static void handle_user_delete(struct mg_connection *c,
                               struct mg_http_message *hm, void *ud) {
    (void)ud;
    char id_buf[32] = {0};
    int id_len = mg_http_get_var(&hm->query, "id", id_buf, sizeof(id_buf));
    if (id_len <= 0) {
        mg_http_reply(c, 400, HEADER_JSON,
                      "{\"code\":-1,\"msg\":\"missing param: id\"}");
        return;
    }
    mg_http_reply(c, 200, HEADER_JSON,
                  "{\"code\":0,\"msg\":\"deleted\",\"id\":%s}", id_buf);
}

/* ---- 路由表 ---- */
static const cgi_route_t user_routes[] = {
    { "GET",    "/list",   handle_user_list   },
    { "GET",    "/detail", handle_user_detail },
    { "POST",   "/create", handle_user_create },
    { "DELETE", "/delete", handle_user_delete },
};

const cgi_module_t mod_user = {
    .prefix      = "/api/user",
    .routes      = user_routes,
    .route_count = sizeof(user_routes) / sizeof(user_routes[0]),
    .module_data = NULL,
};
