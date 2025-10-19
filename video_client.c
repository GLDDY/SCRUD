#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

typedef struct
{
  unsigned int magic;      // 魔数 0x12345678
  unsigned int frame_size; // 帧数据大小
  unsigned int width;      // 图像宽度
  unsigned int height;     // 图像高度
  unsigned int format;     // 图像格式 (0=YUYV)
  unsigned int timestamp;  // 时间戳
} frame_header_t;

static int g_running = 1;

/**
 * @brief 信号处理函数
 */
void signal_handler(int sig)
{
  printf("\n接收到信号 %d, 正在关闭客户端...\n", sig);
  g_running = 0;
}

/**
 * @brief 接收完整数据
 */
int recv_full(int sock, void *buffer, size_t size)
{
  size_t received = 0;
  while (received < size)
  {
    int n = recv(sock, (char *)buffer + received, size - received, 0);
    if (n <= 0)
    {
      if (n < 0)
      {
        perror("recv failed");
      }
      else
      {
        printf("连接已关闭\n");
      }
      return -1;
    }
    received += n;
  }
  return 0;
}

/**
 * @brief YUYV转RGB24并保存为PPM文件
 */
void save_frame_as_ppm(const unsigned char *yuyv, int width, int height, int frame_num)
{
  char filename[64];
  snprintf(filename, sizeof(filename), "frame_%04d.ppm", frame_num);

  FILE *fp = fopen(filename, "wb");
  if (!fp)
  {
    perror("打开文件失败");
    return;
  }

  // 写入PPM文件头
  fprintf(fp, "P6\n%d %d\n255\n", width, height);

  // 转换YUYV到RGB24并写入
  for (int i = 0; i < width * height / 2; i++)
  {
    int y0 = yuyv[i * 4];
    int u = yuyv[i * 4 + 1];
    int y1 = yuyv[i * 4 + 2];
    int v = yuyv[i * 4 + 3];

    // 转换第一个像素
    int c = y0 - 16;
    int d = u - 128;
    int e = v - 128;

    unsigned char r = ((298 * c + 409 * e + 128) >> 8);
    unsigned char g = ((298 * c - 100 * d - 208 * e + 128) >> 8);
    unsigned char b = ((298 * c + 516 * d + 128) >> 8);

    fputc(r > 255 ? 255 : (r < 0 ? 0 : r), fp);
    fputc(g > 255 ? 255 : (g < 0 ? 0 : g), fp);
    fputc(b > 255 ? 255 : (b < 0 ? 0 : b), fp);

    // 转换第二个像素
    c = y1 - 16;
    r = ((298 * c + 409 * e + 128) >> 8);
    g = ((298 * c - 100 * d - 208 * e + 128) >> 8);
    b = ((298 * c + 516 * d + 128) >> 8);

    fputc(r > 255 ? 255 : (r < 0 ? 0 : r), fp);
    fputc(g > 255 ? 255 : (g < 0 ? 0 : g), fp);
    fputc(b > 255 ? 255 : (b < 0 ? 0 : b), fp);
  }

  fclose(fp);
  printf("保存帧 %d 到 %s\n", frame_num, filename);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "用法: %s <服务器IP> <端口>\n", argv[0]);
    fprintf(stderr, "示例: %s 192.168.1.100 8888\n", argv[0]);
    return 1;
  }

  const char *server_ip = argv[1];
  int server_port = atoi(argv[2]);

  // 注册信号处理
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  printf("========================================\n");
  printf("   智能家庭视频监控系统 - 客户端\n");
  printf("========================================\n\n");

  // 1. 创建套接字
  printf("正在连接服务器 %s:%d...\n", server_ip, server_port);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0)
  {
    perror("socket创建失败");
    return 1;
  }

  // 2. 连接服务器
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);

  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
  {
    fprintf(stderr, "无效的IP地址\n");
    close(sock_fd);
    return 1;
  }

  if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("连接服务器失败");
    close(sock_fd);
    return 1;
  }

  printf("成功连接到服务器!\n\n");
  printf("========================================\n");
  printf("正在等待接收截屏图像...\n");
  printf("按 Ctrl+C 退出\n");
  printf("提示: 在服务器端点击【截屏】按钮发送图片\n");
  printf("========================================\n\n");

  // 3. 接收截屏图像
  int frame_count = 0;
  time_t start_time = time(NULL);

  unsigned char *frame_buffer = malloc(FRAME_WIDTH * FRAME_HEIGHT * 2);
  if (!frame_buffer)
  {
    perror("malloc失败");
    close(sock_fd);
    return 1;
  }

  while (g_running)
  {
    frame_header_t header;

    // 接收数据包头
    printf("等待接收截屏...\n");
    if (recv_full(sock_fd, &header, sizeof(header)) < 0)
    {
      if (g_running)
      {
        fprintf(stderr, "接收数据包头失败\n");
      }
      break;
    }

    // 验证魔数
    if (header.magic != 0x12345678)
    {
      fprintf(stderr, "错误: 无效的数据包魔数 0x%08X (期望 0x12345678)\n", header.magic);
      break;
    }

    // 接收图像数据
    if (header.frame_size > FRAME_WIDTH * FRAME_HEIGHT * 2)
    {
      fprintf(stderr, "错误: 帧大小过大 (%u bytes, 最大 %d bytes)\n",
              header.frame_size, FRAME_WIDTH * FRAME_HEIGHT * 2);
      break;
    }

    printf("接收图像数据中... (大小: %u bytes)\n", header.frame_size);
    if (recv_full(sock_fd, frame_buffer, header.frame_size) < 0)
    {
      fprintf(stderr, "接收图像数据失败\n");
      break;
    }

    frame_count++;

    // 显示接收信息
    time_t current_time = time(NULL);
    struct tm *tm_info = localtime(&current_time);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("\n========================================\n");
    printf("接收到截屏 #%d\n", frame_count);
    printf("  时间: %s\n", time_str);
    printf("  分辨率: %dx%d\n", header.width, header.height);
    printf("  格式: %s\n", header.format == 0 ? "YUYV" : "Unknown");
    printf("  大小: %u bytes\n", header.frame_size);
    printf("========================================\n");

    // 保存图像
    save_frame_as_ppm(frame_buffer, header.width, header.height, frame_count);
  }

  printf("\n\n========================================\n");
  printf("客户端统计信息:\n");
  printf("  接收截屏数: %d\n", frame_count);
  printf("  运行时间: %.0f 秒\n", difftime(time(NULL), start_time));
  printf("========================================\n");

  // 清理资源
  free(frame_buffer);
  close(sock_fd);
  printf("客户端已关闭\n");

  return 0;
}
