#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <linux/videodev2.h>
#include <linux/fb.h>
#include <linux/input.h>

// 摄像头设备结构体
typedef struct
{
  int fd;                 // 设备文件描述符
  struct v4l2_buffer buf; // 缓冲区信息
  void *mptr[4];          // 映射内存指针数组
  unsigned int size[4];   // 每个缓冲区大小
  int width;              // 图像宽度
  int height;             // 图像高度
} camera_t;

/**
 * @brief 初始化摄像头
 * @param dev_name 摄像头设备路径，如 "/dev/video0"
 * @param width 图像宽度
 * @param height 图像高度
 * @return 成功返回摄像头结构体指针，失败返回NULL
 */
camera_t *camera_init(const char *dev_name, int width, int height);

/**
 * @brief 开始视频采集
 * @param cam 摄像头结构体指针
 * @return 成功返回0，失败返回-1
 */
int camera_start(camera_t *cam);

/**
 * @brief 获取一帧图像数据
 * @param cam 摄像头结构体指针
 * @param yuyv_data 输出YUYV格式图像数据指针
 * @param data_size 输出数据大小
 * @return 成功返回0，失败返回-1
 */
int camera_get_frame(camera_t *cam, unsigned char **yuyv_data, unsigned int *data_size);

/**
 * @brief 释放一帧图像
 * @param cam 摄像头结构体指针
 * @return 成功返回0，失败返回-1
 */
int camera_release_frame(camera_t *cam);

/**
 * @brief 停止视频采集
 * @param cam 摄像头结构体指针
 * @return 成功返回0，失败返回-1
 */
int camera_stop(camera_t *cam);

/**
 * @brief 关闭摄像头并释放资源
 * @param cam 摄像头结构体指针
 */
void camera_close(camera_t *cam);

/**
 * @brief YUYV转RGB888
 * @param yuyv YUYV格式数据
 * @param rgb RGB888格式输出缓冲区
 * @param width 图像宽度
 * @param height 图像高度
 */
void yuyv_to_rgb888(const unsigned char *yuyv, unsigned char *rgb, int width, int height);

/**
 * @brief 在LCD上显示摄像头图像
 * @param cam 摄像头结构体指针
 * @param x0 显示起始x坐标
 * @param y0 显示起始y坐标
 * @return 成功返回0，失败返回-1
 */
int camera_display(camera_t *cam, int x0, int y0);

#endif // __CAMERA_H__
