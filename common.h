#ifndef __VIDEO_SERVER_H__
#define __VIDEO_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "camera.h"
#include "lcd.h"
#include "bmp.h"
#include "ts.h"
#include "camera_module.h"
#include "server_module.h"

// 全局变量声明
extern int g_running;
extern camera_t *g_camera;
extern int g_server_fd;
extern pthread_mutex_t camera_mutex;

// 函数声明
void signal_handler(int sig);

void back_menu(void);

#endif // __VIDEO_SERVER_H__