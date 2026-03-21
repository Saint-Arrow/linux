/**
 * main.c - Mongoose CGI Server 入口
 *
 * 职责:
 *   1. 初始化 Mongoose 事件管理器
 *   2. 注册所有业务模块
 *   3. 启动 HTTP 监听
 *   4. 事件循环
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "mongoose.h"
#include "route.h"
#include "common/cgi_util.h"

/* ---- 外部模块声明 ---- */
extern const cgi_module_t mod_user;
extern const cgi_module_t mod_device;
extern const cgi_module_t mod_system;

/* ---- 默认配置 ---- */
#define DEFAULT_LISTEN_URL   "http://0.0.0.0:8080"
#define DEFAULT_STATIC_ROOT  "www"

static const char *s_listen_url = DEFAULT_LISTEN_URL;
static const char *s_static_root = DEFAULT_STATIC_ROOT;

static volatile int s_running = 1;

static void signal_handler(int sig) {
    (void)sig;
    s_running = 0;
}

/* ---- HTTP 事件回调 ---- */
static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    /* 打印访问日志 */
    CGI_INFO("%.*s %.*s",
             (int)hm->method.len, hm->method.buf,
             (int)hm->uri.len, hm->uri.buf);

    /* 1) CORS preflight */
    if (cgi_is_preflight(hm)) {
        cgi_handle_preflight(c);
        return;
    }

    /* 2) 尝试路由分发 */
    if (router_dispatch(c, hm)) return;

    /* 3) 未命中路由 -> 静态文件服务 */
    struct mg_http_serve_opts opts = {
        .root_dir = s_static_root,
        .ssi_pattern = NULL,
    };
    mg_http_serve_dir(c, hm, &opts);
}

static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -l <url>    Listen URL (default: %s)\n", DEFAULT_LISTEN_URL);
    printf("  -r <dir>    Static root directory (default: %s)\n", DEFAULT_STATIC_ROOT);
    printf("  -h          Show this help\n");
}

static int parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return -1;
        } else if (strcmp(argv[i], "-l") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -l requires an argument\n");
                return -1;
            }
            s_listen_url = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -r requires an argument\n");
                return -1;
            }
            s_static_root = argv[++i];
        } else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    /* 解析命令行参数 */
    if (parse_args(argc, argv) != 0) {
        return 1;
    }

    /* 信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 初始化路由 */
    router_init();

    /* 注册业务模块 */
    router_register(&mod_user);
    router_register(&mod_device);
    router_register(&mod_system);

    /* 初始化 Mongoose */
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    struct mg_connection *listener = mg_http_listen(&mgr, s_listen_url,
                                                    ev_handler, NULL);
    if (listener == NULL) {
        CGI_ERR("Failed to listen on %s", s_listen_url);
        return 1;
    }

    printf("========================================\n");
    printf("  Mongoose CGI Server started\n");
    printf("  Listening on %s\n", s_listen_url);
    printf("  Static root: %s\n", s_static_root);
    printf("========================================\n");
    printf("Registered routes:\n");
    printf("  /api/user   -> mod_user\n");
    printf("  /api/device -> mod_device\n");
    printf("  /api/sys    -> mod_system\n");
    printf("  /*          -> static files\n");
    printf("========================================\n");

    /* 事件循环 */
    while (s_running) {
        mg_mgr_poll(&mgr, 1000);
    }

    printf("\nShutting down...\n");
    mg_mgr_free(&mgr);
    return 0;
}
