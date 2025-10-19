#include "server_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include "lcd.h"

// 全局客户端socket列表（用于截屏广播）
#define MAX_CLIENT_SOCKETS 10
static int g_client_sockets[MAX_CLIENT_SOCKETS];
static int g_client_count = 0;
static pthread_mutex_t g_client_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 发送完整数据
 */
static int send_full(int sock, const void *data, size_t size)
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
 * @brief 添加客户端socket
 */
static void add_client_socket(int sock)
{
  pthread_mutex_lock(&g_client_mutex);
  if (g_client_count < MAX_CLIENT_SOCKETS)
  {
    g_client_sockets[g_client_count++] = sock;
    printf("添加客户端socket: %d, 当前客户端数: %d\n", sock, g_client_count);
  }
  pthread_mutex_unlock(&g_client_mutex);
}

/**
 * @brief 移除客户端socket
 */
static void remove_client_socket(int sock)
{
  pthread_mutex_lock(&g_client_mutex);
  for (int i = 0; i < g_client_count; i++)
  {
    if (g_client_sockets[i] == sock)
    {
      // 将后面的元素前移
      for (int j = i; j < g_client_count - 1; j++)
      {
        g_client_sockets[j] = g_client_sockets[j + 1];
      }
      g_client_count--;
      printf("移除客户端socket: %d, 当前客户端数: %d\n", sock, g_client_count);
      break;
    }
  }
  pthread_mutex_unlock(&g_client_mutex);
}

/**
 * @brief 客户端处理线程（不再主动发送帧）
 */
void *client_handler_wrapper(void *arg)
{
  int client_sock = *(int *)arg;
  free(arg);

  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  getpeername(client_sock, (struct sockaddr *)&addr, &addr_len);
  printf("[客户端 %s:%d] 已连接\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

  // 添加到客户端列表
  add_client_socket(client_sock);

  // 保持连接，等待截屏指令
  char buffer[256];
  while (1)
  {
    // 检测客户端是否断开（通过接收数据判断）
    int n = recv(client_sock, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);
    if (n == 0)
    {
      // 客户端断开连接
      break;
    }
    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
      break;
    }
    sleep(1);
  }

  printf("[客户端 %s:%d] 已断开\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

  // 从客户端列表中移除
  remove_client_socket(client_sock);
  close(client_sock);
  return NULL;
}

/**
 * @brief 本地显示线程
 */
static void *display_thread_func(void *arg)
{
  server_module_t *server = (server_module_t *)arg;
  printf("本地显示线程启动\n");

  while (server->is_running)
  {
    // 在LCD上显示摄像头画面
    if (camera_module_display(server->camera_module, 0, 0) < 0)
    {
      usleep(10000);
      continue;
    }

    usleep(50000); // 50ms, 约20fps
  }

  printf("本地显示线程退出\n");
  return NULL;
}

/**
 * @brief 初始化服务器模块
 */
server_module_t *server_module_init(camera_module_t *camera_module)
{
  if (!camera_module)
  {
    fprintf(stderr, "摄像头模块为空\n");
    return NULL;
  }

  server_module_t *server = (server_module_t *)malloc(sizeof(server_module_t));
  if (!server)
  {
    perror("malloc server_module_t failed");
    return NULL;
  }

  memset(server, 0, sizeof(server_module_t));
  server->server_fd = -1;
  server->is_running = 0;
  server->camera_module = camera_module;

  // 初始化客户端列表
  g_client_count = 0;
  memset(g_client_sockets, 0, sizeof(g_client_sockets));

  printf("服务器模块初始化成功\n");
  return server;
}

/**
 * @brief 启动服务器
 */
int server_module_start(server_module_t *server)
{
  if (!server)
  {
    return -1;
  }

  // 创建TCP服务器
  printf("创建TCP服务器 (端口:%d)...\n", PORT);
  server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->server_fd < 0)
  {
    perror("socket创建失败");
    return -1;
  }

  // 设置端口复用
  int opt = 1;
  setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // 绑定地址
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(server->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("bind失败");
    close(server->server_fd);
    return -1;
  }

  // 监听
  if (listen(server->server_fd, MAX_CLIENTS) < 0)
  {
    perror("listen失败");
    close(server->server_fd);
    return -1;
  }

  printf("服务器正在监听端口 %d...\n", PORT);

  // 启动本地显示线程
  printf("启动本地显示线程...\n");
  server->is_running = 1;
  if (pthread_create(&server->display_thread, NULL, display_thread_func, server) != 0)
  {
    perror("创建显示线程失败");
    close(server->server_fd);
    return -1;
  }

  printf("服务器启动成功\n");
  return 0;
}

/**
 * @brief 停止服务器
 */
int server_module_stop(server_module_t *server)
{
  if (!server)
  {
    return -1;
  }

  // 检查是否已经停止，避免重复操作
  if (server->is_running == 0 && server->server_fd < 0)
  {
    printf("服务器已经停止\n");
    return 0;
  }

  printf("\n正在停止服务器...\n");
  server->is_running = 0;

  // 关闭服务器socket（只关闭一次）
  if (server->server_fd >= 0)
  {
    shutdown(server->server_fd, SHUT_RDWR); // 先shutdown中断阻塞
    close(server->server_fd);
    server->server_fd = -1;
  }

  // 等待显示线程退出（不使用cancel，让它自然退出）
  if (server->display_thread)
  {
    // 给线程一些时间自然退出
    usleep(200000); // 200ms

    // 如果还没退出，再等待
    void *retval;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1; // 1秒超时

    int ret = pthread_join(server->display_thread, &retval);
    if (ret != 0)
    {
      printf("警告：显示线程退出异常，强制取消\n");
      pthread_cancel(server->display_thread);
      pthread_join(server->display_thread, NULL);
    }
    server->display_thread = 0;
  }

  // 关闭所有客户端连接
  pthread_mutex_lock(&g_client_mutex);
  for (int i = 0; i < g_client_count; i++)
  {
    if (g_client_sockets[i] >= 0)
    {
      close(g_client_sockets[i]);
      g_client_sockets[i] = -1;
    }
  }
  g_client_count = 0;
  pthread_mutex_unlock(&g_client_mutex);

  printf("服务器已停止\n");
  return 0;
}

/**
 * @brief 关闭服务器并释放资源
 */
void server_module_close(server_module_t *server)
{
  if (!server)
  {
    return;
  }

  server_module_stop(server);
  free(server);
  printf("服务器模块已关闭\n");
}

/**
 * @brief 发送截屏帧给所有客户端
 */
int server_module_send_capture(server_module_t *server)
{
  if (!server || !server->camera_module)
  {
    return -1;
  }

  unsigned char *yuyv_data = NULL;
  unsigned int data_size = 0;

  // 获取截屏帧
  if (camera_module_capture_frame(server->camera_module, &yuyv_data, &data_size) < 0)
  {
    fprintf(stderr, "获取截屏帧失败\n");
    return -1;
  }

  // 准备数据包头
  frame_header_t header;
  header.magic = 0x12345678;
  header.frame_size = data_size;
  header.width = FRAME_WIDTH;
  header.height = FRAME_HEIGHT;
  header.format = 0; // YUYV
  header.timestamp = (unsigned int)time(NULL);

  printf("发送截屏帧给所有客户端，大小: %u bytes\n", data_size);

  // 广播给所有客户端
  pthread_mutex_lock(&g_client_mutex);
  for (int i = 0; i < g_client_count; i++)
  {
    int sock = g_client_sockets[i];

    // 发送数据包头
    if (send_full(sock, &header, sizeof(header)) < 0)
    {
      fprintf(stderr, "发送包头失败，客户端: %d\n", sock);
      continue;
    }

    // 发送图像数据
    if (send_full(sock, yuyv_data, data_size) < 0)
    {
      fprintf(stderr, "发送图像数据失败，客户端: %d\n", sock);
      continue;
    }

    printf("成功发送截屏到客户端: %d\n", sock);
  }
  pthread_mutex_unlock(&g_client_mutex);

  printf("截屏发送完成\n");
  return 0;
}

/**
 * @brief 服务器主循环（接受客户端连接）
 */
int server_module_run(server_module_t *server)
{
  if (!server || server->server_fd < 0)
  {
    return -1;
  }

  printf("========================================\n");
  printf("系统运行中...\n");
  printf("等待客户端连接...\n");
  printf("========================================\n\n");

  // 主循环: 接受客户端连接
  while (server->is_running)
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
      if (server->is_running)
      {
        perror("accept失败");
      }
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

  return 0;
}
