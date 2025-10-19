#ifndef __LCD_H__
#define __LCD_H__

void open_lcd();

//解除映射并且关闭屏幕文件
void close_lcd();

void display_point(int x,int y,int color);

#endif