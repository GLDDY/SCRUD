#include "common.h"
#include "camera_module.h"
#include "server_module.h"
#include <pthread.h>

// 全局模块指针
static camera_module_t *g_cam_module = NULL;
static server_module_t *g_srv_module = NULL;
static int g_system_running = 0;

/**
 * @brief 触摸屏控制线程
 */
void *touch_control_thread()
{
  ts_point pt;

  printf("触摸屏控制线程启动\n");

  while (g_system_running)
  {
    get_ts_point(&pt);
    printf("检测到触摸: x=%d, y=%d\n", pt.x, pt.y);

    // 退出按钮区域
    if (pt.x >= 640 && pt.x <= 800 && pt.y >= 240 && pt.y <= 480)
    {
      printf("点击了'退出'按钮\n");
      g_system_running = 0;

      // 强制停止服务器，中断accept阻塞
      if (g_srv_module && g_srv_module->server_fd >= 0)
      {
        shutdown(g_srv_module->server_fd, SHUT_RDWR);
      }

      // ❌ 不要在这里调用back_menu()，会导致段错误
      // back_menu()应该在资源清理完成后由主函数调用
      break;
    }
    // 截屏按钮区域
    else if (pt.x >= 640 && pt.x <= 800 && pt.y >= 0 && pt.y <= 240)
    {
      printf("点击了'截屏'按钮\n");
      if (g_srv_module)
      {
        server_module_send_capture(g_srv_module);
      }
    }

    usleep(100000); // 100ms，避免过于频繁检测
  }

  printf("触摸屏控制线程退出\n");
  return NULL;
}

/**
 * @brief 服务器接受连接线程
 */
void *server_accept_thread(void *arg)
{
  server_module_t *server = (server_module_t *)arg;

  printf("服务器接受连接线程启动\n");

  // 主循环: 接受客户端连接
  while (g_system_running && server->is_running)
  {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int *client_sock = malloc(sizeof(int));
    if (!client_sock)
    {
      continue;
    }

    *client_sock = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_len);

    if (*client_sock < 0)
    {
      // accept被中断，检查是否需要退出
      if (!g_system_running || !server->is_running)
      {
        printf("服务器正在关闭，停止接受新连接\n");
        free(client_sock);
        break;
      }
      perror("accept失败");
      free(client_sock);
      continue;
    }

    // 为每个客户端创建处理线程
    pthread_t tid;
    if (pthread_create(&tid, NULL, client_handler_wrapper, client_sock) != 0)
    {
      perror("创建客户端处理线程失败");
      close(*client_sock);
      free(client_sock);
      continue;
    }
    pthread_detach(tid); // 分离线程,自动回收资源
  }

  printf("服务器接受连接线程退出\n");
  return NULL;
}

/**
 * @brief 视频监控主函数
 */
int video_monitor(int argc, char *argv[])
{
  printf("========================================\n");
  printf("   启动视频监控系统\n");
  printf("========================================\n");

  // 显示UI界面
  bmp_display("./ui.bmp", 0, 0);

  // 1. 初始化摄像头模块
  printf("[1/3] 初始化摄像头模块...\n");
  g_cam_module = camera_module_init("/dev/video7", FRAME_WIDTH, FRAME_HEIGHT);
  if (!g_cam_module)
  {
    fprintf(stderr, "摄像头模块初始化失败\n");
    return -1;
  }

  if (camera_module_start(g_cam_module) < 0)
  {
    fprintf(stderr, "摄像头模块启动失败\n");
    camera_module_close(g_cam_module);
    return -1;
  }

  // 2. 初始化服务器模块
  printf("[2/3] 初始化服务器模块...\n");
  g_srv_module = server_module_init(g_cam_module);
  if (!g_srv_module)
  {
    fprintf(stderr, "服务器模块初始化失败\n");
    camera_module_stop(g_cam_module);
    camera_module_close(g_cam_module);
    return -1;
  }

  if (server_module_start(g_srv_module) < 0)
  {
    fprintf(stderr, "服务器模块启动失败\n");
    server_module_close(g_srv_module);
    camera_module_stop(g_cam_module);
    camera_module_close(g_cam_module);
    return -1;
  }

  // 3. 启动触摸屏控制线程
  printf("[3/3] 启动触摸屏控制线程...\n");
  g_system_running = 1;
  pthread_t touch_tid, server_tid;

  if (pthread_create(&touch_tid, NULL, touch_control_thread, NULL) != 0)
  {
    perror("创建触摸屏控制线程失败");
    server_module_stop(g_srv_module);
    server_module_close(g_srv_module);
    camera_module_stop(g_cam_module);
    camera_module_close(g_cam_module);
    return -1;
  }

  // 4. 启动服务器接受连接线程
  if (pthread_create(&server_tid, NULL, server_accept_thread, g_srv_module) != 0)
  {
    perror("创建服务器接受连接线程失败");
    g_system_running = 0;
    pthread_join(touch_tid, NULL);
    server_module_stop(g_srv_module);
    server_module_close(g_srv_module);
    camera_module_stop(g_cam_module);
    camera_module_close(g_cam_module);
    return -1;
  }

  printf("========================================\n");
  printf("系统运行中...\n");
  printf("点击'截屏'按钮发送当前帧给客户端\n");
  printf("点击'退出'按钮返回主菜单\n");
  printf("按 Ctrl+C 也可以退出系统\n");
  printf("========================================\n\n");

  // 监控全局退出信号（Ctrl+C）
  // extern int g_running;
  // while (g_system_running && g_running)
  // {
  //   usleep(100000); // 100ms

  //   // 检查Ctrl+C信号
  //   if (!g_running)
  //   {
  //     printf("\n检测到Ctrl+C信号，正在退出...\n");
  //     g_system_running = 0;

  //     // 强制停止服务器
  //     if (g_srv_module && g_srv_module->server_fd >= 0)
  //     {
  //       shutdown(g_srv_module->server_fd, SHUT_RDWR);
  //     }
  //     break;
  //   }
  // }

  // 等待线程退出（设置超时避免永久阻塞）
  printf("等待线程退出...\n");

  // 给线程一些时间自然退出
  usleep(200000); // 200ms

  // 如果系统正在退出，尝试取消还未退出的线程
  // 注意：避免在线程正在执行关键操作时取消
  void *retval;
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 2; // 2秒超时

  // 等待触摸线程退出（不强制取消，因为它可能已经退出）
  int ret = pthread_join(touch_tid, &retval);
  if (ret != 0)
  {
    printf("警告：触摸线程退出异常\n");
  }

  // 等待服务器线程退出
  ret = pthread_join(server_tid, &retval);
  if (ret != 0)
  {
    printf("警告：服务器线程退出异常\n");
  }

  printf("所有线程已退出\n");

  printf("所有线程已退出\n");

  // 清理资源
  printf("\n正在清理资源...\n");

  // 先停止服务器（这会关闭socket和显示线程）
  if (g_srv_module)
  {
    server_module_stop(g_srv_module);
    server_module_close(g_srv_module);
    g_srv_module = NULL;
  }

  // 再停止摄像头
  if (g_cam_module)
  {
    camera_module_stop(g_cam_module);
    camera_module_close(g_cam_module);
    g_cam_module = NULL;
  }

  printf("视频监控系统已退出\n");

  // 返回主菜单（在所有资源清理完成后）
  back_menu();

  return 0;
}
