#include "mainwindow.h"
#include "./ui_mainwindow.h"
 double dither2[]={0,2,3,1};
 double dither4[]={0,  8,  2, 10,
                  12,  4, 14,  6,
                   3, 11,  1, 9,
                   15, 7, 13, 5};
 double dither8[]={0, 32, 8, 40, 2, 34, 10, 42,
                  48, 16, 56, 24, 50, 18, 58, 26,
                  12, 44, 4, 36, 14, 46, 6, 38,
                  60, 28, 52, 20, 62, 30, 54, 22,
                  3, 35, 11, 43, 1, 33, 9, 41,
                  51, 19, 59, 27, 49, 17, 57, 25,
                  15, 47, 7, 39, 13, 45, 5, 37,
                  63, 31, 55, 23, 61, 29, 53, 21};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *){
}

void MainWindow::on_action_openPic_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,"打开图片","C:\\");
    if(filename.isEmpty())
    {
        QMessageBox::warning(this,"未选择文件","请选择图片！");
        return;
     }
    curPixmap=new Pixmap(filename);
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_open_pic_bmp_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,"打开图片","C:\\");
    if(filename.isEmpty())
    {
        QMessageBox::warning(this,"未选择文件","请选择图片！");
        return;
     }
    QString fileType("BMP");
    curPixmap=new Pixmap(filename,fileType);
    if(curPixmap->getPicOPen()){
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
    }else{
        QMessageBox::warning(this,"确定打开的是bmp格式的吗？","打开文件失败！");
        return;
    }
}

void MainWindow::on_action_to_binary_triggered()
{
    if(!QMessageBox::question(this,QStringLiteral("单阈值法"),QStringLiteral("是否手动设置阈值？"),QStringLiteral("手动设置"),
                              QStringLiteral("OTSU")))
    {
        curPixmap->ConvertToBin(QInputDialog::getInt(this,QStringLiteral("图像二值化"),
                                 QStringLiteral("请输入阈值（取值范围0~255）"),150,0,255,1));
    }else {
       curPixmap->ConvertToGrey();
       curPixmap->ConvertToBin(curPixmap->OtsuGetThre());

    }

    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_histoEqualizing_triggered()
{
    curPixmap->HistoEqualizing();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}



void MainWindow::on_action_ordered_dither_triggered()
{
    switch (QInputDialog::getInt(this,QStringLiteral("order dither"),
                                 QStringLiteral("请输入dither矩阵大小（取值范围2或4或8）"),2,2,8,2)) {
    case 2:
        ui->label->setPixmap(QPixmap::fromImage(*((curPixmap->OrderDitherToBin(dither2,2))->Output())));
        break;
    case 4:
        ui->label->setPixmap(QPixmap::fromImage(*((curPixmap->OrderDitherToBin(dither4,4))->Output())));
        break;
    case 8:
        ui->label->setPixmap(QPixmap::fromImage(*((curPixmap->OrderDitherToBin(dither8,8))->Output())));

    }

    update();

}

void MainWindow::on_action_dither_triggered()
{



    switch (QInputDialog::getInt(this,QStringLiteral("dither"),
                                 QStringLiteral("请输入dither矩阵大小（取值范围2或4或8）"),2,2,8,2)) {
    case 2:
        ui->label->setPixmap(QPixmap::fromImage(*((curPixmap->UnOrderedDitherToBin(dither2,2))->Output())));
        break;
    case 4:
        ui->label->setPixmap(QPixmap::fromImage(*((curPixmap->UnOrderedDitherToBin(dither4,4))->Output())));
        break;
    case 8:
        ui->label->setPixmap(QPixmap::fromImage(*((curPixmap->UnOrderedDitherToBin(dither8,8))->Output())));
    }

    update();
}

void MainWindow::on_action_log_triggered()
{
    curPixmap->LOGTransform(QInputDialog::getDouble(this,QStringLiteral("对数变换"),
                                                 QStringLiteral("请输入对数变换参数（取值范围）"),0.2,0,100));
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_power_law_triggered()
{
    curPixmap->POWTransform(QInputDialog::getDouble(this,QStringLiteral("幂次变换"),
                                                    QStringLiteral("请输入幂次变换参数（取值范围）"),2.2,0,100));
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_RGB_to_HSI_triggered()
{
    curPixmap->ConvertToHSI();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}



void MainWindow::on_action_HSI_to_RGB_triggered()
{
    curPixmap->ConvertToRGB();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}


void MainWindow::on_action_RGB_to_YUV_triggered()
{
    curPixmap->ConvertToYUV();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_YUV_to_RGB_triggered()
{
    curPixmap->ConvertToRGB();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}



void MainWindow::on_action_RGB_to_grey_YUV_triggered()
{
    curPixmap->ConvertToGrey();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_RGB_to_Grey_HSI_triggered()
{
    curPixmap->ConvertToGreyHSI();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_quantization_triggered()
{
    QImage image;
    QImage aimage;
    QImage rimage;
    image=(*(curPixmap->Output()));
    uchar* qimage = new uchar[image.width()*image.height()];
    uchar* AfterScaled = new uchar[image.width()*image.height()];
    uchar* Recover = new uchar[image.width()*image.height()];
    qimage = image.bits();
    int scale = 4;
    for(int i = 0;i<image.width();i++){
        for(int j = 0;j< image.height();j++){

            AfterScaled[j*image.width()+i] = qimage[j*image.width()+i]/scale;
            Recover[j*image.width()+i] = AfterScaled[j*image.width()+i] * scale;
        }
    }

    aimage = QImage(AfterScaled,image.width(),image.height(),QImage::Format_Indexed8);
    rimage = QImage(Recover,image.width(),image.height(),QImage::Format_Indexed8);
    QPixmap apix = QPixmap::fromImage(aimage);
    QPixmap rpix = QPixmap::fromImage(rimage);
    ui->label->setPixmap(apix);
    //ui->label->setPixmap(rpix);
    update();


}


void MainWindow::on_action_YCbCr_to_RGB_triggered()
{
    curPixmap->ConvertToRGB();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();

}

void MainWindow::on_action_RGB_to_YCbCr_triggered()
{
    curPixmap->ConvertToYCbCr();
    ui->label->setPixmap(QPixmap::fromImage(*(curPixmap->Output())));
    update();
}

void MainWindow::on_action_bmp_save_triggered()
{

    QFileDialog fileDialog(this);
    QString aimFileName=fileDialog.getSaveFileName(this,QStringLiteral("Save BMP File"),".","BMP (*.bmp)");
    if(!aimFileName.isNull())
    {
        QByteArray ba=aimFileName.toLatin1();
        char *file=ba.data();
        curPixmap->SaveImage(file);
        QMessageBox::information(this,"保存bmp","成功");
    }else{
        QMessageBox::warning(this,"保存bmp","保存图片失败！");
        return;
    }

}
