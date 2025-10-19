#include "camera_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief 初始化摄像头模块
 */
camera_module_t *camera_module_init(const char *dev_name, int width, int height)
{
  camera_module_t *cam_module = (camera_module_t *)malloc(sizeof(camera_module_t));
  if (!cam_module)
  {
    perror("malloc camera_module_t failed");
    return NULL;
  }

  memset(cam_module, 0, sizeof(camera_module_t));

  // 初始化互斥锁
  if (pthread_mutex_init(&cam_module->mutex, NULL) != 0)
  {
    perror("pthread_mutex_init failed");
    free(cam_module);
    return NULL;
  }

  // 初始化摄像头
  cam_module->camera = camera_init(dev_name, width, height);
  if (!cam_module->camera)
  {
    fprintf(stderr, "摄像头初始化失败\n");
    pthread_mutex_destroy(&cam_module->mutex);
    free(cam_module);
    return NULL;
  }

  cam_module->is_running = 0;
  cam_module->last_frame = NULL;
  cam_module->last_frame_size = 0;
  cam_module->capture_request = 0;

  printf("摄像头模块初始化成功\n");
  return cam_module;
}

/**
 * @brief 启动摄像头模块
 */
int camera_module_start(camera_module_t *cam_module)
{
  if (!cam_module || !cam_module->camera)
  {
    return -1;
  }

  if (camera_start(cam_module->camera) < 0)
  {
    fprintf(stderr, "摄像头启动失败\n");
    return -1;
  }

  cam_module->is_running = 1;
  printf("摄像头模块启动成功\n");
  return 0;
}

/**
 * @brief 停止摄像头模块
 */
int camera_module_stop(camera_module_t *cam_module)
{
  if (!cam_module || !cam_module->camera)
  {
    return -1;
  }

  cam_module->is_running = 0;

  if (camera_stop(cam_module->camera) < 0)
  {
    fprintf(stderr, "摄像头停止失败\n");
    return -1;
  }

  printf("摄像头模块停止成功\n");
  return 0;
}

/**
 * @brief 关闭摄像头模块并释放资源
 */
void camera_module_close(camera_module_t *cam_module)
{
  if (!cam_module)
  {
    return;
  }

  if (cam_module->camera)
  {
    camera_close(cam_module->camera);
  }

  if (cam_module->last_frame)
  {
    free(cam_module->last_frame);
  }

  pthread_mutex_destroy(&cam_module->mutex);
  free(cam_module);

  printf("摄像头模块已关闭\n");
}

/**
 * @brief 获取一帧图像数据（带锁保护）
 */
int camera_module_get_frame(camera_module_t *cam_module, unsigned char **yuyv_data, unsigned int *data_size)
{
  if (!cam_module || !cam_module->camera)
  {
    return -1;
  }

  pthread_mutex_lock(&cam_module->mutex);
  int ret = camera_get_frame(cam_module->camera, yuyv_data, data_size);
  pthread_mutex_unlock(&cam_module->mutex);

  return ret;
}

/**
 * @brief 释放一帧图像（带锁保护）
 */
int camera_module_release_frame(camera_module_t *cam_module)
{
  if (!cam_module || !cam_module->camera)
  {
    return -1;
  }

  pthread_mutex_lock(&cam_module->mutex);
  int ret = camera_release_frame(cam_module->camera);
  pthread_mutex_unlock(&cam_module->mutex);

  return ret;
}

/**
 * @brief 在LCD上显示摄像头图像（带锁保护）
 */
int camera_module_display(camera_module_t *cam_module, int x0, int y0)
{
  if (!cam_module || !cam_module->camera)
  {
    return -1;
  }

  pthread_mutex_lock(&cam_module->mutex);
  int ret = camera_display(cam_module->camera, x0, y0);
  pthread_mutex_unlock(&cam_module->mutex);

  return ret;
}

/**
 * @brief 请求截屏并保存当前帧
 */
int camera_module_capture_frame(camera_module_t *cam_module, unsigned char **yuyv_data, unsigned int *data_size)
{
  if (!cam_module || !cam_module->camera)
  {
    return -1;
  }

  pthread_mutex_lock(&cam_module->mutex);

  // 获取当前帧
  unsigned char *frame_data = NULL;
  unsigned int frame_size = 0;

  if (camera_get_frame(cam_module->camera, &frame_data, &frame_size) < 0)
  {
    pthread_mutex_unlock(&cam_module->mutex);
    return -1;
  }

  // 释放旧的截屏缓冲区
  if (cam_module->last_frame)
  {
    free(cam_module->last_frame);
  }

  // 分配新的缓冲区并复制数据
  cam_module->last_frame = (unsigned char *)malloc(frame_size);
  if (!cam_module->last_frame)
  {
    perror("malloc capture frame failed");
    camera_release_frame(cam_module->camera);
    pthread_mutex_unlock(&cam_module->mutex);
    return -1;
  }

  memcpy(cam_module->last_frame, frame_data, frame_size);
  cam_module->last_frame_size = frame_size;

  // 返回截屏数据
  *yuyv_data = cam_module->last_frame;
  *data_size = cam_module->last_frame_size;

  camera_release_frame(cam_module->camera);
  pthread_mutex_unlock(&cam_module->mutex);

  printf("截屏成功，帧大小: %u bytes\n", frame_size);
  return 0;
}
