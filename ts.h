#ifndef __TS_H__
#define __TS_H__

typedef struct point
{
  int x;
  int y;
} ts_point;

// “滑动方向”
typedef enum
{
  MOVE_UP = 1,    // 向上滑动
  MOVE_DOWN = 2,  // 向下滑动
  MOVE_LEFT = 3,  // 向左滑动
  MOVE_RIGHT = 4, // 向右滑动

  MOVE_UNKNOWN = 100
} move_dir_t;

// 获取手指在触摸屏上的滑动方向
move_dir_t get_ts_direction(void);

// 获取触摸屏点击事件
void get_ts_point(ts_point *p);

#endif