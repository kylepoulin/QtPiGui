#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    QObject::connect(workerThread, SIGNAL(finished()), imgCreator, SIGNAL(QObjet::deleteLater()));
    QObject::connect(this, SIGNAL(launchNucleo()), imgCreator, SLOT(doWork()));
    QObject::connect(imgCreator, SIGNAL(resultsReady()), this, SLOT(processResults()));
    QObject::connect(imgCreator, SIGNAL(sendImg(QVector<QRgb>&)), this, SLOT(imageShow(QVector<QRgb>&)));
    QObject::connect(imgCreator, SIGNAL(errorQuit()), this, SLOT(errorSerialQuit()));

    createLoop = imgCreator->serveLoopEnder();
    *createLoop = 0;
}

MainWindow::~MainWindow()
{
    if(workerThread->isRunning()){
        *createLoop = 0;
        workerThread->quit();
        workerThread->wait(100);
    }
    delete workerThread;
    delete imgCreator;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(*createLoop==0){
        ui->pushButton->setText("Stop");
        workerThread->start();
        *createLoop = 1;
        emit launchNucleo();

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
        *createLoop = 0;
    }
}

void MainWindow::imageShow(QVector<QRgb>& pixels)
{
    QImage image(8,2,QImage::Format_RGB32);
    for(int i=0; i<8; i++){
        image.setPixel(i,0,pixels[i]);
        image.setPixel(i+8,1,pixels[i+8]);
    }
    const int w = ui->label1->width();
    const int h = ui->label1->height();
    ui->label1->setPixmap(QPixmap::fromImage(image).scaled(w,h,Qt::IgnoreAspectRatio));
    ui->label1->show();

}
void MainWindow::errorSerialQuit()
{
    close();
}

void MainWindow::processResults()
{

}
