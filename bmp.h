#ifndef __BMP_H__
#define __BMP_H__

/*
    bmp_display:这个函数的作用是在开发板对应的位置开始显示一张Bmp图片。
    @bmp_file:要显示的bmp图片的文件名
    @x0,y0:显示起始位置的坐标

    返回值：无
*/
void bmp_display(const char *bmp_file,int x0 ,int y0);


#endif