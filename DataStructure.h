#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <string>
#include <QImage>
#include <QPixmap>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace PIXMAP{
enum{
    FMT_NULL,FMT_RGB,FMT_YUV,FMT_GREY,FMT_BIN,FMT_HSI,FMT_YCbCr
};
enum {
    SELECT_R=1,SELECT_G=2,SELECT_B=4,SELECT_GREY=8
};	//标记需要计数的通道
}

using namespace std;


typedef unsigned char UNUM8;	//8位无符号数
typedef unsigned short UNUM16;	//16位无符号数
typedef unsigned int UNUM32;	//32位无符号数
typedef signed char SNUM8;		//8位有符号数
typedef signed short SNUM16;	//16位有符号数
typedef signed int SNUM32;		//32位有符号数
typedef struct {
    UNUM8 r, g, b, a,empty;
}Pixel32b;


//直接读取bmp格式(windows.h里面的)
//位图头文件 14字节
//位图信息头 40字节
//调色板(palette):颜色定义，长度可变
//图像数据

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

//位图头文件 14Byte 2+4+2+2+4
typedef struct{
    WORD bfType;    //文件类型，必须是字符串"BM" 即0x424D
    DWORD bfSize;        // file size
    WORD bfReserved1;   //保留字不考虑
    WORD bfReserved2;
    DWORD bfOffBits;    // offset from file header to pixel data
}BITMAPFILEHEADER;

//位图信息头 40Byte
typedef struct{
    DWORD biSize;        // structure size 固定40
    LONG biWidth;        //图像宽度
    LONG biHeight;      //图像高度
    WORD biPlanes;        // must be 1
    WORD biBitCount;    // bit number of one pixel
    DWORD biCompression;//压缩类型 BI_RGB不压缩
    DWORD biSizeImage;  //实际图像数据占用的字节数
    LONG  biXPelsPerMeter; //水平分辨率
    LONG  biYPelsPerMeter;  //垂直分辨率
    DWORD biClrUsed;    //实际使用的颜色数 Default=0
    DWORD biClrImportant;//重要的颜色数
}BITMAPINFOHEADER;

//调色板是一个数组，共有2^biBitCount个元素，数组中每个元素的类型为一个结构体，占据4字节
typedef struct{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
}RGBQUAD ;
//真彩色图24/32 BITMAPINFOHEADER后面直接是位图数据，就是RGB
//biBitCount为1 4 8时，图像数据是该像素颜色在调色板中对应的索引值；
//1，调色板数组仅有(0,0,0,0)  (255,255,255,0)
//4 调色板数组16元素，4bit表示一个像素颜色
//8 1byte表示一个像素，8-bit gray 调色板R=B=G, 8-bit color一般具有不同的调色板


class Pixmap
{
    unsigned int height, width, format;	//height为图像高度 width为图像宽度 format为状态
    UNUM8 *r, *g, *b, *a;	//r、g、b、a为4个通道的数组 顺序为从左下到右上，先行后列
    bool isOpen=false;
public:

    //从文件读入QImage自动探测
    Pixmap(QString fileName);
    //文件操作读入bmp
    Pixmap(QString fileName,QString fileType);
    //拷贝构造
    Pixmap(const Pixmap &pixmap);
    //设定位图宽高构造
    Pixmap(unsigned int width, unsigned int height, unsigned char value);



    ~Pixmap() { FreePixmap(); }
    int Load(const Pixmap &pixmap);
    int Load(const QImage &image);
    void FreePixmap();	//清空数据

    shared_ptr<QImage> Output();



    int ConvertFormat(unsigned int newFormat, int thre=-1);	//转换格式
    int ConvertToYUV();     //转化为YUV颜色格式
    int ConvertToRGB();     //转化为RGB颜色格式
    int ConvertToGrey();	//用YUV中的Y通道填充RGB，转换为灰阶图像
    int ConvertToGreyHSI(); //用HSI中的I通道填充RGB，转换为灰阶图像
    int ConvertToHSI();     //转化为HSI颜色格式,注意灰度图无法转化,转化后的值是RGB->ISH
    int ConvertToYCbCr(); //
    unsigned char OtsuGetThre();	//获得大津法阈值
    int ConvertToBin(int thre=-1);	//二值化，若thre不在0~255之间，则先进行大津法求阈值操作


    Pixmap* OrderDitherToBin(double * filter, unsigned int filterSize);
    Pixmap* UnOrderedDitherToBin(double * filter, unsigned int filterSize);

    int POWTransform(double gamma=2.2);
    int LOGTransform(double param=0.2);


    void ChangeLuma(int del);	//改变YUV通道中的Y值，并恢复原格式
    int InverseColor();	//反色
    int LogOperation();	//对数操作
    int HistoEqualizing();	//直方图均衡
    static double Gaussian(double x,double r);//计算高斯分布

    const unsigned char *getRHead() const { return r; }	//返回r数组
    const unsigned char *getGHead() const { return g; }	//返回g数组
    const unsigned char *getBHead() const { return b; }	//返回b数组
    const unsigned char *getAHead() const { return a; }	//返回a数组

    unsigned char *getRHead() { return r; }	//非常量版 返回r数组
    unsigned char *getGHead() { return g; }	//非常量版 返回g数组
    unsigned char *getBHead() { return b; }	//非常量版 返回b数组
    unsigned char *getAHead() { return a; }	//非常量版 返回a数组

    unsigned int getFormat() const { return format; }	//返回格式
    unsigned int getWidth(){return width;}
    unsigned int getHeight(){return height;}

    const unsigned char *getR(unsigned int x, unsigned int y) const { if (x < width&&y < height) return r + y*width + x; return NULL; }//返回(x,y)坐标处的r值指针
    const unsigned char *getG(unsigned int x, unsigned int y) const { if (x < width&&y < height)return g + y*width + x; return NULL; } //返回(x,y)坐标处的g值指针
    const unsigned char *getB(unsigned int x, unsigned int y) const { if (x < width&&y < height) return b + y*width + x; return NULL; } //返回(x,y)坐标处的b值指针
    const unsigned char *getA(unsigned int x, unsigned int y) const { if (x < width&&y < height) return a + y*width + x; return NULL; } //返回(x,y)坐标处的a值指针
    Pixel32b getPixel(unsigned int x,unsigned int y) const;	//获得(x,y)处的像素点颜色
    bool getPicOPen();//是否加载图片


    unsigned char *getR(unsigned int x, unsigned int y) { if (x < width&&y < height) return r + y*width + x; return NULL; } //非常量版 返回(x,y)坐标处的r值指针
    unsigned char *getG(unsigned int x, unsigned int y) { if (x < width&&y < height) return g + y*width + x; return NULL; }//非常量版 返回(x,y)坐标处的g值指针
    unsigned char *getB(unsigned int x, unsigned int y) { if (x < width&&y < height) return b + y*width + x; return NULL; }//非常量版 返回(x,y)坐标处的b值指针
    unsigned char *getA(unsigned int x, unsigned int y) { if (x < width&&y < height) return a + y*width + x; return NULL; }//非常量版 返回(x,y)坐标处的a值指针

};


class Histogram3c
{
    unsigned int *hgR, *hgG, *hgB;	//3通道颜色计数
public:
    Histogram3c();
    Histogram3c(Pixmap &pixmap,unsigned char select=PIXMAP::SELECT_R|PIXMAP::SELECT_G|PIXMAP::SELECT_B);
    ~Histogram3c() { FreeHistogram3c(); }
    void FreeHistogram3c();
    void LoadPixmap(Pixmap &pixmap, unsigned char select = PIXMAP::SELECT_R|PIXMAP::SELECT_G|PIXMAP::SELECT_B);	//对Pixmap类计算直方图
    unsigned int* CalcHistogram(unsigned int total,const unsigned char *color);
    unsigned int getRCount(unsigned char r) const;
    unsigned int getGCount(unsigned char g) const;
    unsigned int getBCount(unsigned char b) const;
    unsigned int *getRHead() { return hgR; }
    unsigned int *getGHead() { return hgG; }
    unsigned int *getBHead() { return hgB; }
};

#endif // DATASTRUCTURE_H
