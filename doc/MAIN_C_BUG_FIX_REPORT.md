# main.c 代码问题修复报告

## 检测到的主要问题

### ❌ 问题 1: 全局变量 `g_running` 未初始化
**位置**: main.c 第21行
```c
while (g_running)
```

**问题**: 虽然在 video_server.h 中 `g_running` 被声明为 static 并初始化为 1，但由于 static 关键字的作用域限制，main.c 中无法访问到这个变量。

**影响**: 编译链接错误或未定义行为

**修复方案**: 在 main 函数开始处显式初始化
```c
g_running = 1;
```

---

### ❌ 问题 2: `get_ts_point` 返回值未检查
**位置**: main.c 第23行
```c
get_ts_point(&pt);
```

**问题**: 函数可能返回错误（如触摸屏读取失败），但未检查返回值

**影响**: 可能读取到未初始化的触摸点数据，导致程序异常

**修复方案**: 检查返回值并处理错误
```c
if (get_ts_point(&pt) < 0)
{
  perror("获取触摸点失败");
  usleep(100000);
  continue;
}
```

---

### ❌ 问题 3: `system_run` 返回值未处理
**位置**: main.c 第29行
```c
system_run(argc, argv);
```

**问题**: system_run 可能返回错误码（初始化失败等），但未检查

**影响**: 系统初始化失败后可能继续运行，导致崩溃

**修复方案**: 检查返回值并处理
```c
int ret = system_run(argc, argv);

if (ret < 0)
{
  fprintf(stderr, "系统运行失败\n");
  close_lcd();
  return -1;
}
```

---

### ❌ 问题 4: 缺少正常退出时的资源清理
**位置**: main.c 第34行（main 函数末尾）

**问题**: main 函数在退出循环后没有关闭 LCD 和返回值

**影响**: 资源泄漏，LCD 屏幕可能残留显示内容

**修复方案**: 添加清理代码
```c
// 正常退出
printf("主程序退出\n");
close_lcd();
return 0;
```

---

### ⚠️ 问题 5: video_server.h 中的全局变量定义问题
**位置**: video_server.h 第23-26行

**问题**: 在头文件中使用 `static` 定义全局变量，会导致每个包含该头文件的源文件都有自己的副本

**影响**: main.c 和 video_server.c 中的 g_running 是不同的变量，修改不会同步

**修复方案**: 需要重构 video_server.h
```c
// 方案1: 使用 extern 声明（推荐）
extern camera_t *g_camera;
extern int g_server_fd;
extern int g_running;
extern pthread_mutex_t camera_mutex;

// 在 video_server.c 中定义
camera_t *g_camera = NULL;
int g_server_fd = -1;
int g_running = 1;
pthread_mutex_t camera_mutex = PTHREAD_MUTEX_INITIALIZER;
```

---

## 完整修复后的代码

### main.c (修复后)
```c
#include "video_server.h"

int main(int argc, char *argv[])
{
  // 初始化全局变量
  g_running = 1;

  // 注册信号处理
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  printf("========================================\n");
  printf("   智能家庭视频监控系统 - 服务器端\n");
  printf("========================================\n\n");

  // 1. 初始化LCD
  printf("[1/4] 初始化LCD显示...\n");
  open_lcd();
  bmp_display("./main.bmp", 0, 0); // 清屏

  // 2. 点击"进入"按钮启动系统
  printf("点击屏幕"进入"按钮启动系统...\n");
  ts_point pt;

  while (g_running)
  {
    // 检查触摸点获取是否成功
    if (get_ts_point(&pt) < 0)
    {
      perror("获取触摸点失败");
      usleep(100000); // 100ms
      continue;
    }

    if (pt.x >= 280 && pt.x <= 520 && pt.y >= 270 && pt.y <= 460)
    {
      // 点击了"进入"按钮
      printf("系统启动中...\n");
      
      // 调用系统运行函数并检查返回值
      int ret = system_run(argc, argv);
      
      if (ret < 0)
      {
        fprintf(stderr, "系统运行失败\n");
        close_lcd();
        return -1;
      }
      
      break;
    }

    usleep(100000); // 100ms
  }

  // 正常退出
  printf("主程序退出\n");
  close_lcd();
  return 0;
}
```

---

## 修复清单

- [x] **问题 1**: 添加 `g_running` 初始化
- [x] **问题 2**: 添加 `get_ts_point` 返回值检查
- [x] **问题 3**: 添加 `system_run` 返回值处理
- [x] **问题 4**: 添加资源清理和返回值
- [ ] **问题 5**: 需要修改 video_server.h（见下一节）

---

## 下一步需要修复 video_server.h

需要创建新的 video_server.h：

```c
#ifndef __VIDEO_SERVER_H__
#define __VIDEO_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "camera.h"
#include "lcd.h"
#include "bmp.h"
#include "ts.h"

#define PORT 8888
#define MAX_CLIENTS 5
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

// 全局变量声明（使用 extern）
extern camera_t *g_camera;
extern int g_server_fd;
extern int g_running;
extern pthread_mutex_t camera_mutex;

// 数据包头结构
typedef struct
{
  unsigned int magic;      // 魔数 0x12345678
  unsigned int frame_size; // 帧数据大小
  unsigned int width;      // 图像宽度
  unsigned int height;     // 图像高度
  unsigned int format;     // 图像格式 (0=YUYV)
  unsigned int timestamp;  // 时间戳
} frame_header_t;

// 函数声明
void signal_handler(int sig);
int system_run(int argc, char *argv[]);

#endif
```

然后在 video_server.c 开头添加全局变量定义：
```c
#include "video_server.h"

// 全局变量定义
camera_t *g_camera = NULL;
int g_server_fd = -1;
int g_running = 1;
pthread_mutex_t camera_mutex = PTHREAD_MUTEX_INITIALIZER;

// ... 其余代码
```

---

## 编译说明

修复后需要更新 Makefile：

```makefile
# 添加 main.c 到服务器源文件
SERVER_SRCS = main.c video_server.c camera.c lcd.c bmp.c ts.c
```

编译命令：
```bash
make clean
make server
```

---

## 测试建议

1. **编译测试**: 确保无编译错误
2. **静态检查**: 使用 `-Wall -Wextra` 检查警告
3. **运行测试**: 
   - 正常启动测试
   - 触摸点测试
   - Ctrl+C 退出测试
   - 初始化失败测试

---

## 总结

修复后的代码：
- ✅ 正确初始化全局变量
- ✅ 完善的错误处理
- ✅ 资源正确释放
- ✅ 符合编程规范

**注意**: 问题 5（全局变量定义）是最关键的问题，必须修复才能保证程序正确运行！
