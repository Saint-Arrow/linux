/**
 * router.c - 路由分发器实现
 *
 * 负责模块注册、URI 前缀匹配、子路径路由分发。
 */
#include <string.h>
#include <stdio.h>
#include "route.h"

/* ---- 全局模块注册表 ---- */
static const cgi_module_t *g_modules[ROUTER_MAX_MODULES];
static int g_module_count = 0;

void router_init(void) {
    g_module_count = 0;
    memset(g_modules, 0, sizeof(g_modules));
}

int router_register(const cgi_module_t *mod) {
    if (g_module_count >= ROUTER_MAX_MODULES) {
        fprintf(stderr, "[router] module limit reached (%d)\n",
                ROUTER_MAX_MODULES);
        return -1;
    }
    printf("[router] register module: %s (%d routes)\n",
           mod->prefix, mod->route_count);
    g_modules[g_module_count++] = mod;
    return 0;
}

/* URI 前缀匹配 */
static bool starts_with(struct mg_str uri, const char *prefix) {
    size_t len = strlen(prefix);
    return uri.len >= len && memcmp(uri.buf, prefix, len) == 0;
}

bool router_dispatch(struct mg_connection *c, struct mg_http_message *hm) {
    for (int i = 0; i < g_module_count; i++) {
        const cgi_module_t *mod = g_modules[i];
        if (!starts_with(hm->uri, mod->prefix)) continue;

        /* 取模块前缀之后的子路径 */
        size_t plen = strlen(mod->prefix);
        struct mg_str sub = mg_str_n(hm->uri.buf + plen,
                                     hm->uri.len - plen);

        /* 如果子路径为空, 当作 "/" 处理 */
        if (sub.len == 0) {
            sub = mg_str("/");
        }

        for (int j = 0; j < mod->route_count; j++) {
            const cgi_route_t *r = &mod->routes[j];

            /* 方法匹配 (NULL = 匹配任意方法) */
            if (r->method != NULL &&
                mg_strcmp(hm->method, mg_str(r->method)) != 0) {
                continue;
            }

            /* 子路径匹配, 支持 mg_match 通配符 (* / ? / #) */
            if (mg_match(sub, mg_str(r->uri), NULL)) {
                r->handler(c, hm, mod->module_data);
                return true;
            }
        }
    }
    return false; /* 未匹配任何路由 */
}
