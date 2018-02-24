#include "imagecreator.h"
#include "wiringPi.h"
#include "wiringSerial.h"
#include <QDebug>
#include <QThread>

ImageCreator::ImageCreator(QObject *parent) : QObject(parent)
{
    wiringPiSetup();
    createLoop=0;
    pinMode(8,OUTPUT);
    digitalWrite(8,0);
}

void ImageCreator::doWork()
{

    QSerialPort serial;
    QSerialPortInfo info("ttyAMA0");
    serial.setPort(info);
    if(serial.open(QIODevice::ReadWrite)){
        serial.setBaudRate(QSerialPort::Baud115200);
        serial.setDataBits(QSerialPort::Data8);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setParity(QSerialPort::NoParity);
    } else {
        serial.close();
        qWarning("Did no worko");
        emit errorQuit();
    }

    //while loop to check for new bytes goes here
    int mapIt = 0;
    char led[2];
    char mean[2];
    char std[2];
    bool reading = 0;
    uint8_t ledOuts[16];
    QVector<QRgb> vector(16);
    digitalWrite(8,1);
    while(createLoop)
    {
        while(true&&createLoop){
            QThread::usleep(100);
            //qDebug() << "We're attempting to read";
            if(serial.bytesAvailable()){
                serial.readLine(led, 2);
                reading=1;
                break;
            }
        }
        while(true&&createLoop){
            QThread::usleep(100);
            qDebug() << "We're attempting to read2";
            if(serial.bytesAvailable()){
                serial.readLine(mean, 2);
                break;
            }
        }
        while(true&&createLoop){
            QThread::usleep(100);
            if(serial.bytesAvailable()){
                serial.readLine(std, 2);
                reading=1;
                break;
            }
        }
        if(!createLoop){
            digitalWrite(8,0);
            break;
        }
        if(reading){
            ledOuts[(uint8_t)led[0]] = (uint8_t)mean[0];
            reading = 0;
        }
        if(mapIt==16){
            //get weighted averages and output the relative coloured pixel?
            //Right now we'll just update with a single colour
            for(int i=0; i<16; i++){
                vector[i] = qRgb(ledOuts[i],0,0);
            }
            emit sendImg(vector);
            mapIt=0;
        }
        mapIt++;
    }
    serial.close();
}

bool* ImageCreator::serveLoopEnder()
{
    bool* loopEnder = &createLoop;
    return loopEnder;
}

