#include "DataStructure.h"
#include <cstdlib>
#include <QDebug>
using namespace PIXMAP;
#ifndef PI
#define PI 3.141592
#endif

//设定位图宽高构造
Pixmap::Pixmap(unsigned int width, unsigned int height, unsigned char value=0) :format(PIXMAP::FMT_NULL), r(NULL), g(NULL), b(NULL), a(NULL),width(width),height(height)
{

    r = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    g = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    b = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    a = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
    memset(r, value, width*height);
    memset(g, value, width*height);
    memset(b, value, width*height);
    memset(a, 255, width*height);
    format = PIXMAP::FMT_RGB;
}

Pixmap::Pixmap(QString fileName):height(0),width(0),format(PIXMAP::FMT_NULL), r(NULL), g(NULL), b(NULL), a(NULL)

{
    QImage tmpImage(fileName);
    tmpImage.convertToFormat(QImage::Format_ARGB32);
    if(!tmpImage.isNull())
    {
        Load(tmpImage);
    }

}

Pixmap::Pixmap(QString fileName,QString fileType):height(0),width(0),format(PIXMAP::FMT_NULL), r(NULL), g(NULL), b(NULL), a(NULL)
{
    if(fileType=="BMP"){

        FILE * fp;
        QByteArray ba=fileName.toLatin1();
        char *file=ba.data();
        fp=fopen(file,"rb");
        //fread(&bf,sizeof(BITMAPFILEHEADER),1,fp);由于结构体填充的问题，会导致结构体为16字节，直接写死就可以了
        fread(&bf,14,1,fp);
        //qDebug()<<sizeof (WORD)<<" "<<sizeof(DWORD)<<" "<<sizeof(BITMAPFILEHEADER);
        if(bf.bfType!=0x4d42)
        {
            fclose(fp);
            return ;
        }

        //fread(&bi,sizeof(BITMAPINFOHEADER),1,fp);
        fread(&bi,40,1,fp);
        LONG bmpWidth = bi.biWidth;
        LONG bmpHeight = bi.biHeight;
        width=bmpWidth;
        height=bmpHeight;
        isOpen=true;
        isReverse=true;
        format=PIXMAP::FMT_RGB;

        r = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        g = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        b = (UNUM8*)malloc(sizeof(UNUM8)*width*height);
        a = (UNUM8*)malloc(sizeof(UNUM8)*width*height);

        UNUM8 *pr=r,*pg=g,*pb=b,*pa=a;

        if(bi.biBitCount==24){
            //真彩色，直接可以读颜色咯
            int totalSize = (bmpWidth *bi.biBitCount/8+3)/4*4*bmpHeight;
            BYTE *pBmpBuf = new BYTE[totalSize];
            size_t size = 0;
            while(true)
            {
                int iret = fread(&pBmpBuf[size],1,1,fp);
                imgData.push_back(pBmpBuf[size]);
                if(iret == 0)
                    break;
                size = size + iret;
            }
            fclose(fp);
            //开始存到Pixmap里面
            unsigned int pitch=width%4;
            for(unsigned int i=0;i<height;i++)
            {
                int realPitch=i*pitch;
                for(unsigned j=0;j<width;j++)
                {
                    *pr++=pBmpBuf[(i*width+j)*3+2+realPitch];
                    *pg++=pBmpBuf[(i*width+j)*3+1+realPitch];
                    *pb++=pBmpBuf[(i*width+j)*3+realPitch];
                    *pa++=255;
                }
            }
            delete [] pBmpBuf;
            pBmpBuf = NULL;

            return ;
    }else if(bi.biBitCount==8){
            //8位 256 的图，可能是彩色，也可能是灰度
            //注意读入调色板应该有256个元素
            fread(&colorMap, sizeof(RGBQUAD), 256, fp);
            int channels = 1;
            int offset = 0;
            int linelength = bmpWidth * channels;
            offset = linelength % 4;
            if (offset > 0)
            {
                offset = 4 - offset;
            }
            UNUM8 pixVal;
            for (int i = 0; i < bmpHeight; i++)
            {
                for (int j = 0; j < linelength; j++)
                {
                    fread(&pixVal, sizeof(UNUM8), 1, fp);
                    *pr++=colorMap[pixVal].rgbRed;
                    *pg++=colorMap[pixVal].rgbGreen;
                    *pb++=colorMap[pixVal].rgbBlue;
                    *pa++=255;
                    imgData.push_back(pixVal);
                }
                for (int k = 0; k < offset; k++)
                {
                    fread(&pixVal, sizeof(UNUM8), 1, fp);
                }
            }
            fclose(fp);
            return ;

        }else {
            return;
        }
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
        isOpen=true;
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

        if(isReverse){
            QTransform matrix;
            matrix.rotate(180);
            *image=image->transformed(matrix);
            *image = image->mirrored(true, false);
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
    case FMT_HSI:return ConvertToHSI();break;
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
    case FMT_HSI:ConvertToRGB();
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

int Pixmap::ConvertToHSI(){
    switch(format){
    case FMT_NULL:
    case FMT_BIN:
    case FMT_GREY:return 1;
    case FMT_HSI:return 0;
    case FMT_YUV:ConvertToRGB();
    case FMT_RGB:
        UNUM8 *rp = r, *gp = g, *bp = b;
        double cr, cg, cb;
        for (unsigned int i = 0; i < width*height; i++)
        {
            cr = (double)(*rp)/255.0;
            cb = (double)(*bp)/255.0;
            cg = (double)(*gp)/255.0;
            double minVal = qMin(qMin(cr, cg), cb);
            double   I = (cr + cg + cb)/3.0;
            double   S = 0.0;
            double   H = 0.0;
            double  diff  = 0.5*(cr-cg + cr-cb);
            double  diff2 = (cr-cg)*(cr-cg) + (cr-cb)*(cg-cb);
            double  sita  = acos(diff/(sqrt(diff2)+1e-5));
            H = (cg>=cb) ? sita : 2.0*PI - sita;
            S = 1.0 - minVal/I;
            H = H/(2.0*PI);
            *rp++ = (UNUM8)ClipToUNUM8(I*255.0);
            *gp++ = (UNUM8)ClipToUNUM8(S*255.0);
            *bp++ = (UNUM8)ClipToUNUM8(H*255.0);
        }
        format = FMT_HSI;
    }
    return 0;
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
    {
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
        format = FMT_RGB;return 0;
    }
    case FMT_HSI:
    {
        UNUM8 *ip = r, *sp = g, *hp = b;
        double H, S, I;
        double t1,t2,t3;
        double R, G, B;
        for (unsigned int i = 0; i < width*height; i++)
        {
            I = (double)(*ip)/255.0;
            S = (double)(*sp)/255.0;
            H = (double)(*hp)/255.0*2.0*PI;
            t1=(1.0-S)/3.0;
            if(H>=0&&H<(PI*2.0/3.0)){
                B=t1;
                t2=S*cos(H);
                t3=cos(PI/3.0-H);
                R=(1.0+t2/t3)/3.0;
                R=3.0*I*R;
                B=3.0*I*B;
                G=3.0*I-(R+B);
            }
            else if(H>=(PI*2.0/3.0)&&H<(PI*4.0/3.0)){
                R=t1;
                t2=S*cos(H-2.0*PI/3.0);
                t3=cos(PI-H);
                G=(1.0+t2/t3)/3.0;
                R=3.0*I*R;
                G=3.0*G*I;
                B=3.0*I-(R+G);
            }
            else if(H>=(PI*4.0/3.0)&&H<=(PI*2.0)){
                G=t1;
                t2=S*cos(H-4.0*PI/3.0);
                t3=cos(PI*5.0/3.0-H);
                B=(1.0+t2/t3)/3.0;
                G=3.0*G*I;
                B=3.0*I*B;
                R=3.0*I-(G+B);
            }
            *ip++=(UNUM8)ClipToUNUM8(R*255.0);
            *hp++=(UNUM8)ClipToUNUM8(B*255.0);
            *sp++=(UNUM8)ClipToUNUM8(G*255.0);
        }
        format = FMT_RGB;return 0;
    }
    case FMT_YCbCr:{
        UNUM8 *yp = r, *up = g, *vp = b;
        double y, cb, cr;
        for (unsigned int i = 0; i < width*height; i++)
        {
            y = *yp-15;
            cb = *up-127;
            cr = *vp-127;
            *yp++ = (UNUM8)ClipToUNUM8(1.164*y + 1.596*cr);
            *up++ = (UNUM8)ClipToUNUM8(1.164*y - 0.392*cb - 0.813*cr);
            *vp++ = (UNUM8)ClipToUNUM8(1.164*y + 2.017*cb);
        }
        format = FMT_RGB;return 0;
    }
    }
    return 0;
}

int Pixmap::ConvertToYCbCr(){
    switch (format)
    {
    case FMT_NULL:
    case FMT_GREY:
    case FMT_BIN:	 return 0;
    case FMT_YCbCr:  return 1;
    case FMT_YUV:
    case FMT_HSI:ConvertToRGB();
    case FMT_RGB:
        UNUM8 cr, cg, cb, *rp = r, *gp = g, *bp = b;
        for (unsigned int i = 0; i < width*height; i++)
        {
            cr = *rp;
            cg = *gp;
            cb = *bp;
            *rp++ = (UNUM8)(0.257*cr + 0.564*cg + 0.098*cb+15);
            *gp++ = (UNUM8)(-0.148*cr-0.291*cg+0.439*cb+127);
            *bp++ = (UNUM8)(0.439*cr-0.368*cg-0.071*cb+127);
        }
        format = FMT_YCbCr;
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
    case FMT_YUV:
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

int Pixmap::ConvertToGreyHSI()
{
    switch (format)
    {
    case FMT_BIN:	format = FMT_GREY; return 0;
    case FMT_GREY:return 0;
    case FMT_NULL:return 1;
    case FMT_YUV:
    case FMT_RGB:
        UNUM8 cr, cg, cb, *pr = r, *pg = g, *pb = b;
        for (unsigned int i = 0; i < width*height; i++)
        {
            cr = *pr;
            cg = *pg;
            cb = *pb;
            *pr++ = *pg++ = *pb++ = (UNUM8)(0.33*cr + 0.33*cg + 0.33*cb);
        }
        format = FMT_GREY;
    }
    return 0;
}

Pixmap* Pixmap::OrderDitherToBin(double * filter, unsigned int filterSize)
{
    if (this->format == FMT_NULL || filter == NULL ||this->format==FMT_BIN)
        return nullptr;
    else if(this->format!=FMT_GREY)ConvertToGrey();



    int MaxVal=filterSize*filterSize-1;    
    Pixmap* res =new Pixmap(width,height);
    res->setReverse(isReverse);
    unsigned char *dataR = res->getRHead();//after
    unsigned char *dataG = res->getGHead();
    unsigned char *dataB = res->getBHead();

    for (unsigned int x = 0; x < width; x+=filterSize)
        for (unsigned int y = 0; y < height; y+=filterSize)
        {
            for (unsigned int i = 0; i < filterSize; i++)
                for (unsigned int j = 0; j < filterSize; j++)
                {
                    if((x+i)<width&&(y+j)<height){
                        *(dataR+y*width+x)=((*(this->getR(x+i, y+j)))/255.0*MaxVal)<(*(filter+j*filterSize+i))?0:255;
                        *(dataG+y*width+x)=*(dataR+y*width+x);
                        *(dataB+y*width+x)=*(dataR+y*width+x);
                    }
                }
        }


    format=FMT_BIN;
    return res;
}
Pixmap* Pixmap::UnOrderedDitherToBin(double * filter, unsigned int filterSize)
{
    if (this->format == FMT_NULL || filter == NULL ||this->format==FMT_BIN)
        return nullptr ;
    else if(this->format!=FMT_GREY) ConvertToGrey();
    int MaxVal=filterSize*filterSize-1;
    int newWidth=width*filterSize;
    int newHeight=height*filterSize;

    Pixmap* res =new Pixmap(newWidth,newHeight);
    res->setReverse(isReverse);
    unsigned char *dataR = res->getRHead();//after
    unsigned char *dataG = res->getGHead();
    unsigned char *dataB = res->getBHead();

    for (unsigned int y = 0; y < height; y++)
        for (unsigned int x = 0; x < width; x++)
        {
            int m=x*filterSize,n=y*filterSize;

            for (unsigned int i = 0; i < filterSize; i++)
                for (unsigned int j = 0; j < filterSize; j++)
                {
                    if(filter[i*filterSize+j]<*(getR(x,y))*MaxVal/255.0){
                       *(dataR+(n+i)*newWidth+m+j)=255;
                       *(dataG+(n+i)*newWidth+m+j)=255;
                       *(dataB+(n+i)*newWidth+m+j)=255;
                    }
                    else {
                       *(dataB+(n+i)*newWidth+m+j)=0;
                       *(dataB+(n+i)*newWidth+m+j)=0;
                       *(dataB+(n+i)*newWidth+m+j)=0;
                    }
                }
        }
    res->format=FMT_BIN;
    return res;
}


int Pixmap::POWTransform(double gamma){
    unsigned int oldFormat=format;
    switch(format){
    case FMT_NULL:
    case FMT_BIN:return 0;
    case FMT_GREY:
    case FMT_HSI:
    case FMT_RGB:ConvertToYUV();
    case FMT_YUV:{
        for(unsigned int i=0;i<height;i++){
            for(unsigned int j=0;j<width;j++){
                double data=*(r+i*width+j)/255.0;
                data=pow(data,gamma);
                *(r+i*width+j)=ceil(data*255.0);
            }
        }
    }
    }
    ConvertFormat(oldFormat);
    return 1;
}
int Pixmap::LOGTransform(double param){
    unsigned int oldFormat=format;
    switch(format){
    case FMT_NULL:
    case FMT_BIN:return 0;
    case FMT_GREY:
    case FMT_HSI:
    case FMT_RGB:ConvertToYUV();
    case FMT_YUV:{
        for(unsigned int i=0;i<height;i++){
            for(unsigned int j=0;j<width;j++){
                double data=*(r+i*width+j)/255.0;
                data=log(1 + param * data) / log(param + 1);
                *(r+i*width+j)=ceil(data*255.0);
            }
        }
    }
    }
    ConvertFormat(oldFormat);
    return 1;
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
    case FMT_HSI:
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
    int hsiFlag = 0;
    if(format==FMT_HSI){
        hsiFlag=1;
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
    if(hsiFlag)
        ConvertToHSI();
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

bool Pixmap::getPicOPen(){
    return isOpen;
}

bool Pixmap::getReverse(){
    return isReverse;
}

bool Pixmap::setReverse(bool _isReverse){
    isReverse=_isReverse;
    return _isReverse;
}

double Pixmap::Gaussian(double x,double r)
{
    const double tmp=pow(2*PI,0.5);
    return pow(2.7182818,-x*x/(2*r*r))/(r*tmp);
}


bool Pixmap::SaveImage(const char* path)
{
    FILE* pFile;
    pFile = fopen(path, "wb");
    if (!pFile)
    {
        return 0;
    }
    // Processing
    fwrite(&bf, 14, 1, pFile);
    fwrite(&bi, 40, 1, pFile);
    // Get Channel num of a pixel
    int channels = 0;
    if (bi.biBitCount == 8)
    {
        channels = 1;
        fwrite(&colorMap, sizeof(RGBQUAD), 256, pFile);
    }
    else if (bi.biBitCount == 24)
    {
        channels = 3;
    }
    // Get offset of every scanline,length(scanline)=length(pixel)+offset
    int offset = 0;
    int linelength = bi.biWidth * channels;
    offset = (channels * bi.biWidth) % 4;
    if (offset > 0)
    {
        offset = 4 - offset;
    }
    // Write Pixel
    uint8_t pixVal;
    auto iter = imgData.begin();
    for (int i = 0; i < bi.biHeight; i++)
    {
        for (int j = 0; j < linelength; j++)
        {
            pixVal = *iter;
            fwrite(&pixVal, sizeof(uint8_t), 1, pFile);
            iter += 1;
        }
        pixVal = 0;
        for (int k = 0; k < offset; k++)
        {
            fwrite(&pixVal, sizeof(uint8_t), 1, pFile);
        }
    }
    fclose(pFile);
    return true;
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

