#ifndef __CAMERA_MODULE_H__
#define __CAMERA_MODULE_H__

#include <pthread.h>
#include "camera.h"

// 摄像头模块结构
typedef struct
{
  camera_t *camera;
  pthread_mutex_t mutex;
  int is_running;
  unsigned char *last_frame;
  unsigned int last_frame_size;
  int capture_request; // 截屏请求标志
} camera_module_t;

/**
 * @brief 初始化摄像头模块
 * @param dev_name 摄像头设备路径
 * @param width 图像宽度
 * @param height 图像高度
 * @return 成功返回摄像头模块指针，失败返回NULL
 */
camera_module_t *camera_module_init(const char *dev_name, int width, int height);

/**
 * @brief 启动摄像头模块
 * @param cam_module 摄像头模块指针
 * @return 成功返回0，失败返回-1
 */
int camera_module_start(camera_module_t *cam_module);

/**
 * @brief 停止摄像头模块
 * @param cam_module 摄像头模块指针
 * @return 成功返回0，失败返回-1
 */
int camera_module_stop(camera_module_t *cam_module);

/**
 * @brief 关闭摄像头模块并释放资源
 * @param cam_module 摄像头模块指针
 */
void camera_module_close(camera_module_t *cam_module);

/**
 * @brief 获取一帧图像数据（带锁保护）
 * @param cam_module 摄像头模块指针
 * @param yuyv_data 输出YUYV格式图像数据指针
 * @param data_size 输出数据大小
 * @return 成功返回0，失败返回-1
 */
int camera_module_get_frame(camera_module_t *cam_module, unsigned char **yuyv_data, unsigned int *data_size);

/**
 * @brief 释放一帧图像（带锁保护）
 * @param cam_module 摄像头模块指针
 * @return 成功返回0，失败返回-1
 */
int camera_module_release_frame(camera_module_t *cam_module);

/**
 * @brief 在LCD上显示摄像头图像（带锁保护）
 * @param cam_module 摄像头模块指针
 * @param x0 显示起始x坐标
 * @param y0 显示起始y坐标
 * @return 成功返回0，失败返回-1
 */
int camera_module_display(camera_module_t *cam_module, int x0, int y0);

/**
 * @brief 请求截屏
 * @param cam_module 摄像头模块指针
 * @return 成功返回0，失败返回-1
 */
int camera_module_capture_frame(camera_module_t *cam_module, unsigned char **yuyv_data, unsigned int *data_size);

#endif // __CAMERA_MODULE_H__
