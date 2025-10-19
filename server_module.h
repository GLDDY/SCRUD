#ifndef __SERVER_MODULE_H__
#define __SERVER_MODULE_H__

#include "camera_module.h"

#define PORT 8888
#define MAX_CLIENTS 5
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

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

// 服务器模块结构
typedef struct
{
  int server_fd;
  int is_running;
  camera_module_t *camera_module;
  pthread_t display_thread;
} server_module_t;

/**
 * @brief 初始化服务器模块
 * @param camera_module 摄像头模块指针
 * @return 成功返回服务器模块指针，失败返回NULL
 */
server_module_t *server_module_init(camera_module_t *camera_module);

/**
 * @brief 启动服务器
 * @param server 服务器模块指针
 * @return 成功返回0，失败返回-1
 */
int server_module_start(server_module_t *server);

/**
 * @brief 停止服务器
 * @param server 服务器模块指针
 * @return 成功返回0，失败返回-1
 */
int server_module_stop(server_module_t *server);

/**
 * @brief 关闭服务器并释放资源
 * @param server 服务器模块指针
 */
void server_module_close(server_module_t *server);

/**
 * @brief 发送截屏帧给客户端
 * @param server 服务器模块指针
 * @return 成功返回0，失败返回-1
 */
int server_module_send_capture(server_module_t *server);

/**
 * @brief 客户端处理线程包装器
 * @param arg 客户端socket指针
 * @return NULL
 */
void *client_handler_wrapper(void *arg);

#endif // __SERVER_MODULE_H__
