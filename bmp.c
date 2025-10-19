
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "lcd.h"

/*
    bmp_display: 在屏幕的坐标(x0,y0)处显示一张指定的bmp图片
    @bmp_file: 要显示的bmp图片的文件名
    @x0: 显示位置左上顶点的x轴坐标
    @y0: 显示位置左上顶点的y轴坐标
    返回值：
        无。
*/
void bmp_display(const char * bmp_file, 
                int x0, int y0)
{
    int fd;


    fd = open(bmp_file, O_RDONLY);
    if (fd == -1)
    {
        perror("failed to open bmp_file");
        return ;
    }

    int width, height;
    short depth;
    

    lseek(fd, 0x12, SEEK_SET);
    read(fd, &width, 4);
    


    lseek(fd, 0x16, SEEK_SET);
    read(fd, &height, 4);
    

    lseek(fd, 0x1C, SEEK_SET);
    read(fd, &depth, 2);
   

    printf("%d x %d\n", width, height);

    if ( !(depth == 24 || depth == 32))
    {
        printf("Sorry, Not Supported Bmp Format!\n");
        close(fd);

        return ;
    }


    int valid_bytes_per_line; //每一行有效的数据字节数
    int laizi = 0; // 每一行末尾的填充的“赖子”数
    int total_bytes_per_line; //每一行实际的字节数.
    int total_bytes; //整个像素数组的字节数


    valid_bytes_per_line =  abs(width) * (depth / 8);
    if (valid_bytes_per_line % 4)
    {
        laizi = 4 - valid_bytes_per_line % 4;
    }
    total_bytes_per_line = valid_bytes_per_line + laizi;
    total_bytes = abs(height) * total_bytes_per_line;

    unsigned char * pixel = (unsigned char*) malloc( total_bytes );
    lseek(fd, 54, SEEK_SET);
    read(fd, pixel, total_bytes);

    // 解析像素数据，并在屏幕上显示
    unsigned char a,r, g,b;
    int color;
    int i = 0;
    int x,y;

    for (y = 0; y < abs(height); y++)
    {
        for (x = 0; x < abs(width); x++)
        {
            b = pixel[i++];
            g = pixel[i++];
            r = pixel[i++];
            if (depth == 32)
            {
                a = pixel[i++];
            }
            else
            {
                a = 0;
            }
            color = (a << 24) | (r << 16) | (g << 8) | b;

            int x1, y1; //该像素点在屏幕上显示的  坐标

            x1 = (width > 0) ? (x0 + x) : (x0 + abs(width) - 1 - x);
            y1 = (height > 0) ?  (y0 + height - 1 - y) : y0 + y;
            display_point(x1, y1, color); 
        }

        i += laizi; //跳过“赖子”
    }

    free(pixel);

}


