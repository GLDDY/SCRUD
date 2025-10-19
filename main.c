#include "common.h"
#include "module.h"
#include "utils.h"

// 外部全局变量声明
extern int g_running;

int main(int argc, char *argv[])
{
  // 初始化全局变量
  g_running = 1;

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  printf("========================================\n");
  printf("   智能家庭视频监控系统 - 服务器端\n");
  printf("========================================\n\n");

  printf("[1/4] 初始化LCD显示...\n");
  open_lcd();

  printf("显示开始界面...\n");
  bmp_display("./main.bmp", 0, 0); // 显示开始界面背景

  printf("点击屏幕'进入'按钮启动系统...\n");
  ts_point pt;

  while (g_running)
  {
    printf("等待触摸屏幕启动系统...\n");
    get_ts_point(&pt);

    printf("检测到触摸: x=%d, y=%d\n", pt.x, pt.y);

    if (pt.x >= 280 && pt.x <= 520 && pt.y >= 270 && pt.y <= 460)
    {
      printf("点击了'进入'按钮，启动视频监控系统...\n");

      video_monitor(argc, argv);
    }
    else
    {
      printf("请点击'进入'按钮区域启动系统\n");
    }
  }
  // 正常退出
  printf("主程序退出\n");

  close_lcd();
  return 0;
}