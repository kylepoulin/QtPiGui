#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label1->adjustSize();
    ui->pushButton->adjustSize();
    //setCentralWidget(ui->label1);

    workerThread = new QThread();
    imgCreator = new ImageCreator();
    imgCreator->moveToThread(workerThread);
    workerThread->start();

    oldImage = new QImage(3750,255, QImage::Format_RGB32);
    oldImage->fill(qRgb(255,255,255));
    oldImagePixelsLeft = new QVector<uint8_t>(3750);
    oldImagePixelsLeft->fill(113);
    oldImagePixelsRight = new QVector<uint8_t>(3750);
    oldImagePixelsRight->fill(111);
    oldImagePixelsTotal = new QVector<uint8_t>(3750);
    oldImagePixelsTotal->fill(112);

    qRegisterMetaType<QVector<QRgb>>("QVector<QRgb>");

    QObject::connect(workerThread, SIGNAL(finished()), imgCreator, SIGNAL(QObjet::deleteLater()));
    QObject::connect(this, SIGNAL(launchNucleoDisplay()), imgCreator, SLOT(doWork()));
    QObject::connect(this, SIGNAL(launchNucleoCalibrate()), imgCreator, SLOT(calibrate()));
    QObject::connect(imgCreator, SIGNAL(resultsReady()), this, SLOT(processResults()));
    QObject::connect(imgCreator, SIGNAL(sendImgCalibrate(QVector<QRgb>)), this, SLOT(imageShowCalibrate(QVector<QRgb>)));
    QObject::connect(imgCreator, SIGNAL(sendImg(QVector<uint8_t>)), this, SLOT(imageShow(QVector<uint8_t>)));
    QObject::connect(imgCreator, SIGNAL(calibrationCompletionPercentage(float)), this, SLOT(calibrationPercentage(float)));

    QObject::connect(imgCreator, SIGNAL(errorQuit()), this, SLOT(errorSerialQuit()));
    QObject::connect( this, SIGNAL(changeMode(int)), imgCreator, SLOT(changeMode(int)));

    createLoop = imgCreator->serveLoopEnder();
    *createLoop = 0;
    originalBrain = new QImage();
    originalBrain->load(":/brain.bmp");
    mode = 0;
}

MainWindow::~MainWindow()
{
    if(workerThread->isRunning()){
        *createLoop = 0;
        workerThread->quit();
        workerThread->wait(100);
    }
    delete workerThread;
    if(originalBrain!=NULL){
        delete originalBrain;
    }
    delete oldImage;
    delete oldImagePixelsLeft;
    delete oldImagePixelsRight;
    delete oldImagePixelsTotal;
    delete imgCreator;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(mode==1){
        //emit the calibration signal
        //TO-DO: figure out a proper display thing for the time
            //imageCreator should send a percent finished, as well as the pixel values
            //mainwindow should update the pixel colors in the main window, and also show a new window with a loading bar
        if(*createLoop==0){
            *createLoop = 1;
            ui->pushButton->setText("Stop");
            emit launchNucleoCalibrate();
        } else {
            ui->pushButton->setText("Start");
            *createLoop = 0;
        }


    } else {
        if(*createLoop==0){
            ui->pushButton->setText("Stop");
            *createLoop = 1;
            emit launchNucleoDisplay();

            QImage image(4,2, QImage::Format_RGB32);
            for(int i=0; i<4; i++){
                image.setPixel(i,0,qRgb(i*20,i*6,0));
                image.setPixel(i,1,qRgb(i*18,i*4,0));
            }
            const int w = ui->label1->width();
            const int h = ui->label1->height();
            ui->label1->setPixmap(QPixmap::fromImage(image).scaled(w,h,Qt::IgnoreAspectRatio));
            ui->label1->show();

            for(int i=0; i<4; i++){
                image.setPixel(i,0,qRgb(0,i*6,i*20));
                image.setPixel(i,1,qRgb(0,i*4,i*18));
            }
            ui->label1->setPixmap(QPixmap::fromImage(image).scaled(w,h,Qt::IgnoreAspectRatio));
            ui->label1->show();
        } else {
            ui->pushButton->setText("Start");
            QImage image(4,2, QImage::Format_RGB32);
            for(int i=0; i<4; i++){
                image.setPixel(i,0,qRgb(i*20,i*6,0));
                image.setPixel(i,1,qRgb(i*18,i*4,0));
            }
            const int w = ui->label1->width();
            const int h = ui->label1->height();
            ui->label1->setPixmap(QPixmap::fromImage(image).scaled(w,h,Qt::IgnoreAspectRatio));
            ui->label1->show();
            *createLoop = 0;
        }
    }
}

void MainWindow::imageShowCalibrate(QVector<QRgb> pixels)
{
    QImage image(8,2,QImage::Format_RGB32);
    for(int i=0; i<8; i++){
        image.setPixel(i,0,pixels[i]);
        image.setPixel(i,1,pixels[i+8]);
    }
    const int w = ui->label1->width();
    const int h = ui->label1->height();
    ui->label1->setPixmap(QPixmap::fromImage(image).scaled(w,h,Qt::IgnoreAspectRatio));
    ui->label1->show();
}

void MainWindow::calibrationSuccess()
{
    ui->pushButton->setText("Start");
}

void MainWindow::imageShow(QVector<uint8_t> means)
{
    switch(mode){
        case 0:
        {
            //round means for left and right optodes
            //0 4 8 12 are 860 for left? 2 6 10 14 are 860 for right
            //this might be better done in the imgCreator class lol probably going to crash
            uint8_t meanleft=0, meanright=0, meantotal=0;
            meanleft = std::round((means[0] + means[4] +means[8] +means[12])/4);
            meanright = std::round((means[2] + means[6] +means[10] +means[14])/4);
            meantotal = std::round((meanleft+meanright)/2);
            for(int i=0; i<3749; i++){
                oldImage->setPixel(i,oldImagePixelsLeft->at(i),qRgb(255,255,255));
                oldImage->setPixel(i,oldImagePixelsLeft->at(i+1),qRgb(200,0,0));

                oldImage->setPixel(i,oldImagePixelsRight->at(i),qRgb(255,255,255));
                oldImage->setPixel(i,oldImagePixelsRight->at(i+1),qRgb(0,200,0));

                oldImage->setPixel(i,oldImagePixelsTotal->at(i),qRgb(255,255,255));
                oldImage->setPixel(i,oldImagePixelsTotal->at(i+1),qRgb(0,0,200));
            }
            oldImage->setPixel(3749,meanleft,qRgb(200,0,0));
            oldImage->setPixel(3749, meanright, qRgb(0,200,0));
            oldImage->setPixel(3749, meantotal, qRgb(0,0,200));

            oldImagePixelsLeft->removeFirst();
            oldImagePixelsRight->removeFirst();
            oldImagePixelsTotal->removeFirst();

            oldImagePixelsLeft->append(meanleft);
            oldImagePixelsRight->append(meanright);
            oldImagePixelsTotal->append(meantotal);

            int w = ui->label1->width();
            int h = ui->label1->height();
            ui->label1->setPixmap(QPixmap::fromImage(*oldImage).scaled(w,h,Qt::IgnoreAspectRatio));
            ui->label1->show();
            break;
        }
        case 1:
            break;
        case 2:

            break;
    }
}


void MainWindow::calibrationPercentage(float percentage)
{
    //update window with percentage
}

void MainWindow::errorSerialQuit()
{
    close();
}

void MainWindow::processResults()
{

}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if(index==0){
        qWarning("Time Chart");
        emit changeMode(index);
        mode = index;
    } else if (index == 1){
        qWarning("Calibrate");
        emit changeMode(index);
        mode = index;
    } else {
        qWarning("Brain Display");
        emit changeMode(index);
        mode = index;
    }
}
