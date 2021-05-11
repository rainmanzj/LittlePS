#include "DataStructure.h"
#include <cstdlib>
#include <QDebug>
using namespace PIXMAP;
#ifndef PI
#define PI 3.141592
#endif

typedef struct strBmpFileHeader
{
    UNUM16 bfType, bfReserved1, bfReserved2;
    UNUM32 bfSize, bfOffBits;
}BmpFileHeader;

typedef struct strBmpFileInformation
{
    UNUM16 biPlanes, biBitCount;
    UNUM32 biSize, biWidth, biCompression, biSizeImage, biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImportant;
    SNUM32 biHeight;
}BmpFileInformation;


//设定位图宽高构造
Pixmap::Pixmap(unsigned int width, unsigned int height, unsigned char value=0) :format(PIXMAP::FMT_NULL), r(NULL), g(NULL), b(NULL), a(NULL)
{
    r = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    g = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    b = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    a = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    memset(r, value, width*height);
    memset(g, value, width*height);
    memset(b, value, width*height);
    memset(a, value, width*height);
    format = PIXMAP::FMT_RGB;
}

Pixmap::Pixmap(string fileName):height(0),width(0),format(PIXMAP::FMT_NULL), r(NULL), g(NULL), b(NULL), a(NULL)

{
    QImage tmpImage(QString::fromStdString(fileName));
    tmpImage.convertToFormat(QImage::Format_ARGB32);
    if(!tmpImage.isNull())
    {
        Load(tmpImage);
    }

}

int Pixmap::Load(const Pixmap &pixmap)
{
    if ((format = pixmap.format) != PIXMAP::FMT_NULL)
    {
        height = pixmap.height;
        width = pixmap.width;
        r = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        g = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        b = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        a = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        memcpy(r, pixmap.r, width*height);
        memcpy(g, pixmap.g, width*height);
        memcpy(b, pixmap.b, width*height);
        memcpy(a, pixmap.a, width*height);
        return 0;
    }
    else
        FreePixmap();
    return 0;
}

void Pixmap::FreePixmap()
{
    free(r);
    free(g);
    free(b);
    free(a);
    r = NULL;
    g = NULL;
    b = NULL;
    a = NULL;
    width = 0;
    height = 0;
    format = PIXMAP::FMT_NULL;
}

int Pixmap::Load(const QImage &image)
{
    if(image.isNull())
        return 1;
    FreePixmap();
    width=image.width();
    height=image.height();
    r = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    g = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    b = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    a = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    format=PIXMAP::FMT_RGB;
    QRgb color;
    unsigned char *pr=r,*pg=g,*pb=b,*pa=a;


        for(unsigned int y=0;y<height;y++)
            for(unsigned int x=0;x<width;x++)
        {
            color=image.pixel(QPoint(x,y));
            *pr++=(color>>16)&0x000000FF;
            *pg++=(color>>8)&0x000000FF;
            *pb++=color&0x000000FF;
            *pa++=(color>>24)&0x000000FF;
        }
    return 0;
}

shared_ptr<QImage> Pixmap::Output()
{
    if(format==PIXMAP::FMT_NULL||height==0||width==0)
        return nullptr;
    shared_ptr<QImage> image(new QImage(QSize(width,height),QImage::Format_ARGB32));
    QColor color;
    unsigned char *pr=r,*pg=g,*pb=b,*pa=a;


        for(unsigned int y=0;y<height;y++)
            for(unsigned int x=0;x<width;x++)
        {
            color.setRed(*pr++);
            color.setGreen(*pg++);
            color.setBlue(*pb++);
            color.setAlpha(*pa++);
            image->setPixel(QPoint(x,y),color.rgba());
        }
    return image;
}

//-----------------------Origin Pixmap---------------------//

int Pixmap::ConvertFormat(unsigned int newFormat, int thre)
{
    switch (newFormat)
    {
    case FMT_NULL:FreePixmap(); return 0; break;
    case FMT_RGB:return ConvertToRGB(); break;
    case FMT_YUV:return ConvertToYUV(); break;
    case FMT_GREY:return ConvertToGrey(); break;
    case FMT_BIN:return ConvertToBin(thre); break;
    default:return 1;
    }
}

int Pixmap::ConvertToYUV()
{
    switch (format)
    {
    case FMT_NULL:return 1;
    case FMT_YUV:return 0;
    case FMT_BIN:
    case FMT_GREY:
    case FMT_RGB:
        UNUM8 *rp = r, *gp = g, *bp = b, cr, cg, cb, y;
        for (unsigned int i = 0; i < width*height; i++)
        {
            cr = *rp;
            cb = *bp;
            cg = *gp;
            *rp++ = y = (UNUM8)(0.3*cr + 0.59*cg + 0.11*cb);
            *gp++ = (UNUM8)(0.87*(0.493*((int)cb - y)) + 127); // 0.87U+127
            *bp++ = (UNUM8)(0.8*(0.877*((int)cr - y)) + 127); //0.8V+127
        }
        format = FMT_YUV;
    }
    return 0;
}

template<typename T> static T ClipToUNUM8(T x)
{
    if (x < 0)
        return 0;
    return (x > 255) ? 255 : x;
}

int Pixmap::ConvertToRGB()
{
    switch (format)
    {
    case FMT_NULL:return 1;
    case FMT_BIN:format = FMT_RGB; return 0;
    case FMT_GREY:format = FMT_RGB; return 0;
    case FMT_RGB:return 0;
    case FMT_YUV:
        UNUM8 *yp = r, *up = g, *vp = b;
        double y, u, v;
        for (unsigned int i = 0; i < width*height; i++)
        {
            y = *yp;
            u = (*up - 127.0) / 0.87;
            v = (*vp - 127.0) / 0.8;
            *yp++ = (UNUM8)ClipToUNUM8(y + 1.14*v);
            *up++ = (UNUM8)ClipToUNUM8(y - 0.39*u - 0.58*v);
            *vp++ = (UNUM8)ClipToUNUM8(y + 2.03*u);
        }
        format = FMT_RGB;
    }
    return 0;
}

int Pixmap::ConvertToGrey()
{
    switch (format)
    {
    case FMT_BIN:	format = FMT_GREY; return 0;
    case FMT_GREY:return 0;
    case FMT_NULL:return 1;
    case FMT_YUV:ConvertToRGB();
    case FMT_RGB:
        UNUM8 cr, cg, cb, *pr = r, *pg = g, *pb = b;
        for (unsigned int i = 0; i < width*height; i++)
        {
            cr = *pr;
            cg = *pg;
            cb = *pb;
            *pr++ = *pg++ = *pb++ = (UNUM8)(0.3*cr + 0.59*cg + 0.11*cb);
        }
        format = FMT_GREY;
    }
    return 0;
}

unsigned char Pixmap::OtsuGetThre()
{
    if (format != FMT_GREY)
        return 0;
    Histogram3c histo(*this, SELECT_R);
    unsigned char thre = 0;
    unsigned int totalPixel = width*height, currCount, currGrey;
    unsigned int  *his = histo.getRHead();
    double w0, w1, u0, u1, max = -1, curr, totalGrey = 0;
    for (unsigned int i = 0; i < 256; i++)
        totalGrey += his[i] * i;
    for (unsigned int i = 0; i < 256; i++)
    {
        w0 = w1 = u0 = u1 = 0.0;
        currCount = currGrey = 0;
        for (unsigned int j = 0; j < i; j++)
        {
            currCount += his[j];
            currGrey += his[j] * j;
        }
        w0 = ((double)currCount) / totalPixel;
        u0 = currGrey ? currGrey / currCount : 0;
        w1 = ((double)totalPixel - currCount) / totalPixel;
        u1 = (totalPixel - currCount) ? (totalGrey - currGrey) / (totalPixel - currCount) : 0;
        curr = w0*w1*(u0 - u1)*(u0 - u1);
        if (curr > max)
        {
            max = curr;
            thre = i;
        }
    }
    return thre;
}

int Pixmap::ConvertToBin(int thre)
{
    switch (format)
    {
    case FMT_BIN:return 0;
    case FMT_NULL:return 1;
    case FMT_YUV:
    case FMT_RGB:ConvertToGrey();
    case FMT_GREY:
        if (thre < 0 || thre>255)
            thre = OtsuGetThre();
        UNUM8 *pr = r, *pg = g, *pb = b;
        for (unsigned int i = 0; i < width*height; i++)
        {
            *pr++ = *pg++ = *pb++ = ((*pr > thre) ? 255 : 0);
        }
        format = FMT_BIN;
    }
    return 0;
}

void Pixmap::ChangeLuma(int del)
{
    del = del > 255 ? 255 : del;
    del = del < -255 ? -255 : del;
    UNUM8 *yp = r;
    unsigned int oldFormat = format;
    ConvertToYUV();
    for (unsigned int i = 0; i < width*height; i++, yp++)
        *yp = (UNUM8)ClipToUNUM8((double)*yp + del);
    ConvertFormat(oldFormat);
}

int Pixmap::InverseColor()
{
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    unsigned char *pr = r, *pg = g, *pb = b;
    for (unsigned int i = 0; i < width*height; i++)
    {
        *pr++ = 255 - *pr;
        *pg++ = 255 - *pg;
        *pb++ = 255 - *pb;
    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

int Pixmap::LogOperation()
{
    if (format == FMT_NULL)
        return 1;
    unsigned int oldFormat = format;
    ConvertToYUV();
    UNUM8 maxLuma = 0, *py = r;
    for (unsigned int i = 0; i < width*height; i++, py++)
        if (*py > maxLuma)
            maxLuma = *py;
    py = r;
    for (unsigned int i = 0; i < width*height; i++)
        *py++ = (UNUM8)(log(*py / 255.0 + 1) / log(maxLuma / 255.0 + 1) * 255);
    ConvertFormat(oldFormat);
    return 0;
}

int Pixmap::HistoEqualizing()
{
    if (format == FMT_NULL)
        return 1;
    unsigned int oldFormat = format;
    ConvertToYUV();
    Histogram3c histo(*this, SELECT_R);
    UNUM8 maxY = 0, minY = 255, *py = r, diffY;
    unsigned int *his = histo.getRHead(), size = width*height;
    if (!size)
        return 1;
    double *hisDis = (double*)malloc(sizeof(double) * 256), *hisCumu = (double*)malloc(sizeof(double) * 256);	//hisDis为该值出现概率 hisCumu为累积概率
    for (unsigned int i = 0; i < 256; i++)
    {
        if (i > maxY&&his[i] > 0)
            maxY = i;
        if (i < minY&&his[i] > 0)
            minY = i;
        hisDis[i] = (double)his[i] / size;
        hisCumu[i] = (i == 0) ? hisDis[i] : hisDis[i] + hisCumu[i - 1];
    }
    diffY = maxY - minY;
    for (unsigned int i = 0; i < size; i++)
        *py++ = (UNUM8)(hisCumu[*py] * diffY + minY);
    ConvertFormat(oldFormat);
    free(hisDis);
    free(hisCumu);
    return 0;
}



Pixel32b Pixmap::getPixel(unsigned int x, unsigned int y) const
{
    Pixel32b res = { 0,0,0,0,1 };
    if (x >= width || y >= height)
        return res;
    res.empty = 0;
    res.r = *getR(x, y);
    res.g = *getG(x, y);
    res.b = *getB(x, y);
    res.a = *getA(x, y);
    return res;
}

double Pixmap::Gaussian(double x,double r)
{
    const double tmp=pow(2*PI,0.5);
    return pow(2.7182818,-x*x/(2*r*r))/(r*tmp);
}


//----------------------------Pixmap End-----------------------------//
//----------------------------histogram begin------------------------//

#define RANGEOFCOLOR 256

Histogram3c::Histogram3c()
{
    hgR = (unsigned int*)malloc(sizeof(unsigned int)*RANGEOFCOLOR);
    hgG = (unsigned int*)malloc(sizeof(unsigned int)*RANGEOFCOLOR);
    hgB = (unsigned int*)malloc(sizeof(unsigned int)*RANGEOFCOLOR);
    memset(hgR, 0, RANGEOFCOLOR * sizeof(unsigned int));
    memset(hgG, 0, RANGEOFCOLOR * sizeof(unsigned int));
    memset(hgB, 0, RANGEOFCOLOR * sizeof(unsigned int));
}

Histogram3c::Histogram3c(Pixmap & pixmap, unsigned char select):hgR(NULL),hgG(NULL),hgB(NULL)
{
    LoadPixmap(pixmap,select);
}

void Histogram3c::FreeHistogram3c()
{
    free(hgR);
    free(hgG);
    free(hgB);
    hgR = NULL;
    hgB = NULL;
    hgG = NULL;
}

void Histogram3c::LoadPixmap(Pixmap & pixmap, unsigned char select)
{
    FreeHistogram3c();
    unsigned int total = pixmap.getHeight()*pixmap.getWidth();
    if (select & 1)
        hgR = CalcHistogram(total, pixmap.getRHead());
    if (select & 2)
        hgG = CalcHistogram(total, pixmap.getGHead());
    if (select & 4)
        hgB = CalcHistogram(total, pixmap.getBHead());
}

unsigned int* Histogram3c::CalcHistogram(unsigned int total, const unsigned char * color)
{
    if (!total)
        return NULL;
    unsigned int *his = (unsigned int*)malloc(sizeof(unsigned int)*RANGEOFCOLOR);
    memset(his,0,sizeof(unsigned int)*RANGEOFCOLOR);
    for (unsigned int i = 0; i < total; i++)
        (his[*color++])++;
    return his;
}

unsigned int Histogram3c::getRCount(unsigned char r) const
{
    if (hgR == NULL)
        return 0;
    return hgR[r];
}

unsigned int Histogram3c::getGCount(unsigned char g) const
{
    if (hgG == NULL)
        return 0;
    return hgG[g];
}

unsigned int Histogram3c::getBCount(unsigned char b) const
{
    if (hgB == NULL)
        return 0;
    return hgB[b];
}

