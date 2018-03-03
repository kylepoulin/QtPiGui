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
    /*
    QSerialPort serial;
    QSerialPortInfo info("ttyAMA0");
    serial.setPort(info);
    if(serial.open(QIODevice::ReadOnly)){
        serial.setBaudRate(QSerialPort::Baud19200);
        serial.setDataBits(QSerialPort::Data8);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setParity(QSerialPort::NoParity);
    } else {
        serial.close();
        qWarning("Did no worko");
        emit errorQuit();
    }
    */
    int fd = serialOpen("/dev/ttyAMA0", 19200);
    if(fd == -1){
        //serialClose(fd);
        qWarning("No worko, error: %s\n", strerror(errno));
        emit errorQuit();
    }
    //set up the averaging array
    QVector<uint8_t> ledVec860Left();
    QVector<uint8_t> ledVec860Right();
    QVector<uint8_t> ledVec660Left();
    QVector<uint8_t> ledVec660Right();
    QVector<uint8_t> stdVec();


    //while loop to check for new bytes goes here
        int mapIt = 0;
        char led[2];
        char mean[2];
        char std[2];
        bool reading = 0;
        uint8_t ledOuts[16];
        QVector<QRgb> vector(16);
        digitalWrite(8,1);
        QThread::usleep(500);
        digitalWrite(8,0);
        serialFlush(fd);
        while(createLoop)
        {
            while(true&&createLoop){
                QThread::usleep(100);
                if(serialDataAvail(fd)){
                    //Read LED number, aka decide which vector to put into it
                    led[0]= (uint8_t)serialGetchar(fd);
                    //qWarning("LED reading: %d", led[0]);
                    reading=1;
                    break;
                }
            }
            while(true&&createLoop){
                QThread::usleep(100);
               // qDebug() << "We're attempting to read2";
                if(serialDataAvail(fd)){
                    mean[0] = (uint8_t)serialGetchar(fd);
                    //qWarning("LED mean: %d", mean[0]);
                    break;
                }
            }
            while(true&&createLoop){
                QThread::usleep(100);
                if(serialDataAvail(fd)){
                    std[0] = (uint8_t)serialGetchar(fd);
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
        serialClose(fd);
        digitalWrite(8,1);
        QThread::usleep(500);
        digitalWrite(8,0);

    /*
    //while loop to check for new bytes goes here
    int mapIt = 0;
    char led[2];
    char mean[2];
    char std[2];
    bool reading = 0;
    uint8_t ledOuts[16];
    QVector<QRgb> vector(16);
    digitalWrite(8,1);
    QThread::usleep(500);
    digitalWrite(8,0);
    while(createLoop)
    {
        while(true&&createLoop){
            QThread::usleep(100);
            if(serial.bytesAvailable()){
                //Read LED number, aka decide which vector to put into it
                serial.readLine(led, 2);
                qWarning("LED reading: %d", led[0]);
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
    digitalWrite(8,1);
    QThread::usleep(500);
    digitalWrite(8,0);
    */

}

bool* ImageCreator::serveLoopEnder()
{
    bool* loopEnder = &createLoop;
    return loopEnder;
}

void ImageCreator::changeMode(int index)
{
    mode = index;
}
