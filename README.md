### Introduction

- a socks server written in c++

### TODO
- [x] 加入http的request_header的解析(使用http-parser）
- [ ] 加入response_header的解析
- [ ] 增加错误处理
- [x] 加入了定时器，主动关闭客户端的连接
- [ ] 支持https协议的解析
- [ ] 加入google test
- [ ] 完善 test-case
- [ ] Connection 自己资源的释放

### 依赖管理
- http-parser, 包含在了deps目录下
- libuv, 系统依赖
- google test， 系统依赖


### test case
- [x] 客户端主动关闭连接(在服务端数据没有完全接受完情况)