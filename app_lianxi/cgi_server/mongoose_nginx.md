
## Mongoose 的优势

### 1. **轻量级嵌入式**
- 单文件库（`mongoose.c` + `mongoose.h`），代码量小
- 零依赖或极少依赖，适合嵌入式设备
- 本项目使用 C99 标准，跨平台支持（Linux/Windows）

### 2. **功能完整**
- 内置 HTTP/HTTPS 服务器
- 支持 WebSocket
- 支持 CGI 处理（本项目基于此）
- 内置静态文件服务

### 3. **易于集成**
- 直接嵌入到现有 C/C++ 项目
- 无需额外安装或配置
- 事件驱动架构，资源占用低

---

## 为什么生产环境要配合 Nginx

| 场景 | Mongoose 单独 | Nginx + Mongoose |
|------|--------------|------------------|
| **静态文件** | 功能简单，性能一般 | Nginx 高性能静态文件服务 |
| **并发处理** | 单线程事件循环，并发有限 | Nginx 异步 + 多 worker，高并发 |
| **负载均衡** | 不支持 | Nginx 内置负载均衡 |
| **SSL/TLS** | 基础支持 | Nginx 更完善的 HTTPS 配置 |
| **安全防护** | 基础 | Nginx 防 DDoS、限流、WAF 等 |
| **反向代理** | 不支持 | Nginx 反向代理到 Mongoose |

### 推荐架构

```
客户端 → Nginx (80/443端口)
           ↓
    ┌──────┴──────┐
    ↓             ↓
静态文件      反向代理
(直接返回)    → Mongoose (127.0.0.1:8080)
                  ↓
              CGI 业务处理
```

### 配置示例

```nginx
server {
    listen 80;
    server_name example.com;

    # 静态文件由 Nginx 直接处理
    location /static/ {
        alias /var/www/static/;
    }

    # API 请求反向代理到 Mongoose
    location /api/ {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

---

## 总结

- **Mongoose**：适合嵌入式场景、快速开发、业务逻辑处理
- **Nginx**：适合高并发、静态资源、安全防护、反向代理
- **配合方案**：Nginx 作为入口网关，Mongoose 专注 CGI 业务处理