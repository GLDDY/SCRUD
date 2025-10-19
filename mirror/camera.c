#include "common.h"
#include "camera.h"
//#include "jpeglib.h"
#include <stdint.h> // 用于uint8_t、uint32_t等类型定义
char formats[5][16] = {0};
struct v4l2_fmtdesc fmtdesc;
struct v4l2_format  fmt;
struct v4l2_capability cap;


int CAMERA_W, CAMERA_H;

int R[256][256];
int G[256][256][256];
int B[256][256];

// 获取摄像头格式信息（固定）
bool get_caminfo(int camfd)
{
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    bool ret = false;
    printf("当前摄像头所支持的格式有: \n");
    while((ret=ioctl(camfd, VIDIOC_ENUM_FMT, &fmtdesc)) == 0)
    {
        printf("[%d]", fmtdesc.index);
        sprintf(formats[fmtdesc.index]+0, "%c", (fmtdesc.pixelformat>>8*0)&0xFF);
        sprintf(formats[fmtdesc.index]+1, "%c", (fmtdesc.pixelformat>>8*1)&0xFF);
        sprintf(formats[fmtdesc.index]+2, "%c", (fmtdesc.pixelformat>>8*2)&0xFF);
        sprintf(formats[fmtdesc.index]+3, "%c", (fmtdesc.pixelformat>>8*3)&0xFF);

        printf("\"%s\"", formats[fmtdesc.index]);
        printf("（详细描述: %s）\n", fmtdesc.description);
        fmtdesc.index++;
        ret = true;
    }
    printf("\n");

    return ret;
}

// 获取摄像头格式信息（可调）
bool get_camfmt(int camfd)
{
    bzero(&fmt, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(ioctl(camfd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("获取摄像头格式信息失败: %s\n", strerror(errno));
        return false;
    }

    printf("摄像头当前的配置信息:\n");
    printf("分辨率: %d×%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

    printf("像素格式: ");
    switch(fmt.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_MJPEG:
        printf("V4L2_PIX_FMT_MJPEG\n");
        break;
    case V4L2_PIX_FMT_JPEG:
        printf("V4L2_PIX_FMT_JPEG\n");
        break;
    case V4L2_PIX_FMT_MPEG:
        printf("V4L2_PIX_FMT_MPEG\n");
        break;
    case V4L2_PIX_FMT_MPEG1:
        printf("V4L2_PIX_FMT_MPEG1\n");
        break;
    case V4L2_PIX_FMT_MPEG2:
        printf("V4L2_PIX_FMT_MPEG2\n");
        break;
    case V4L2_PIX_FMT_MPEG4:
        printf("V4L2_PIX_FMT_MPEG4\n");
        break;
    case V4L2_PIX_FMT_H264:
        printf("V4L2_PIX_FMT_H264\n");
        break;
    case V4L2_PIX_FMT_XVID:
        printf("V4L2_PIX_FMT_XVID\n");
        break;
    case V4L2_PIX_FMT_RGB24:
        printf("V4L2_PIX_FMT_RGB24\n");
        break;
    case V4L2_PIX_FMT_BGR24:
        printf("V4L2_PIX_FMT_BGR24\n");
        break;
    case V4L2_PIX_FMT_YUYV:
        printf("V4L2_PIX_FMT_YUYV\n");
        break;
    case V4L2_PIX_FMT_YYUV:
        printf("V4L2_PIX_FMT_YYUV\n");
        break;
    case V4L2_PIX_FMT_YVYU:
        printf("V4L2_PIX_FMT_YVYU\n");
        break;
    case V4L2_PIX_FMT_YUV444:
        printf("V4L2_PIX_FMT_YUV444\n");
        break;
    case V4L2_PIX_FMT_YUV410:
        printf("V4L2_PIX_FMT_YUV410\n");
        break;
    case V4L2_PIX_FMT_YUV420:
        printf("V4L2_PIX_FMT_YUV420\n");
        break;
    case V4L2_PIX_FMT_YVU420:
        printf("V4L2_PIX_FMT_YVU420\n");
        break;
    case V4L2_PIX_FMT_YUV422P:
        printf("V4L2_PIX_FMT_YUV422P\n");
        break;
    default:
        printf("未知\n");
    }
    printf("\n");

    return true;
}


// 获取摄像头设备的基本参数
bool get_camcap(int camfd)
{
    bzero(&cap, sizeof(cap));
    if(ioctl(camfd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        printf("获取摄像头基本信息失败: %s\n", strerror(errno));
        return false;
    }

    printf("驱动：%s\n", cap.driver);
    printf("显卡：%s\n", cap.card);
    printf("总线：%s\n", cap.bus_info);
    printf("版本：%d\n", cap.version);

    if((cap.capabilities&V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
    {
        printf("该设备为视频采集设备\n");
    }
    if((cap.capabilities&V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
    {
        printf("该设备支持流IO操作\n\n");
    }

    return true;
}

// 配置摄像头像素格式
void set_camfmt(int camfd)
{
    struct v4l2_format *tmp = calloc(1, sizeof(*tmp));
    bzero(tmp, sizeof(*tmp));

    // 设置视频设备的类型：CAPTURE即摄像头
    tmp->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // 试图设置摄像头的分辨率
    tmp->fmt.pix.width  = 640;
    tmp->fmt.pix.height = 480;

    printf("请选择你要配置的序号:\n");
    int n;
    scanf("%d", &n);
    getchar();

    if(!strncmp(formats[n], "JPEG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
    else if(!strncmp(formats[n], "MJPG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    else if(!strncmp(formats[n], "MPEG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_MPEG;
    else if(!strncmp(formats[n], "YUYV", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    else if(!strncmp(formats[n], "YVYU", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
    else if(!strncmp(formats[n], "H264", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    else
    {
        printf("对不起，所选格式无法配置.\n");
        exit(0);
    }

    // 设置摄像头捕获图像的方式
    tmp->fmt.pix.field = V4L2_FIELD_INTERLACED;

    // 使得以上设置参数生效
    if(ioctl(camfd, VIDIOC_S_FMT, tmp) == -1)
    {
        printf("ioctl() VIDIOC_S_FMT 失败了: %s\n", strerror(errno));
    }
}


void *convert(void *arg __attribute__((unused)))
{
    /*******************************
     R = Y + 1.042*(V-128);
     G = Y - 0.344*(U-128)-0.714*(V-128);
     B = Y + 1.772*(U-128);
    *******************************/
    pthread_detach(pthread_self());

    for(int i=0; i<256; i++)  //i--->Y
    {
        for(int j=0; j<256; j++)  //j--->V   j--->U
        {
            R[i][j] = i + 1.042*(j-128);
            R[i][j] = R[i][j]>255 ? 255 : R[i][j];
            R[i][j] = R[i][j]<0   ? 0   : R[i][j];

            B[i][j] = i + 1.772*(j-128);
            B[i][j] = B[i][j]>255 ? 255 : B[i][j];
            B[i][j] = B[i][j]<0   ? 0   : B[i][j];

            for(int k=0; k<256; k++)   //j--->U    k--->V
            {
                G[i][j][k] = i + 0.344*(j-128)-0.714*(k-128);
                G[i][j][k] = G[i][j][k]>255 ? 255 : G[i][j][k];
                G[i][j][k] = G[i][j][k]<0   ? 0   : G[i][j][k];
            }
        }
    }
    pthread_exit(NULL);
}


// void yuv2jpg(uint8_t *yuv)
// {
//     // 准备RGB数据
//     uint8_t *rgbdata = (uint8_t *)calloc(1, CAMERA_W * CAMERA_H * 3);

//     uint8_t Y0, U;
//     uint8_t Y1, V;

//     int yuv_offset, rgb_offset;
//     for(int i=0; i<CAMERA_H; i++)
//     {
//         for(int j=0; j<CAMERA_W; j+=2)
//         {
//             yuv_offset = ( CAMERA_W*i + j ) * 2;
//             rgb_offset = ( CAMERA_W*i + j ) * 3;

//             Y0 = *(yuv + yuv_offset + 0);
//             U  = *(yuv + yuv_offset + 1);
//             Y1 = *(yuv + yuv_offset + 2);
//             V  = *(yuv + yuv_offset + 3);

//             *(rgbdata + rgb_offset + 0) = R[Y0][V];
//             *(rgbdata + rgb_offset + 1) = G[Y0][U][V];
//             *(rgbdata + rgb_offset + 2) = B[Y0][U];

//             *(rgbdata + rgb_offset + 3) = R[Y1][V];
//             *(rgbdata + rgb_offset + 4) = G[Y1][U][V];
//             *(rgbdata + rgb_offset + 5) = B[Y1][U];
//         }
//     }

//     struct jpeg_compress_struct cinfo;
//     struct jpeg_error_mgr jerr;

//     cinfo.err = jpeg_std_error(&jerr);
//     jpeg_create_compress(&cinfo);


//     // 准备图片文件
//     FILE *fp = fopen("a.jpg", "w");
//     if(fp == NULL)
//     {
//         printf("创建文件[a.jpg]失败:%s\n", strerror(errno));
//         return;
//     }
//     else
//         printf("创建照片a.jpg成功\n");

//     jpeg_stdio_dest(&cinfo, fp);

//     cinfo.image_width  = CAMERA_W;
//     cinfo.image_height = CAMERA_H;
//     cinfo.input_components = 3;
//     cinfo.in_color_space   = JCS_RGB;

//     jpeg_set_defaults(&cinfo);
//     jpeg_set_quality(&cinfo, 80, TRUE);

//     jpeg_start_compress(&cinfo, TRUE);

//     JSAMPROW row_pointer[1];

//     // 开始编码，并将jpg数据存入指定的图片文件中
//     while(cinfo.next_scanline < cinfo.image_height)
//     {
//         row_pointer[0] = rgbdata + cinfo.next_scanline * CAMERA_W * 3;
//         jpeg_write_scanlines(&cinfo, row_pointer, 1);
//     }

//     jpeg_finish_compress(&cinfo);
//     jpeg_destroy_compress(&cinfo);

//     free(rgbdata);
//     fclose(fp);
// }



// 定义BMP文件头结构（14字节，小端序）
typedef struct {
    uint8_t  bfType[2];    // BMP文件标识，必须是"BM"（0x42, 0x4D）
    uint32_t bfSize;       // 整个文件的大小（字节）
    uint16_t bfReserved1;  // 保留，设为0
    uint16_t bfReserved2;  // 保留，设为0
    uint32_t bfOffBits;    // 像素数据相对于文件头的偏移量（14+40=54字节）
} BMP_FILE_HEADER;

// 定义BMP信息头结构（40字节，小端序）
typedef struct {
    uint32_t biSize;        // 信息头大小（40字节）
    int32_t  biWidth;       // 图像宽度（像素）
    int32_t  biHeight;      // 图像高度（像素，正数表示从下到上存储）
    uint16_t biPlanes;      // 色彩平面数，必须为1
    uint16_t biBitCount;    // 位深，24表示RGB三色各8位
    uint32_t biCompression; // 压缩方式，0表示无压缩
    uint32_t biSizeImage;   // 像素数据大小（字节，向上对齐到4的倍数）
    int32_t  biXPelsPerMeter;// 水平分辨率（像素/米，设为0即可）
    int32_t  biYPelsPerMeter;// 垂直分辨率（像素/米，设为0即可）
    uint32_t biClrUsed;     // 使用的颜色数，24位无需调色板，设为0
    uint32_t biClrImportant;// 重要颜色数，设为0
} BMP_INFO_HEADER;

// YUV转BMP（替换原yuv2jpg函数）
void yuv2bmp(uint8_t *yuv, const char *filename)
{
    // 1. 计算BMP关键参数
    int width = CAMERA_W;
    int height = CAMERA_H;
    int bit_count = 24; // 24位RGB
    int row_bytes = (width * bit_count + 31) / 32 * 4; // 每行字节数（4字节对齐）
    int pixel_data_size = row_bytes * height; // 像素数据总大小
    int file_total_size = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER) + pixel_data_size; // 文件总大小

    // 2. 分配RGB缓存（存储转换后的24位RGB数据）
    uint8_t *rgb_data = (uint8_t *)calloc(1, pixel_data_size);
    if (rgb_data == NULL) {
        printf("分配RGB缓存失败: %s\n", strerror(errno));
        return;
    }

    // 3. YUYV转RGB（沿用原转换逻辑，适配BMP的行对齐和存储顺序）
    uint8_t Y0, U, Y1, V;
    int yuv_offset, rgb_row_offset, rgb_col_offset;

    // BMP像素从下到上存储，因此从最后一行开始处理
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j += 2) {
            // 计算YUYV数据偏移（原顺序不变）
            yuv_offset = (width * i + j) * 2;
            // 计算RGB数据偏移（按BMP的行对齐处理）
            rgb_row_offset = (height - 1 - i) * row_bytes; // 反转行顺序
            rgb_col_offset = j * 3;

            // 提取YUYV 分量（原逻辑不变）
            Y0 = *(yuv + yuv_offset + 0);
            U  = *(yuv + yuv_offset + 1);
            Y1 = *(yuv + yuv_offset + 2);
            V  = *(yuv + yuv_offset + 3);

            // 转换第一个像素（Y0, U, V）→ RGB，BMP存储顺序为BGR
            *(rgb_data + rgb_row_offset + rgb_col_offset + 0) = B[Y0][U];    // B通道
            *(rgb_data + rgb_row_offset + rgb_col_offset + 1) = G[Y0][U][V]; // G通道
            *(rgb_data + rgb_row_offset + rgb_col_offset + 2) = R[Y0][V];    // R通道

            // 转换第二个像素（Y1, U, V）→ RGB
            *(rgb_data + rgb_row_offset + rgb_col_offset + 3) = B[Y1][U];    // B通道
            *(rgb_data + rgb_row_offset + rgb_col_offset + 4) = G[Y1][U][V]; // G通道
            *(rgb_data + rgb_row_offset + rgb_col_offset + 5) = R[Y1][V];    // R通道
        }
    }

    // 4. 初始化BMP文件头和信息头
    BMP_FILE_HEADER file_header = {0};
    BMP_INFO_HEADER info_header = {0};

    // 文件头赋值（注意"BM"的小端存储）
    file_header.bfType[0] = 'B';
    file_header.bfType[1] = 'M';
    file_header.bfSize = file_total_size;
    file_header.bfOffBits = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER); // 数据偏移54字节

    // 信息头赋值
    info_header.biSize = sizeof(BMP_INFO_HEADER);
    info_header.biWidth = width;
    info_header.biHeight = height; // 正数表示从下到上存储
    info_header.biPlanes = 1;
    info_header.biBitCount = bit_count;
    info_header.biCompression = 0; // 无压缩
    info_header.biSizeImage = pixel_data_size;

    // 5. 打开文件并写入数据
    FILE *fp = fopen(filename, "wb"); // 必须用"wb"（二进制写），避免文本模式转义
    if (fp == NULL) {
        printf("创建BMP文件[%s]失败: %s\n", filename, strerror(errno));
        free(rgb_data);
        return;
    }

    // 按顺序写入：文件头 → 信息头 → 像素数据
    fwrite(&file_header, sizeof(BMP_FILE_HEADER), 1, fp);
    fwrite(&info_header, sizeof(BMP_INFO_HEADER), 1, fp);
    fwrite(rgb_data, pixel_data_size, 1, fp);

    printf("创建BMP文件[%s]成功\n", filename);

    // 6. 释放资源
    fclose(fp);
    free(rgb_data);
}
