#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "DataStructure.h"
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
//绘图事件
    void paintEvent(QPaintEvent*);//重写窗体重绘事件

private slots:
    void on_action_openPic_triggered();

    void on_action_to_binary_triggered();

    void on_action_histoEqualizing_triggered();

    void on_action_ordered_dither_triggered();

    void on_action_dither_triggered();

    void on_action_log_triggered();

    void on_action_RGB_to_HSI_triggered();

    void on_action_HSI_to_RGB_triggered();

    void on_action_power_law_triggered();

    void on_action_RGB_to_grey_YUV_triggered();

    void on_action_RGB_to_Grey_HSI_triggered();

    void on_action_quantization_triggered();

    void on_action_RGB_to_YUV_triggered();

    void on_action_YUV_to_RGB_triggered();



private:
    Ui::MainWindow *ui;
    Pixmap* curPixmap;

};
#endif // MAINWINDOW_H
