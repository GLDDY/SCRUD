# 视频监控系统 - 项目文件索引

## 📁 项目文件结构

```
src/
├── 📋 核心功能源码
│   ├── camera.h              # 摄像头接口头文件
│   ├── camera.c              # V4L2摄像头驱动实现 (350行)
│   ├── video_server.c        # 服务器端主程序 (250行)
│   ├── video_client.c        # 客户端主程序 (200行)
│   ├── lcd.h                 # LCD显示接口
│   ├── lcd.c                 # LCD显示实现
│   ├── bmp.h                 # BMP图片处理接口
│   ├── bmp.c                 # BMP图片处理实现
│   ├── ts.h                  # 触摸屏接口
│   └── ts.c                  # 触摸屏实现
│
├── 🔧 编译与测试
│   ├── Makefile              # 编译脚本
│   └── test_system.sh        # 测试脚本 (可执行)
│
├── 📚 文档
│   ├── VIDEO_MONITOR_README.md    # 详细技术文档 (完整版)
│   ├── DEPLOYMENT_GUIDE.md        # 部署指南 (快速上手)
│   ├── PROJECT_SUMMARY.md         # 项目总结 (答辩用)
│   ├── FILE_INDEX.md              # 本文件索引
│   └── 23级通信工程《网络程序设计实训》任务书.md
│
├── 🎯 其他模块 (可选功能)
│   ├── module.h              # 扩展模块接口
│   ├── module.c              # 扩展模块实现
│   ├── gy39.h / gy39.c       # GY-39气象传感器
│   ├── mplayer.c             # MPlayer视频播放器
│   ├── main.c                # 主菜单程序
│   ├── server.c / client.c   # 通用网络程序示例
│   └── ...
│
└── 🖼️ 资源文件 (需要准备)
    ├── blank.bmp             # 空白图片 (清屏用)
    ├── menu.bmp              # 主菜单背景
    └── /dev/video0           # 摄像头设备
```

## 📝 文件说明

### 核心源码文件

#### camera.h / camera.c
**功能**: V4L2摄像头驱动  
**关键函数**:
- `camera_init()` - 初始化摄像头
- `camera_start()` - 开始采集
- `camera_get_frame()` - 获取一帧
- `camera_display()` - 显示到LCD
- `yuyv_to_rgb888()` - 颜色转换

**依赖**: linux/videodev2.h, lcd.h

#### video_server.c
**功能**: 服务器端主程序  
**特性**:
- TCP服务器 (端口8888)
- 多线程支持多客户端
- 本地LCD实时显示
- 线程安全的摄像头访问

**编译**: `make server`  
**运行**: `./video_server`

#### video_client.c
**功能**: 客户端主程序  
**特性**:
- TCP客户端
- 接收视频流
- 保存PPM图像
- 实时统计信息

**编译**: `make client`  
**运行**: `./video_client <IP> <PORT>`

#### lcd.h / lcd.c
**功能**: LCD显示驱动  
**关键函数**:
- `open_lcd()` - 打开LCD设备
- `close_lcd()` - 关闭LCD
- `display_point()` - 显示单个像素

**依赖**: /dev/fb0 framebuffer设备

#### bmp.h / bmp.c
**功能**: BMP图片处理  
**关键函数**:
- `bmp_display()` - 显示BMP图片

**支持**: 24位真彩色BMP

#### ts.h / ts.c
**功能**: 触摸屏驱动  
**关键函数**:
- `get_ts_point()` - 获取触摸点
- `get_ts_direction()` - 获取滑动方向

### 编译与测试文件

#### Makefile
**用途**: 自动化编译脚本  
**命令**:
```bash
make help    # 查看帮助
make server  # 编译服务器 (ARM)
make client  # 编译客户端 (x86)
make both    # 同时编译
make clean   # 清理
make deploy  # 部署到开发板
```

#### test_system.sh
**用途**: 系统测试脚本  
**功能**:
- 检查编译工具
- 检查源文件
- 自动编译
- 验证可执行文件

**运行**:
```bash
chmod +x test_system.sh
./test_system.sh
```

### 文档文件

#### VIDEO_MONITOR_README.md
**内容**:
- 系统架构详解
- 技术实现细节
- API接口说明
- 性能参数
- 扩展功能建议

**适用**: 技术学习、代码理解

#### DEPLOYMENT_GUIDE.md
**内容**:
- 快速部署步骤
- 环境配置
- 常见问题解决
- 网络配置
- 自动启动设置

**适用**: 快速上手、部署运维

#### PROJECT_SUMMARY.md
**内容**:
- 项目完成情况
- 技术难点分析
- 测试结果
- 学习收获
- 答辩要点

**适用**: 项目总结、答辩准备

## 🔄 文件依赖关系

```
video_server.c
    ├── camera.h/c         (V4L2采集)
    ├── lcd.h/c            (本地显示)
    ├── bmp.h/c            (清屏/背景)
    └── <pthread.h>        (多线程)

video_client.c
    └── <socket相关头文件>  (网络接收)

camera.c
    ├── lcd.h              (显示功能)
    └── <linux/videodev2.h> (V4L2接口)

lcd.c
    └── <sys/mman.h>       (内存映射)

bmp.c
    └── lcd.h              (显示输出)
```

## 📊 代码量统计

| 文件           | 行数      | 功能     | 难度 |
| -------------- | --------- | -------- | ---- |
| camera.c       | ~350      | V4L2驱动 | ⭐⭐⭐⭐ |
| video_server.c | ~250      | 服务器   | ⭐⭐⭐⭐ |
| video_client.c | ~200      | 客户端   | ⭐⭐⭐  |
| bmp.c          | ~150      | BMP解析  | ⭐⭐   |
| lcd.c          | ~100      | LCD显示  | ⭐⭐   |
| ts.c           | ~100      | 触摸屏   | ⭐⭐   |
| **总计**       | **~1150** | -        | -    |

## 🎯 快速导航

### 我想...

**...理解系统架构**
→ 阅读 `VIDEO_MONITOR_README.md` 第二节

**...快速部署运行**
→ 阅读 `DEPLOYMENT_GUIDE.md` 快速开始

**...修改摄像头参数**
→ 编辑 `camera.c` 中的 `camera_init()` 函数

**...修改网络端口**
→ 编辑 `video_server.c` 中的 `#define PORT 8888`

**...调整帧率**
→ 编辑 `video_server.c` 中的 `usleep()` 参数

**...添加H.264编码**
→ 参考 `VIDEO_MONITOR_README.md` 扩展功能章节

**...准备答辩**
→ 阅读 `PROJECT_SUMMARY.md`

**...解决编译问题**
→ 阅读 `DEPLOYMENT_GUIDE.md` 常见问题

## 🔍 核心代码位置

### V4L2采集核心代码
**文件**: camera.c  
**函数**: `camera_init()` (第20-150行)  
**关键**: mmap内存映射、VIDIOC_QBUF/DQBUF

### 颜色转换核心代码
**文件**: camera.c  
**函数**: `yuyv_to_rgb888()` (第200-240行)  
**关键**: YUV到RGB转换公式

### TCP服务器核心代码
**文件**: video_server.c  
**函数**: `client_handler()` (第50-120行)  
**关键**: 帧头打包、数据发送

### TCP客户端核心代码
**文件**: video_client.c  
**函数**: `main()` 接收循环 (第100-180行)  
**关键**: recv_full完整接收

## 📦 编译产物

```
编译后生成:
├── video_server          # ARM可执行文件 (约200KB)
├── video_client          # x86可执行文件 (约150KB)
├── *.o                   # 目标文件 (编译中间产物)
└── frame_*.ppm           # 客户端保存的图像
```

## 🚀 快速命令参考

```bash
# 编译
make server && make client

# 测试
./test_system.sh

# 部署
scp video_server root@192.168.1.100:/root/

# 运行服务器 (开发板上)
./video_server

# 运行客户端 (PC上)
./video_client 192.168.1.100 8888

# 查看保存的图像
ls frame_*.ppm

# 清理
make clean
```

## 📞 技术支持

遇到问题时的查找顺序:

1. **编译问题** → DEPLOYMENT_GUIDE.md 常见问题
2. **运行问题** → VIDEO_MONITOR_README.md 故障排查
3. **代码理解** → 源码注释 + VIDEO_MONITOR_README.md
4. **答辩准备** → PROJECT_SUMMARY.md
5. **其他问题** → 咨询指导老师

## 📌 重要提示

1. **必须文件**: camera.c/h, video_server.c, video_client.c, lcd.c/h, bmp.c/h
2. **可选文件**: ts.c/h, module.c/h (扩展功能)
3. **必备资源**: blank.bmp (清屏图片), /dev/video0 (摄像头)
4. **网络配置**: 确保开发板和PC在同一网段
5. **权限问题**: 可能需要root权限访问设备

## 🎓 学习路径建议

```
第一天: 理解系统架构
  └─ 阅读 VIDEO_MONITOR_README.md

第二天: 编译部署运行
  └─ 跟随 DEPLOYMENT_GUIDE.md 操作

第三天: 代码细节学习
  ├─ camera.c (V4L2)
  ├─ video_server.c (TCP服务器)
  └─ video_client.c (TCP客户端)

第四天: 调试优化
  ├─ 调整参数
  ├─ 性能测试
  └─ 问题解决

第五天: 答辩准备
  └─ PROJECT_SUMMARY.md + 演示准备
```

---

**最后更新**: 2025年10月18日  
**项目版本**: v1.0  
**状态**: ✅ 完成并测试通过
