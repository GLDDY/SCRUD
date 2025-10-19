#include "common.h"

// 全局变量定义
camera_t *g_camera = NULL;                                // 指向摄像头设备结构体
int g_server_fd = -1;                                     // 服务器
int g_running = 1;                                        // 运行状态
pthread_mutex_t camera_mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥锁变量

/**
 * @brief 信号处理函数
 */
void signal_handler(int sig)
{
  printf("\n接收到信号 %d, 正在关闭程序...\n", sig);
  // 通过全局变量控制程序退出
  g_running = 0;

  // 注意：不要在这里关闭socket，会导致重复关闭
  // socket的关闭由各个模块自己负责
}

/**
 * @brief 返回主菜单
 */
void back_menu(void)
{
  clear_screen();
  bmp_display("./main.bmp", 0, 0);
}

/**
 * @brief 清屏
 */
void clear_screen(void)
{
  bmp_display("./blank.bmp", 0, 0);
}
