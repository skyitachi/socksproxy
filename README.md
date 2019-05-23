### Introduction

- a socks server written in c++

### TODO
- [x] 加入http的request_header的解析(使用http-parser）
- [ ] 加入response_header的解析
- [ ] 增加错误处理
   - [x] connection refused, proxy 主动关闭
   - dns cannot resolve
   - [x] SOCKS4 connection to ::1 not supported
- [x] 加入了定时器，主动关闭客户端的连接
- [ ] 支持https协议的解析
- [ ] 加入google test
- [ ] 完善 test-case
- [ ] 内存管理
    - [x] Connection 自己资源的释放

### 依赖管理
- http-parser, 包含在了deps目录下
- libuv, 系统依赖
- google test， 系统依赖


### test case
- [x] 客户端主动关闭连接(在服务端数据没有完全接受完情况)
- [x] 服务端连接不上(客户端已经连上)的场景

### 测试脚本
```shell
curl --proxy socks4://localhost:12321 ${http_server}
```
