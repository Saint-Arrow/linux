/**
 * route.h - CGI 路由表与模块结构定义
 *
 * 每个业务模块定义一组 cgi_route_t 路由条目，
 * 并通过 cgi_module_t 注册到路由分发器中。
 */
#ifndef ROUTE_H
#define ROUTE_H

#include "mongoose.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 路由处理函数签名 ---- */
typedef void (*cgi_handler_fn)(struct mg_connection *c,
                               struct mg_http_message *hm,
                               void *user_data);

/* ---- 单条路由 ---- */
typedef struct {
    const char *method;     /* "GET" / "POST" / "PUT" / "DELETE", NULL=任意方法 */
    const char *uri;        /* 子路径匹配模式, 如 "/list", "/detail/xx" */
    cgi_handler_fn handler; /* 处理函数 */
} cgi_route_t;

/* ---- 路由模块 ---- */
typedef struct {
    const char *prefix;           /* 模块前缀, 如 "/api/user" */
    const cgi_route_t *routes;    /* 该模块下的路由数组 */
    int route_count;              /* 路由条目数 */
    void *module_data;            /* 模块私有数据 (DB句柄等) */
} cgi_module_t;

/* ---- 路由分发器 API ---- */
#define ROUTER_MAX_MODULES 64

void router_init(void);
int  router_register(const cgi_module_t *mod);
bool router_dispatch(struct mg_connection *c, struct mg_http_message *hm);

#ifdef __cplusplus
}
#endif

#endif /* ROUTE_H */
