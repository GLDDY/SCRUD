#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>//头文件。
#include <unistd.h>
#include <sys/mman.h>

int fd = -1;
int *plcd = NULL;

//打开屏幕并且映射
void open_lcd()
{
    //1.打开LCD屏幕文件
    fd = open("/dev/fb0",O_RDWR);
    if(fd == -1)
    {
        perror("open LCD file failed\n");
        return ;
    }

    plcd = mmap(NULL,800*480*4,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(plcd == MAP_FAILED)
    {
        perror("mmap failed\n");
        return ;
    }
}


//解除映射并且关闭屏幕文件
void close_lcd()
{
    munmap(plcd,800*480*4);

    close(fd);
}

void display_point(int x,int y,int color)
{
    if(0<= x && x<800 && 0<= y && y<480)
    {
        *(plcd + 800 * y + x) = color;
    }
}