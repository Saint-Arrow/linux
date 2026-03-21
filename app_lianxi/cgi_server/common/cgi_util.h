/**
 * cgi_util.h - CGI 公共工具
 *
 * 提供通用响应头、日志宏、常用辅助函数。
 */
#ifndef CGI_UTIL_H
#define CGI_UTIL_H

#include "../mongoose.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 通用响应头 ---- */
#define HEADER_JSON \
    "Content-Type: application/json\r\n" \
    "Access-Control-Allow-Origin: *\r\n"

#define HEADER_TEXT \
    "Content-Type: text/plain; charset=utf-8\r\n" \
    "Access-Control-Allow-Origin: *\r\n"

/* ---- 日志宏 ---- */
#define CGI_LOG(level, fmt, ...) \
    fprintf(stderr, "[%s] " fmt "\n", level, ##__VA_ARGS__)

#define CGI_INFO(fmt, ...)  CGI_LOG("INFO",  fmt, ##__VA_ARGS__)
#define CGI_WARN(fmt, ...)  CGI_LOG("WARN",  fmt, ##__VA_ARGS__)
#define CGI_ERR(fmt, ...)   CGI_LOG("ERROR", fmt, ##__VA_ARGS__)

/* ---- 辅助函数 ---- */

/**
 * 从 query string 中提取参数, 失败返回空字符串
 * 调用者提供 buf 和 buf_size
 */
static inline const char *cgi_get_param(struct mg_http_message *hm,
                                        const char *name,
                                        char *buf, size_t buf_size) {
    int len = mg_http_get_var(&hm->query, name, buf, (size_t)buf_size);
    if (len <= 0) buf[0] = '\0';
    return buf;
}

/**
 * 返回标准 JSON 错误响应
 */
static inline void cgi_reply_error(struct mg_connection *c,
                                   int status, int code,
                                   const char *msg) {
    mg_http_reply(c, status, HEADER_JSON,
                  "{\"code\":%d,\"msg\":\"%s\"}", code, msg);
}

/**
 * 判断请求是否为 OPTIONS (CORS preflight)
 */
static inline bool cgi_is_preflight(struct mg_http_message *hm) {
    return mg_strcmp(hm->method, mg_str("OPTIONS")) == 0;
}

/**
 * 处理 CORS preflight 请求
 */
static inline void cgi_handle_preflight(struct mg_connection *c) {
    mg_http_reply(c, 204,
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS\r\n"
                  "Access-Control-Allow-Headers: Content-Type,Authorization\r\n"
                  "Access-Control-Max-Age: 86400\r\n",
                  "");
}

#ifdef __cplusplus
}
#endif

#endif /* CGI_UTIL_H */
