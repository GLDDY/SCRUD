# 代码重构说明文档

## 重构目标

本次重构的主要目标是：
1. **模块分离**：将摄像头模块与服务器模块完全分离
2. **截屏功能**：实现按钮触发的截屏功能，手动发送帧给客户端
3. **退出功能**：实现退出按钮，可以优雅地退出视频监控系统

## 重构架构

### 新的文件结构

```
src/
├── main.c                  # 主程序入口
├── module.c                # 视频监控模块集成
├── module.h                # 模块头文件
├── camera_module.c         # 摄像头模块实现（新增）
├── camera_module.h         # 摄像头模块头文件（新增）
├── server_module.c         # 服务器模块实现（新增）
├── server_module.h         # 服务器模块头文件（新增）
├── utils.c                 # 工具函数（新增）
├── common.h                # 公共头文件
├── camera.c/h              # 原摄像头驱动
├── lcd.c/h                 # LCD显示驱动
├── bmp.c/h                 # BMP图像处理
├── ts.c/h                  # 触摸屏驱动
└── Makefile                # 编译配置（已更新）
```

## 核心模块说明

### 1. 摄像头模块（camera_module）

**职责**：
- 封装摄像头设备的管理
- 提供线程安全的帧获取接口
- 实现截屏功能（保存当前帧）

**主要函数**：
```c
camera_module_t* camera_module_init(const char *dev_name, int width, int height);
int camera_module_start(camera_module_t *cam_module);
int camera_module_stop(camera_module_t *cam_module);
void camera_module_close(camera_module_t *cam_module);
int camera_module_get_frame(camera_module_t *cam_module, unsigned char **yuyv_data, unsigned int *data_size);
int camera_module_release_frame(camera_module_t *cam_module);
int camera_module_display(camera_module_t *cam_module, int x0, int y0);
int camera_module_capture_frame(camera_module_t *cam_module, unsigned char **yuyv_data, unsigned int *data_size);
```

**特点**：
- 使用互斥锁保护共享资源
- 独立于服务器模块，可单独使用
- 支持截屏缓存

### 2. 服务器模块（server_module）

**职责**：
- 管理TCP服务器连接
- 维护客户端连接列表
- 实现截屏帧的广播发送
- 管理本地显示线程

**主要函数**：
```c
server_module_t* server_module_init(camera_module_t *camera_module);
int server_module_start(server_module_t *server);
int server_module_stop(server_module_t *server);
void server_module_close(server_module_t *server);
int server_module_send_capture(server_module_t *server);
void *client_handler_wrapper(void *arg);
```

**特点**：
- 不再自动发送视频流
- 维护客户端连接列表
- 支持按需截屏广播

### 3. 视频监控模块（module.c）

**职责**：
- 整合摄像头模块和服务器模块
- 管理触摸屏交互
- 协调系统运行

**主要功能**：
```c
int video_monitor(int *flag, int argc, char *argv[]);
```

**工作流程**：
1. 初始化摄像头模块
2. 初始化服务器模块
3. 启动本地显示线程
4. 启动服务器接受连接线程
5. 启动触摸屏控制线程
6. 等待退出信号
7. 清理所有资源

## 功能改进

### 1. 截屏功能

**触发方式**：
- 点击屏幕右侧上方区域（x: 640-800, y: 0-240）

**工作流程**：
```
用户点击截屏按钮
    ↓
触摸屏控制线程检测到
    ↓
调用 server_module_send_capture()
    ↓
调用 camera_module_capture_frame() 获取当前帧
    ↓
复制帧数据到缓存
    ↓
遍历所有客户端连接
    ↓
发送数据包头 + 图像数据
    ↓
完成截屏发送
```

**优势**：
- 按需发送，节省带宽
- 用户主动控制
- 支持多客户端广播

### 2. 退出功能

**触发方式**：
- 点击屏幕右侧下方区域（x: 640-800, y: 240-480）

**工作流程**：
```
用户点击退出按钮
    ↓
触摸屏控制线程检测到
    ↓
设置退出标志
    ↓
服务器停止接受新连接
    ↓
关闭所有客户端连接
    ↓
停止本地显示线程
    ↓
停止摄像头模块
    ↓
清理所有资源
    ↓
返回主菜单
```

**优势**：
- 优雅退出
- 完整的资源清理
- 无内存泄漏

### 3. 取消自动发送

**旧版本**：
- 客户端连接后自动以30fps发送视频流
- 持续占用带宽
- 无法控制发送时机

**新版本**：
- 客户端连接后仅保持连接
- 仅在用户点击截屏时发送
- 节省带宽和CPU资源

## 线程架构

### 线程列表

1. **主线程**（main.c）
   - 显示开始界面
   - 等待用户启动系统
   - 调用 video_monitor()

2. **触摸屏控制线程**（module.c）
   - 检测触摸屏事件
   - 处理截屏和退出按钮

3. **服务器接受连接线程**（module.c）
   - 接受客户端连接
   - 为每个客户端创建处理线程

4. **客户端处理线程**（server_module.c）
   - 维护客户端连接
   - 检测客户端断开

5. **本地显示线程**（server_module.c）
   - 持续在LCD上显示摄像头画面
   - 约20fps刷新率

### 线程同步

- **摄像头访问**：使用 camera_module 中的互斥锁
- **客户端列表**：使用 g_client_mutex 保护
- **退出信号**：使用全局标志 g_system_running

## 编译和部署

### 编译命令

```bash
# 编译服务器端（ARM）
make server

# 编译客户端（PC）
make client

# 同时编译
make both

# 清理
make clean
```

### 部署到开发板

```bash
make deploy BOARD_IP=192.168.1.100
```

### 运行

**开发板（服务器端）**：
```bash
./video_server
```

**PC（客户端）**：
```bash
./video_client 192.168.1.100 8888
```

## 使用说明

### 服务器端操作

1. **启动系统**
   - 运行程序后显示开始界面
   - 点击"进入"按钮启动视频监控

2. **截屏操作**
   - 点击屏幕右侧上方区域
   - 系统自动捕获当前帧并发送给所有客户端

3. **退出系统**
   - 点击屏幕右侧下方区域
   - 系统优雅退出，返回主菜单

### 客户端操作

1. **连接服务器**
   - 运行客户端程序，指定服务器IP和端口
   - 连接成功后等待接收截屏

2. **接收截屏**
   - 服务器端点击截屏后自动接收
   - 显示在窗口中

3. **断开连接**
   - 关闭客户端窗口即可

## 优势总结

### 1. 模块化设计
- 摄像头和服务器完全分离
- 易于维护和扩展
- 可独立测试

### 2. 资源优化
- 不再持续发送视频流
- 按需发送，节省带宽
- 降低CPU和内存占用

### 3. 用户控制
- 用户主动触发截屏
- 明确的退出机制
- 更好的交互体验

### 4. 线程安全
- 完善的锁机制
- 避免竞态条件
- 优雅的线程退出

### 5. 可扩展性
- 易于添加新功能
- 模块接口清晰
- 支持多种使用场景

## 注意事项

1. **UI界面**：
   - 需要准备 `ui.bmp` 文件作为监控界面
   - 按钮区域坐标需根据实际UI调整

2. **设备路径**：
   - 摄像头设备默认为 `/dev/video7`
   - 可根据实际情况修改

3. **端口配置**：
   - 默认端口为 8888
   - 可在 server_module.h 中修改

4. **线程安全**：
   - 所有对摄像头的访问都需要通过 camera_module 接口
   - 避免直接访问 camera_t 结构

## 后续优化建议

1. **配置文件支持**：
   - 添加配置文件读取功能
   - 支持动态配置端口、分辨率等参数

2. **日志系统**：
   - 添加完善的日志记录
   - 便于调试和问题排查

3. **错误恢复**：
   - 添加摄像头异常恢复机制
   - 网络断开自动重连

4. **性能监控**：
   - 添加帧率监控
   - CPU和内存使用统计

5. **更多功能**：
   - 录像功能
   - 图像处理（如滤镜、亮度调整）
   - 多摄像头支持
