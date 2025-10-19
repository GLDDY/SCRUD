#include "camera.h"
#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

// 颜色转换辅助函数
static inline int clip(int value)
{
  return value < 0 ? 0 : (value > 255 ? 255 : value);
}

/**
 * @brief 初始化摄像头
 */
camera_t *camera_init(const char *dev_name, int width, int height)
{
  camera_t *cam = (camera_t *)malloc(sizeof(camera_t));
  if (!cam)
  {
    perror("malloc camera_t failed");
    return NULL;
  }

  memset(cam, 0, sizeof(camera_t));
  cam->width = width;
  cam->height = height;

  // 1. 打开摄像头设备
  cam->fd = open(dev_name, O_RDWR);
  if (cam->fd < 0)
  {
    perror("open camera device failed");
    free(cam);
    return NULL;
  }

  // 2. 查询设备能力
  struct v4l2_capability cap;
  if (ioctl(cam->fd, VIDIOC_QUERYCAP, &cap) < 0)
  {
    perror("VIDIOC_QUERYCAP failed");
    close(cam->fd);
    free(cam);
    return NULL;
  }

  printf("Camera: %s\n", cap.card);

  // 3. 设置视频格式
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = width;
  fmt.fmt.pix.height = height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // YUYV格式
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  if (ioctl(cam->fd, VIDIOC_S_FMT, &fmt) < 0)
  {
    perror("VIDIOC_S_FMT failed");
    close(cam->fd);
    free(cam);
    return NULL;
  }

  // 4. 请求缓冲区
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count = 4; // 请求4个缓冲区
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (ioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0)
  {
    perror("VIDIOC_REQBUFS failed");
    close(cam->fd);
    free(cam);
    return NULL;
  }

  // 5. 映射缓冲区并加入队列
  for (int i = 0; i < 4; i++)
  {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (ioctl(cam->fd, VIDIOC_QUERYBUF, &buf) < 0)
    {
      perror("VIDIOC_QUERYBUF failed");
      camera_close(cam);
      return NULL;
    }

    cam->size[i] = buf.length;
    cam->mptr[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                        MAP_SHARED, cam->fd, buf.m.offset);

    if (cam->mptr[i] == MAP_FAILED)
    {
      perror("mmap failed");
      camera_close(cam);
      return NULL;
    }

    // 将缓冲区放入队列
    if (ioctl(cam->fd, VIDIOC_QBUF, &buf) < 0)
    {
      perror("VIDIOC_QBUF failed");
      camera_close(cam);
      return NULL;
    }
  }

  printf("Camera initialized: %dx%d\n", width, height);
  return cam;
}

/**
 * @brief 开始视频采集
 */
int camera_start(camera_t *cam)
{
  if (!cam)
    return -1;

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(cam->fd, VIDIOC_STREAMON, &type) < 0)
  {
    perror("VIDIOC_STREAMON failed");
    return -1;
  }

  printf("Camera started\n");
  return 0;
}

/**
 * @brief 获取一帧图像数据
 */
int camera_get_frame(camera_t *cam, unsigned char **yuyv_data, unsigned int *data_size)
{
  if (!cam || !yuyv_data || !data_size)
    return -1;

  memset(&cam->buf, 0, sizeof(cam->buf));
  cam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  cam->buf.memory = V4L2_MEMORY_MMAP;

  // 从队列中取出已填充的缓冲区
  if (ioctl(cam->fd, VIDIOC_DQBUF, &cam->buf) < 0)
  {
    perror("VIDIOC_DQBUF failed");
    return -1;
  }

  *yuyv_data = (unsigned char *)cam->mptr[cam->buf.index];
  *data_size = cam->buf.bytesused;

  return 0;
}

/**
 * @brief 释放一帧图像
 */
int camera_release_frame(camera_t *cam)
{
  if (!cam)
    return -1;

  // 将缓冲区重新放入队列
  if (ioctl(cam->fd, VIDIOC_QBUF, &cam->buf) < 0)
  {
    perror("VIDIOC_QBUF failed");
    return -1;
  }

  return 0;
}

/**
 * @brief 停止视频采集
 */
int camera_stop(camera_t *cam)
{
  if (!cam)
    return -1;

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(cam->fd, VIDIOC_STREAMOFF, &type) < 0)
  {
    perror("VIDIOC_STREAMOFF failed");
    return -1;
  }

  printf("Camera stopped\n");
  return 0;
}

/**
 * @brief 关闭摄像头并释放资源
 */
void camera_close(camera_t *cam)
{
  if (!cam)
    return;

  // 取消内存映射
  for (int i = 0; i < 4; i++)
  {
    if (cam->mptr[i] && cam->mptr[i] != MAP_FAILED)
    {
      munmap(cam->mptr[i], cam->size[i]);
    }
  }

  if (cam->fd >= 0)
  {
    close(cam->fd);
  }

  free(cam);
  printf("Camera closed\n");
}

/**
 * @brief YUYV转RGB888
 * YUYV格式: Y0 U0 Y1 V0 (2个像素4个字节)
 */
void yuyv_to_rgb888(const unsigned char *yuyv, unsigned char *rgb, int width, int height)
{
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

    rgb[i * 6] = clip((298 * c + 409 * e + 128) >> 8);               // R
    rgb[i * 6 + 1] = clip((298 * c - 100 * d - 208 * e + 128) >> 8); // G
    rgb[i * 6 + 2] = clip((298 * c + 516 * d + 128) >> 8);           // B

    // 转换第二个像素
    c = y1 - 16;
    rgb[i * 6 + 3] = clip((298 * c + 409 * e + 128) >> 8);           // R
    rgb[i * 6 + 4] = clip((298 * c - 100 * d - 208 * e + 128) >> 8); // G
    rgb[i * 6 + 5] = clip((298 * c + 516 * d + 128) >> 8);           // B
  }
}

/**
 * @brief 在LCD上显示摄像头图像
 */
int camera_display(camera_t *cam, int x0, int y0)
{
  if (!cam)
    return -1;

  unsigned char *yuyv_data = NULL;
  unsigned int data_size = 0;

  // 获取一帧图像
  if (camera_get_frame(cam, &yuyv_data, &data_size) < 0)
  {
    return -1;
  }

  // 分配RGB缓冲区
  unsigned char *rgb_data = (unsigned char *)malloc(cam->width * cam->height * 3);
  if (!rgb_data)
  {
    perror("malloc rgb_data failed");
    camera_release_frame(cam);
    return -1;
  }

  // 转换YUYV到RGB
  yuyv_to_rgb888(yuyv_data, rgb_data, cam->width, cam->height);

  // 显示到LCD
  for (int y = 0; y < cam->height; y++)
  {
    for (int x = 0; x < cam->width; x++)
    {
      int idx = (y * cam->width + x) * 3;
      int r = rgb_data[idx];
      int g = rgb_data[idx + 1];
      int b = rgb_data[idx + 2];

      // RGB888转RGB565或ARGB8888(根据LCD格式)
      int color = (r << 16) | (g << 8) | b | 0xFF000000;
      display_point(x0 + x, y0 + y, color);
    }
  }

  free(rgb_data);
  camera_release_frame(cam);

  return 0;
}
