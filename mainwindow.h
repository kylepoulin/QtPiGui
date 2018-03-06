#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlabel.h>
#include "imagecreator.h"
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void imageShowCalibrate(QVector<QRgb>);
    void imageShow(QVector<uint8_t>);
    void calibrationPercentage(float);
    void calibrationSuccess();

private slots:
    void on_pushButton_clicked();
    void processResults();
    void errorSerialQuit();
    void on_comboBox_currentIndexChanged(int index);

signals:
    void launchNucleoDisplay();
    void launchNucleoCalibrate();
    void changeMode(int);

private:
    Ui::MainWindow *ui;
    ImageCreator* imgCreator;
    QThread* workerThread;
    bool* createLoop;
    QImage* originalBrain;
    int mode;
    QImage* oldImage;
    QVector<uint8_t>* oldImagePixelsLeft;
    QVector<uint8_t>* oldImagePixelsRight;
    QVector<uint8_t>* oldImagePixelsTotal;

};

#endif // MAINWINDOW_H
