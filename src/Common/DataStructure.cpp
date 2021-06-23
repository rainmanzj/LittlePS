#include "../Common/DataStructure.h"
#include <cstdlib>
#include <QDebug>
#include <opencv2/dnn_superres.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <QFileDialog>
#include <QCoreApplication>

using namespace std;
using namespace cv;
using namespace dnn;
using namespace dnn_superres;
using namespace PIXMAP;
#ifndef PI
#define PI 3.141592
#endif

Pixmap::Pixmap(int posX, int posY, int type, const string &name, double scaleX, double scaleY, double angle,unsigned int width,
               unsigned int height,unsigned char value):BaseShape(posX, posY, type, name, scaleX, scaleY, angle)
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

Pixmap::Pixmap(const string &name,string fileName)
    :r(NULL),g(NULL),b(NULL),a(NULL),width(0),height(0),format(PIXMAP::FMT_NULL),
      BaseShape(0, 0, SHAPE::PIXMAP, name, 1.0,1.0,0.0)
{
    QImage tmpImage(QString::fromStdString(fileName));
    tmpImage.convertToFormat(QImage::Format_ARGB32);
    if(!tmpImage.isNull())
    {
        Load(tmpImage);
       BaseShape::setPosX(width/2);
        BaseShape::setPosY(height/2);
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


        for(int y=0;y<height;y++)
            for(int x=0;x<width;x++)
        {
            color=image.pixel(QPoint(x,y));
            *pr++=(color>>16)&0x000000FF;
            *pg++=(color>>8)&0x000000FF;
            *pb++=color&0x000000FF;
            *pa++=(color>>24)&0x000000FF;
        }
}

shared_ptr<QImage> Pixmap::Output()
{
    if(format==PIXMAP::FMT_NULL||height==0||width==0)
        return nullptr;
    shared_ptr<QImage> image(new QImage(QSize(width,height),QImage::Format_ARGB32));
    QColor color;
    unsigned char *pr=r,*pg=g,*pb=b,*pa=a;


        for(int y=0;y<height;y++)
            for(int x=0;x<width;x++)
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

shared_ptr<Pixmap>  Pixmap::AddBorder(unsigned int borderWidth, int mode) const
{
    if (this->format == FMT_NULL)
        return nullptr;
    shared_ptr<Pixmap> res = shared_ptr<Pixmap>(new Pixmap(width + borderWidth * 2, height + borderWidth * 2));

    res->format=format;
    for (unsigned int x = borderWidth; x < borderWidth + width; x++)
        for (unsigned int y = borderWidth; y < borderWidth + height; y++)
        {
            *(res->getR(x, y)) = *(this->getR(x - borderWidth, y - borderWidth));
            *(res->getG(x, y)) = *(this->getG(x - borderWidth, y - borderWidth));
            *(res->getB(x, y)) = *(this->getB(x - borderWidth, y - borderWidth));
            *(res->getA(x, y)) = *(this->getA(x - borderWidth, y - borderWidth));
        }
    for (unsigned int i = 0; i < borderWidth; i++)
    {
        for (unsigned int x = 0; x < width; x++)
        {
            //Top
            *(res->getR(x + borderWidth, borderWidth - i - 1)) = *(this->getR(x, i%height));
            *(res->getG(x + borderWidth, borderWidth - i - 1)) = *(this->getG(x, i%height));
            *(res->getB(x + borderWidth, borderWidth - i - 1)) = *(this->getB(x, i%height));
            *(res->getA(x + borderWidth, borderWidth - i - 1)) = *(this->getA(x, i%height));
            //Buttom
            *(res->getR(x + borderWidth, borderWidth + height + i)) = *(this->getR(x, height - i%height - 1));
            *(res->getG(x + borderWidth, borderWidth + height + i)) = *(this->getG(x, height - i%height - 1));
            *(res->getB(x + borderWidth, borderWidth + height + i)) = *(this->getB(x, height - i%height - 1));
            *(res->getA(x + borderWidth, borderWidth + height + i)) = *(this->getA(x, height - i%height - 1));
        }

        for (unsigned int y = 0; y < height; y++)
        {
            //Left
            *(res->getR(borderWidth - i - 1, y + borderWidth)) = *(this->getR(i%width, y));
            *(res->getG(borderWidth - i - 1, y + borderWidth)) = *(this->getG(i%width, y));
            *(res->getB(borderWidth - i - 1, y + borderWidth)) = *(this->getB(i%width, y));
            *(res->getA(borderWidth - i - 1, y + borderWidth)) = *(this->getA(i%width, y));
            //Right
            *(res->getR(width + borderWidth + i, y + borderWidth)) = *(this->getR(width - i%width - 1, y));
            *(res->getG(width + borderWidth + i, y + borderWidth)) = *(this->getG(width - i%width - 1, y));
            *(res->getB(width + borderWidth + i, y + borderWidth)) = *(this->getB(width - i%width - 1, y));
            *(res->getA(width + borderWidth + i, y + borderWidth)) = *(this->getA(width - i%width - 1, y));

        }

    }
    for (unsigned int x = 0; x < borderWidth; x++)
        for (unsigned int y = 0; y < borderWidth; y++)
        {
            *(res->getR(x, y)) = *(this->getR(0, 0));
            *(res->getR(x + width + borderWidth, y)) = *(this->getR(width - 1, 0));
            *(res->getR(x, y + height + borderWidth)) = *(this->getR(0, height - 1));
            *(res->getR(x + width + borderWidth, y + height + borderWidth)) = *(this->getR(width - 1, height - 1));

            *(res->getG(x, y)) = *(this->getG(0, 0));
            *(res->getG(x + width + borderWidth, y)) = *(this->getG(width - 1, 0));
            *(res->getG(x, y + height + borderWidth)) = *(this->getG(0, height - 1));
            *(res->getG(x + width + borderWidth, y + height + borderWidth)) = *(this->getG(width - 1, height - 1));

            *(res->getB(x, y)) = *(this->getB(0, 0));
            *(res->getB(x + width + borderWidth, y)) = *(this->getB(width - 1, 0));
            *(res->getB(x, y + height + borderWidth)) = *(this->getB(0, height - 1));
            *(res->getB(x + width + borderWidth, y + height + borderWidth)) = *(this->getB(width - 1, height - 1));

            *(res->getA(x, y)) = *(this->getA(0, 0));
            *(res->getA(x + width + borderWidth, y)) = *(this->getA(width - 1, 0));
            *(res->getA(x, y + height + borderWidth)) = *(this->getA(0, height - 1));
            *(res->getA(x + width + borderWidth, y + height + borderWidth)) = *(this->getA(width - 1, height - 1));
        }

    return res;
}


shared_ptr<Pixmap> Pixmap::Convolution(double * filter, unsigned int filterSize, int normalization, double **outR, double **outG, double **outB) const
{
    if (this->format == FMT_NULL || filter == NULL || !(filterSize % 2))
        return nullptr;
    shared_ptr<Pixmap> src = this->AddBorder(filterSize / 2), res = shared_ptr<Pixmap>(new Pixmap(width, height));
    double *dataR = (double*)malloc(sizeof(double)*width*height);
    double *dataG = (double*)malloc(sizeof(double)*width*height);
    double *dataB = (double*)malloc(sizeof(double)*width*height);
    double tmpR, tmpG, tmpB, tmpA;
    int halfSize = filterSize / 2;
    for (unsigned int x = halfSize; x < halfSize + width; x++)
        for (unsigned int y = halfSize; y < halfSize + height; y++)
        {
            tmpR = tmpG = tmpB = tmpA = 0;
            for (int i = -halfSize; i <= halfSize; i++)
                for (int j = -halfSize; j <= halfSize; j++)
                {
                    tmpR += *(filter + (i + halfSize)*filterSize + j + halfSize)* (*(src->getR((unsigned int)(x + j), (unsigned int)(y + i))));
                    tmpG += *(filter + (i + halfSize)*filterSize + j + halfSize)* (*(src->getG((unsigned int)(x + j), (unsigned int)(y + i))));
                    tmpB += *(filter + (i + halfSize)*filterSize + j + halfSize)* (*(src->getB((unsigned int)(x + j), (unsigned int)(y + i))));
                }
            *(dataR + (y - halfSize)*width + x - halfSize) = tmpR;
            *(dataG + (y - halfSize)*width + x - halfSize) = tmpG;
            *(dataB + (y - halfSize)*width + x - halfSize) = tmpB;
        }
    double minR = INFINITY, maxR = -INFINITY, minG = INFINITY, maxG = -INFINITY, minB = INFINITY, maxB = -INFINITY;
    double *rp = dataR, *gp = dataG, *bp = dataB;
    if (normalization)
    {
        for (unsigned int i = 0; i < width*height; i++, rp++, gp++, bp++)
        {
            if (*rp > maxR) maxR = *rp;
            if (*rp < minR) minR = *rp;
            if (*gp > maxG) maxG = *gp;
            if (*gp < minG) minG = *gp;
            if (*bp > maxB) maxB = *bp;
            if (*bp < minB) minB = *bp;
        }
        rp = dataR;
        gp = dataG;
        bp = dataB;
        double diffR = maxR - minR, diffG = maxG - minG, diffB = maxB - minB;
        for (unsigned int i = 0; i < width * height; i++, rp++, gp++, bp++)
        {
            *rp = (*rp - minR) / diffR * 256;
            *gp = (*gp - minG) / diffG * 256;
            *bp = (*bp - minB) / diffB * 256;
        }
        rp = dataR;
        gp = dataG;
        bp = dataB;
    }
    UNUM8 *resR = res->getRHead(), *resG = res->getGHead(), *resB = res->getBHead(), *resA = res->getAHead();
    const UNUM8 *srcA = this->getAHead();
    for (unsigned int i = 0; i < width*height; i++, rp++, gp++, bp++, resR++, resG++, resB++, resA++, srcA++)
    {
        *resR = (UNUM8)ClipToUNUM8(*rp);
        *resG = (UNUM8)ClipToUNUM8(*gp);
        *resB = (UNUM8)ClipToUNUM8(*bp);
        *resA = *srcA;
    }
    if (outR == NULL)
        delete dataR;
    else
        *outR = dataR;

    if (outG == NULL)
        delete dataG;
    else
        *outG = dataG;

    if (outB == NULL)
        delete dataB;
    else
        *outB = dataB;
    //delete src;

    return res;
}

shared_ptr<Pixmap> Pixmap::ConvolutionGet(double * filter, unsigned int filterSize, int normalization, double **outR, double **outG, double **outB)
{
    if (this->format == FMT_NULL || filter == NULL || !(filterSize % 2))
        return nullptr;
    shared_ptr<Pixmap> src = this->AddBorder(filterSize / 2), res = shared_ptr<Pixmap>(new Pixmap(width, height));
    res->setPosX(getPosX());
    res->setPosY(getPosY());
    res->setAngle(getAngle());
    res->setName(getName());
    res->setScaleX(getScaleX());
    res->setScaleY(getScaleY());
    res->setType(getType());
    double *dataR = (double*)malloc(sizeof(double)*width*height);
    double *dataG = (double*)malloc(sizeof(double)*width*height);
    double *dataB = (double*)malloc(sizeof(double)*width*height);
    double tmpR, tmpG, tmpB, tmpA;
    int halfSize = filterSize / 2;
    for (unsigned int x = halfSize; x < halfSize + width; x++)
        for (unsigned int y = halfSize; y < halfSize + height; y++)
        {
            tmpR = tmpG = tmpB = tmpA = 0;
            for (int i = -halfSize; i <= halfSize; i++)
                for (int j = -halfSize; j <= halfSize; j++)
                {
                    tmpR += *(filter + (i + halfSize)*filterSize + j + halfSize)* (*(src->getR((unsigned int)(x + j), (unsigned int)(y + i))));
                    tmpG += *(filter + (i + halfSize)*filterSize + j + halfSize)* (*(src->getG((unsigned int)(x + j), (unsigned int)(y + i))));
                    tmpB += *(filter + (i + halfSize)*filterSize + j + halfSize)* (*(src->getB((unsigned int)(x + j), (unsigned int)(y + i))));
                }
            *(dataR + (y - halfSize)*width + x - halfSize) = tmpR;
            *(dataG + (y - halfSize)*width + x - halfSize) = tmpG;
            *(dataB + (y - halfSize)*width + x - halfSize) = tmpB;
        }
    double minR = INFINITY, maxR = -INFINITY, minG = INFINITY, maxG = -INFINITY, minB = INFINITY, maxB = -INFINITY;
    double *rp = dataR, *gp = dataG, *bp = dataB;
    if (normalization)
    {
        for (unsigned int i = 0; i < width*height; i++, rp++, gp++, bp++)
        {
            if (*rp > maxR) maxR = *rp;
            if (*rp < minR) minR = *rp;
            if (*gp > maxG) maxG = *gp;
            if (*gp < minG) minG = *gp;
            if (*bp > maxB) maxB = *bp;
            if (*bp < minB) minB = *bp;
        }
        rp = dataR;
        gp = dataG;
        bp = dataB;
        double diffR = maxR - minR, diffG = maxG - minG, diffB = maxB - minB;
        for (unsigned int i = 0; i < width * height; i++, rp++, gp++, bp++)
        {
            *rp = (*rp - minR) / diffR * 256;
            *gp = (*gp - minG) / diffG * 256;
            *bp = (*bp - minB) / diffB * 256;
        }
        rp = dataR;
        gp = dataG;
        bp = dataB;
    }
    UNUM8 *resR = res->getRHead(), *resG = res->getGHead(), *resB = res->getBHead(), *resA = res->getAHead();
    const UNUM8 *srcA = this->getAHead();
    for (unsigned int i = 0; i < width*height; i++, rp++, gp++, bp++, resR++, resG++, resB++, resA++, srcA++)
    {
        *resR = (UNUM8)ClipToUNUM8(*rp);
        *resG = (UNUM8)ClipToUNUM8(*gp);
        *resB = (UNUM8)ClipToUNUM8(*bp);
        *resA = *srcA;
    }
    if (outR == NULL)
        delete dataR;
    else
        *outR = dataR;

    if (outG == NULL)
        delete dataG;
    else
        *outG = dataG;

    if (outB == NULL)
        delete dataB;
    else
        *outB = dataB;
    //delete src;
    Load(*res);
    return res;
}


shared_ptr<Pixmap>  Pixmap::LaplacianEnhance(double * filter, unsigned int filterSize)
{
    double *inputFilter;
    if (filter == NULL || filterSize == 0)
    {
        inputFilter = (double*)malloc(sizeof(double) * 9);
        for (int i = 0; i < 9; i++)
            inputFilter[i] = 1;
        inputFilter[4] = -8;
        filterSize = 3;
    }
    else
        inputFilter = filter;
    double **outR = (double**)malloc(sizeof(double*)), **outG = (double**)malloc(sizeof(double*)), **outB = (double**)malloc(sizeof(double*));
    shared_ptr<Pixmap> res = Convolution(inputFilter, filterSize, 0, outR, outG, outB);
    if (res == nullptr)
        return nullptr;
    res->setPosX(getPosX());
    res->setPosY(getPosY());
    res->setAngle(getAngle());
    res->setName(getName());
    res->setScaleX(getScaleX());
    res->setScaleY(getScaleY());
    res->setType(getType());
    UNUM8 *resR = res->getRHead(), *resG = res->getGHead(), *resB = res->getBHead();
    const UNUM8 *leftR = getRHead(), *leftG = getGHead(), *leftB = getBHead();
    double *tmpR = *outR, *tmpG = *outG, *tmpB = *outB;
    for (unsigned int i = 0; i < width*height;  i++, resR++, resB++, resG++,leftR++, leftG++, leftB++,tmpR++,tmpG++,tmpB++)
    {
        *resR = (UNUM8)ClipToUNUM8(*leftR - *tmpR);
        *resG = (UNUM8)ClipToUNUM8(*leftG - *tmpG);
        *resB = (UNUM8)ClipToUNUM8(*leftB - *tmpB);
    }
    delete *outR;
    delete *outG;
    delete *outB;
    delete outR;
    delete outG;
    delete outB;
    Load(*res);
    return res;
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

shared_ptr<Pixmap> Pixmap::BilateralFiltering(int filterSize,double intenPara,double spacePara)
{
    if (this->format == FMT_NULL)
        return nullptr;
    if(filterSize<0) filterSize=(int)(0.05*pow(width*width+height*height,0.5));
    if(!(filterSize%2)) filterSize++;
    if(spacePara<0) spacePara=0.02*pow(width*width+height*height,0.5);
    if(intenPara<0) intenPara=0.15;
    shared_ptr<Pixmap> src = this->AddBorder(filterSize / 2), res = shared_ptr<Pixmap>(new Pixmap(width, height));
    src->ConvertToYUV();
    res->format=FMT_YUV;
    res->setPosX(getPosX());
    res->setPosY(getPosY());
    res->setAngle(getAngle());
    res->setName(getName());
    res->setScaleX(getScaleX());
    res->setScaleY(getScaleY());
    res->setType(getType());
    int halfSize = filterSize / 2;
    double *gauSpace=(double*)malloc(sizeof(double)*filterSize*filterSize);
    double wp,factor,gauInt;
    int gaui=0,i;
    for (int i = -halfSize; i <= halfSize; i++)
        for (int j = -halfSize; j <= halfSize; j++)
            gauSpace[gaui++]=Gaussian(pow(i*i+j*j,0.5),spacePara);
    double srcY,resY=0;
    UNUM8 *dataR=res->getRHead(),*dataG=res->getGHead(),*dataB=res->getBHead(),*dataA=res->getAHead();
    for (unsigned int x = halfSize; x < halfSize + width; x++)
        for (unsigned int y = halfSize; y < halfSize + height; y++)
        {
            srcY=*(src->getR(x,y));
            for (wp=0,gaui=0,resY=0,i = -halfSize; i <= halfSize; i++)
                for (int j = -halfSize; j <= halfSize; j++)
                {
                    gauInt=Gaussian(abs(srcY-(*(src->getR(x+i,y+j))))/255.0,intenPara);
                    factor=gauInt*gauSpace[gaui++];
                    resY+=factor*(*(src->getR(x+i,y+j)));
                    wp+=factor;
                }
            *(dataR+ (y - halfSize)*width + x - halfSize) = (UNUM8)ClipToUNUM8(resY/wp);
            *(dataG + (y - halfSize)*width + x - halfSize) =*(src->getG(x,y));
            *(dataB + (y - halfSize)*width + x - halfSize) =*(src->getB(x,y));
            *(dataA + (y - halfSize)*width + x - halfSize) =*(src->getA(x,y));
        }
    delete gauSpace;
    //delete src;
    res->ConvertFormat(format);
    Load(*res);
    return res;
}


//综合图像处理特效
//怀旧特效
int Pixmap::Nostalgic(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b,cr, cg, cb;
    int ar,ag,ab;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;

        ar = static_cast<int>(0.39*cr +0.77*cg+0.19*cb);
        ag = static_cast<int>(0.35*cr + 0.69*cg +0.17*cb);
        ab = static_cast<int>(0.27*cr +0.53*cg +0.13*cb);

        *pr++ = ar>255?255:ar;
        *pg++ = ag>255?255:ag;
        *pb++ = ab>255?255:ab;

    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

//流年特效
int Pixmap::Fleeting(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pb = b, cb;
    int ab;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cb=*pb;
        ab=sqrt(cb)*12;
        *pb++ = ab>255?255:ab;
    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

//光照特效(稍微复杂一点点)
int Pixmap::Lighting(int centerX,int centerY,int radius,int strength){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }

    //设置光照中心和光照半径，这里先默认是图片中心
    //int centerX = height / 2 ,
            //centerY = width / 2 ,
            //光照范围由半径调节
            //radius = min(centerX, centerY)/10;

    //设置光照强度
    //int strength = 200;
    //各种参数
    int distance,result,cr,cg,cb;
    UNUM8 *pr = r, *pg = g, *pb = b;

    for(int x=0;x<height;x++)
        for(int y=0;y<width;y++)
        {
            //计算当前点到光照中心距离(平面坐标系中两点之间的距离)
            distance = pow((centerY-y), 2) + pow((centerX-x), 2);

            result=0;
            if (distance < radius * radius){
                //按照距离大小计算增强的光照值
                result = static_cast<unsigned int>(strength*( 1.0 - sqrt(distance) / radius ));
            }
            cr=*pr+result;
            cg=*pg+result;
            cb=*pb+result;
            *pr++ = cr>255?255:cr;
            *pg++ = cg>255?255:cg;
            *pb++ = cb>255?255:cb;

        }

    if (yuvFlag)
        ConvertToYUV();
    return 0;

}

//积木特效
int Pixmap::BuildingBlock(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    int cr, cg, cb,avg;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;
        avg=(cr+cg+cb)/3;
        *pr++ = avg>127?255:0;
        *pg++ = avg>127?255:0;
        *pb++ = avg>127?255:0;

    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}


//连环画特效
int Pixmap::Comic(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
     int cr, cg, cb,tr,tg,tb;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;
        tb =int(abs(2*cb-cg+cr)*cr) >> 8;
        tg =int(abs(2*cb-cg+cr)*cg) >> 8;
        tr =int(abs(2*cg-cb+cr)*cr) >> 8;
        int gray = (tb + tg + tr)/3;
        gray=gray>245?245:gray;


        *pr++ = gray+10;
        *pg++ = gray+10;
        *pb++ = gray;

    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

//暗调滤镜
int Pixmap::LowLight(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    int cr, cg, cb;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;

        *pr++ = cr*cr/255;
        *pg++ = cg*cg/255;
        *pb++ = cb*cb/255;

    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

//熔铸特效
int Pixmap::Casting(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    int cr, cg, cb,ar,ag,ab;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;
        ar=cr*128/(cg+cb+1);
        ag=cg*128/(cr+cb+1);
        ab=cb*128/(cr+cg+1);
        *pr++ = (ar>255 ? 255 : (ar<0? -ar : ar));
        *pg++ = (ag>255 ? 255 : (ag<0? -ag : ag));
        *pb++ = (ab>255 ? 255 : (ab<0? -ab : ab));


    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

//霓虹特效
int Pixmap::NeonLight(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    //涉及综合处理的地方需要小心咯
    int cr, cg, cb,ar,ag,ab;
    int r1,g1,b1,r2,g2,b2;
    for (unsigned int i = 0; i < height; i++)
        for(unsigned int j = 0;j < width; j++)
        {
            if(i==height-1||j==width-1)
            {
                pr++;
                pg++;
                pb++;
                continue;
            }
            //当前点的rgb
            cr=*pr;
            cg=*pg;
            cb=*pb;
            //同行相邻点的rgb
            r1=*(pr+1);
            g1=*(pg+1);
            b1=*(pb+1);
            //同列相邻点的rgb
            r2=*(pr+width);
            g2=*(pg+width);
            b2=*(pb+width);

            ar=2*sqrt(pow(cr-r1,2)+pow(cr-r2,2));
            ag=2*sqrt(pow(cg-g1,2)+pow(cg-g2,2));
            ab=2*sqrt(pow(cb-b1,2)+pow(cb-b2,2));

            *pr++ = ar>255?255:ar;
            *pg++ = ag>255?255:ag;
            *pb++ = ab>255?255:ab;


        }

    if (yuvFlag)
        ConvertToYUV();
    return 0;
}

//曝光特效
int Pixmap::Exposal(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    int cr, cg, cb;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;

        *pr++ = cr<128?(255-cr):cr;
        *pg++ = cg<128?(255-cg):cg;
        *pb++ = cg<128?(255-cg):cg;
    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;

}

//冰冻特效
//为什么效果会有问题，实现确认是没有问题的
int Pixmap::Freezing(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    int cr, cg, cb,ar,ag,ab;
    for (unsigned int i = 0; i < width*height; i++)
    {
        cr=*pr;
        cg=*pg;
        cb=*pb;

        ab = (cb-cg-cr)*3/2;
        ag = (cg-cb-cr)*3/2;
        ar = (cr-cg-cb)*3/2;

        *pr++ = (ar>255 ? 255 : (ar<0? -ar : ar));
        *pg++ = (ag>255 ? 255 : (ag<0? -ag : ag));
        *pb++ = (ab>255 ? 255 : (ab<0? -ab : ab));


    }
    if (yuvFlag)
        ConvertToYUV();
    return 0;

}


//油画特效
int Pixmap::OilPaint(){
    if (format == FMT_NULL)
        return 1;
    int yuvFlag = 0;
    if (format == FMT_YUV)
    {
        yuvFlag = 1;
        ConvertToRGB();
    }
    UNUM8 *pr = r, *pg = g, *pb = b;
    //int cr, cg, cb,ar,ag,ab;
    for(int x=1;x<height-2;x++)
        for(int y=0;y<width;y++){
            if(rand()%3==0){
                *(pr+x*width+y)=*(pr+(x-1)*width+y);
                *(pg+x*width+y)=*(pg+(x-1)*width+y);
                *(pb+x*width+y)=*(pb+(x-1)*width+y);

            }else if(rand()%2==0){
                *(pr+x*width+y)=*(pr+(x+1)*width+y);
                *(pg+x*width+y)=*(pg+(x+1)*width+y);
                *(pb+x*width+y)=*(pb+(x+1)*width+y);

            }else {
                *(pr+x*width+y)=*(pr+(x+2)*width+y);
                *(pg+x*width+y)=*(pg+(x+2)*width+y);
                *(pb+x*width+y)=*(pb+(x+2)*width+y);
            }
        }
    if (yuvFlag)
        ConvertToYUV();
    return 0;
}


//超分辨率部分
int Pixmap::FSRCNN(int x,string aimPic){

    //Create the module's object
    DnnSuperResImpl sr;

    //Set the image you would like to upscale
    string img_path = "D:\\tmp.jpg";
    Mat img = cv::imread(img_path);

    string path=QCoreApplication::applicationDirPath().toStdString();
    switch (x) {
    case 2: path += "/model/FSRCNN/FSRCNN_x2.pb";
            break;
    case 3: path += "/model/FSRCNN/FSRCNN_x3.pb"; break;
    case 4: path += "/model/FSRCNN/FSRCNN_x4.pb"; break;
    }
//    QString qstr2 = QString::fromStdString(path);
//    qDebug()<<qstr2;
    sr.readModel(path);//Read the desired model

    //Set the desired model and scale to get correct pre- and post-processing
    sr.setModel("fsrcnn", x);


    //Upscale
    Mat img_new;
    sr.upsample(img, img_new);
    cv::imwrite( aimPic, img_new);
    //cv::imwrite( "D:\\tmp_upscaled.jpg", img_new);
//    img_path="tmp_upscaled.jpg";
//    QImage tmpImage(QString::fromStdString(img_path));
//    tmpImage.convertToFormat(QImage::Format_ARGB32);
//    //FreePixmap();虽然不free会导致内存泄露，但是导致更严重的事情，只能说是不幸
//    this->name=PIXMAP::FMT_RGB;
//    if(!tmpImage.isNull())
//    {
//        Load(tmpImage);
//       BaseShape::setPosX(width);
//        BaseShape::setPosY(height);
//    }

}


int Pixmap::EDSR(int x,string aimPic){
    DnnSuperResImpl sr;

    string img_path = "D:\\tmp.jpg";
    Mat img = cv::imread(img_path);

    string path=QCoreApplication::applicationDirPath().toStdString();

    switch (x) {
    case 2: path += "/model/EDSR/EDSR_x2.pb"; break;
    case 3: path += "/model/EDSR/EDSR_x3.pb"; break;
    case 4: path += "/model/EDSR/EDSR_x4.pb"; break;
    }
    sr.readModel(path);//Read the desired model

    sr.setModel("edsr", x);

    Mat img_new;
    sr.upsample(img, img_new);
    //cv::imwrite( "D:\\tmp_upscaled.jpg", img_new);
    cv::imwrite( aimPic, img_new);
}

int Pixmap::ESPCN(int x,string aimPic){
    DnnSuperResImpl sr;

    string img_path = "D:\\tmp.jpg";
    Mat img = cv::imread(img_path);

    string path=QCoreApplication::applicationDirPath().toStdString();
    switch (x) {
    case 2: path += "/model/ESPCN/ESPCN_x2.pb"; break;
    case 3: path += "/model/ESPCN/ESPCN_x3.pb"; break;
    case 4: path += "/model/ESPCN/ESPCN_x4.pb"; break;
    }
    sr.readModel(path);//Read the desired model

    sr.setModel("espcn", x);

    Mat img_new;
    sr.upsample(img, img_new);
    //cv::imwrite( "D:\\tmp_upscaled.jpg", img_new);
    cv::imwrite( aimPic, img_new);
}

int Pixmap::LapSRN(int x,string aimPic){
    DnnSuperResImpl sr;

    string img_path = "D:\\tmp.jpg";
    Mat img = cv::imread(img_path);

    string path=QCoreApplication::applicationDirPath().toStdString();
    switch (x) {
    case 2: path +=  "/model/LapSRN/LapSRN_x2.pb"; break;
    case 4: path += "/model/LapSRN/LapSRN_x4.pb"; break;
    case 8: path += "/model/LapSRN/LapSRN_x8.pb"; break;
    }
    sr.readModel(path);//Read the desired model

    sr.setModel("lapsrn", x);

    Mat img_new;
    sr.upsample(img, img_new);
    //cv::imwrite( "D:\\tmp_upscaled.jpg", img_new);
    cv::imwrite( aimPic, img_new);
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
    unsigned int total = pixmap.GetHeight()*pixmap.GetWidth();
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

