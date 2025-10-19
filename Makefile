# 智能家庭视频监控系统 Makefile
# 编译器设置
CC = arm-linux-gcc
CC_X86 = gcc

# 编译选项
CFLAGS = -Wall -O2 -lpthread
LIBS = -lpthread

# 目标文件
SERVER = video_server
CLIENT = video_client

# 源文件
SERVER_SRCS = main.c module.c camera.c lcd.c bmp.c ts.c camera_module.c server_module.c utils.c
CLIENT_SRCS = video_client.c

# 目标文件
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# 默认目标
.PHONY: all clean server client help

all: help

help:
	@echo "=========================================="
	@echo "  智能家庭视频监控系统编译选项"
	@echo "=========================================="
	@echo "make server    - 编译服务器端 (ARM开发板)"
	@echo "make client    - 编译客户端 (PC端)"
	@echo "make both      - 同时编译服务器和客户端"
	@echo "make clean     - 清理编译文件"
	@echo "=========================================="

# 编译服务器端 (ARM)
server: $(SERVER)
	@echo "=========================================="
	@echo "服务器端编译完成: $(SERVER)"
	@echo "=========================================="

$(SERVER): $(SERVER_OBJS)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译客户端 (x86)
client: 
	@echo "=========================================="
	@echo "正在编译客户端 (x86)..."
	@echo "=========================================="
	$(CC_X86) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRCS) $(LIBS)
	@echo "=========================================="
	@echo "客户端编译完成: $(CLIENT)"
	@echo "=========================================="

# 同时编译
both: server client

# 清理
clean:
	rm -f $(SERVER) $(CLIENT) *.o *.ppm
	@echo "清理完成"

# 部署到开发板
.PHONY: deploy
deploy: server
	@echo "=========================================="
	@echo "准备部署到开发板..."
	@echo "请确保已连接到开发板"
	@echo "=========================================="
	@if [ -z "$(BOARD_IP)" ]; then \
		echo "错误: 请设置BOARD_IP环境变量"; \
		echo "示例: make deploy BOARD_IP=192.168.1.100"; \
		exit 1; \
	fi
	scp $(SERVER) root@$(BOARD_IP):/root/
	@echo "=========================================="
	@echo "部署完成!"
	@echo "=========================================="

# 运行帮助
.PHONY: run-help
run-help:
	@echo "=========================================="
	@echo "  运行说明"
	@echo "=========================================="
	@echo "1. 在开发板上运行服务器:"
	@echo "   ./$(SERVER)"
	@echo ""
	@echo "2. 在PC上运行客户端:"
	@echo "   ./$(CLIENT) <服务器IP> <端口>"
	@echo "   示例: ./$(CLIENT) 192.168.1.100 8888"
	@echo "=========================================="
