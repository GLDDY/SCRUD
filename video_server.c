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
  printf("\n接收到信号 %d, 正在关闭服务器...\n", sig);
  g_running = 0;

  // 关闭服务器socket以中断accept调用
  if (g_server_fd >= 0)
  {
    close(g_server_fd);
    g_server_fd = -1;
  }
}

/**
 * @brief 发送完整数据
 */
int send_full(int sock, const void *data, size_t size)
{
  size_t sent = 0;
  while (sent < size)
  {
    int n = send(sock, (char *)data + sent, size - sent, 0);
    if (n < 0)
    {
      perror("send failed");
      return -1;
    }
    sent += n;
  }
  return 0;
}

/**
 * @brief 客户端处理线程
 */
void *client_handler(void *arg)
{
  int client_sock = *(int *)arg;
  free(arg);

  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  getpeername(client_sock, (struct sockaddr *)&addr, &addr_len);
  printf("[客户端 %s:%d] 已连接\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

  unsigned char *yuyv_data = NULL;
  unsigned int data_size = 0;

  while (g_running)
  {
    // 加锁获取摄像头帧
    pthread_mutex_lock(&camera_mutex);

    if (camera_get_frame(g_camera, &yuyv_data, &data_size) < 0)
    {
      pthread_mutex_unlock(&camera_mutex);
      usleep(10000); // 10ms
      continue;
    }

    // 准备数据包头
    frame_header_t header;
    header.magic = 0x12345678;
    header.frame_size = data_size;
    header.width = FRAME_WIDTH;
    header.height = FRAME_HEIGHT;
    header.format = 0; // YUYV
    header.timestamp = (unsigned int)time(NULL);

    // 发送数据包头
    if (send_full(client_sock, &header, sizeof(header)) < 0)
    {
      camera_release_frame(g_camera);
      pthread_mutex_unlock(&camera_mutex);
      break;
    }

    // 发送图像数据
    if (send_full(client_sock, yuyv_data, data_size) < 0)
    {
      camera_release_frame(g_camera);
      pthread_mutex_unlock(&camera_mutex);
      break;
    }

    camera_release_frame(g_camera);
    pthread_mutex_unlock(&camera_mutex);

    // usleep(33000);
  }

  printf("[客户端 %s:%d] 已断开\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
  close(client_sock);
  return NULL;
}

/**
 * @brief 本地显示线程
 */
void *display_thread(void *arg)
{
  printf("本地显示线程启动\n");

  while (g_running)
  {
    pthread_mutex_lock(&camera_mutex); // 对互斥锁进行加锁操作， 保护视像头共享资源

    // 在LCD上显示摄像头画面
    if (camera_display(g_camera, 0, 0) < 0)
    {
      pthread_mutex_unlock(&camera_mutex);
      // usleep(10000);
      continue;
    }

    pthread_mutex_unlock(&camera_mutex);

    // usleep(50000);
  }

  return NULL;
}

int server_run(int argc, char *argv[])
{

  // 初始化摄像头
  printf("[2/4] 初始化摄像头...\n");
  g_camera = camera_init("/dev/video7", FRAME_WIDTH, FRAME_HEIGHT);
  if (!g_camera)
  {
    fprintf(stderr, "摄像头初始化失败\n");
    close_lcd();
    return -1;
  }

  if (camera_start(g_camera) < 0)
  {
    fprintf(stderr, "摄像头启动失败\n");
    camera_close(g_camera);
    close_lcd();
    return -1;
  }

  // 创建TCP服务器
  printf("[3/4] 创建TCP服务器 (端口:%d)...\n", PORT);
  g_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_server_fd < 0)
  {
    perror("socket创建失败");
    camera_stop(g_camera);
    camera_close(g_camera);
    close_lcd();
    return -1;
  }

  // 设置端口复用
  int opt = 1;
  setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // 绑定地址
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(g_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("bind失败");
    close(g_server_fd);
    camera_stop(g_camera);
    camera_close(g_camera);
    close_lcd();
    return -1;
  }

  // 监听
  if (listen(g_server_fd, MAX_CLIENTS) < 0)
  {
    perror("listen失败");
    close(g_server_fd);
    camera_stop(g_camera);
    camera_close(g_camera);
    close_lcd();
    return -1;
  }

  printf("服务器正在监听端口 %d...\n", PORT);

  // 启动本地显示线程
  printf("[4/4] 启动本地显示线程...\n\n");
  pthread_t display_tid;
  if (pthread_create(&display_tid, NULL, display_thread, NULL) != 0)
  {
    perror("创建显示线程失败");
    close(g_server_fd);
    camera_stop(g_camera);
    camera_close(g_camera);
    close_lcd();
    return -1;
  }

  printf("========================================\n");
  printf("系统运行中...\n");
  printf("按 Ctrl+C 退出\n");
  printf("========================================\n\n");

  // 主循环: 接受客户端连接
  while (g_running)
  {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int *client_sock = malloc(sizeof(int));
    *client_sock = accept(g_server_fd, (struct sockaddr *)&client_addr, &client_len);

    if (*client_sock < 0)
    {
      if (g_running)
      {
        perror("accept失败");
      }
      free(client_sock);
      continue;
    }

    // 为每个客户端创建处理线程
    pthread_t tid;
    if (pthread_create(&tid, NULL, client_handler, client_sock) != 0)
    {
      perror("创建客户端处理线程失败");
      close(*client_sock);
      free(client_sock);
      continue;
    }
    pthread_detach(tid); // 分离线程,自动回收资源
  }

  if (!g_running)
  {
    printf("\n正在清理资源...\n");
    // 清理资源
    pthread_cancel(display_tid);
    pthread_join(display_tid, NULL);

    if (g_server_fd >= 0)
    {
      close(g_server_fd);
    }
    camera_stop(g_camera);
    camera_close(g_camera);
    close_lcd();

    pthread_mutex_destroy(&camera_mutex);

    printf("服务器已关闭\n");
    return 0;
  }

  // 异常退出情况
  printf("服务器异常退出\n");
  return -1;
}
