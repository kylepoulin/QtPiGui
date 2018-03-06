#include "imagecreator.h"
#include "wiringPi.h"
#include "wiringSerial.h"
#include <QDebug>
#include <QThread>
#include <math.h>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>


ImageCreator::ImageCreator(QObject *parent) : QObject(parent)
{
    wiringPiSetup();
    createLoop=0;
    pinMode(8,OUTPUT);
    digitalWrite(8,0);
    std::ifstream source;
    source.open(":/ledScales.txt", std::fstream::in);
    if(!source){
        qWarning("No data file opened");
        emit errorQuit();
    } else {
        for(int i=0;i<16;i++){
            source >> ledScales[i];
        }
    }
    source.close();
}

void ImageCreator::calibrate()
{
    int fd = serialOpen("/dev/ttyAMA0", 19200);
    if(fd == -1){
        //serialClose(fd);
        qWarning("No worko, error: %s\n", strerror(errno));
        emit errorQuit();
    }
    //while loop to check for new bytes goes here
        int mapIt = 0;
        char led[2];
        char mean[2];
        char std[2];
        bool reading = 0;
        int calibrationSize = 10000;
        uint8_t ledOuts[16];
        uint8_t ledArray[16][calibrationSize];
        QVector<QRgb> vector(16);
        digitalWrite(8,1);
        QThread::usleep(500);
        digitalWrite(8,0);
        serialFlush(fd);

        for(int j=0; j<calibrationSize; j++)
        {
            for(int i=0; i<16; i++){
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
                    break;
                }
                if(reading){
                    if((uint8_t)i == (uint8_t)led[0]){
                        ledOuts[i] = (uint8_t)mean[0];
                        ledArray[i][j] = (uint8_t)mean[0];
                        reading = 0;
                    } else {
                        qWarning("Mismatched %d led reading with %d loop index\n",i,led[0]);
                        emit errorQuit();
                    }
                }
                if(mapIt==16){
                    //output pixels and the calibration completion
                    for(int i=0; i<16; i++){
                        vector[i] = qRgb(ledOuts[i],0,0);
                    }
                    emit sendImgCalibrate(vector);

                    //emit the percentage finished
                    //emit calibrationCompletionPercentage(j/100000);
                    mapIt=0;
                }
                mapIt++;
            }
        }
        //Set LED Scales: Get Mean of all LEDs over this time, then scale appropriately using floats?
        if(createLoop){
            for(int i=0;i<16;i++){
                for(int j=0;j<calibrationSize;j++){
                    ledOuts[i] += ledArray[i][j];
                }
                ledScales[i] = ledOuts[i] / calibrationSize;
            }
            //set Led values in the calibration resource file
            std::ofstream source(":/ledScales.txt");
            if(source.is_open()){
                source << ledScales[0] << " " << ledScales[1] << " " << ledScales[2] << " " << ledScales[3] << " " <<
                          ledScales[4] << " " << ledScales[5] << " " << ledScales[6] << " " << ledScales[7] << " " <<
                          ledScales[8] << " " << ledScales[9] << " " << ledScales[10] << " " << ledScales[11] << " " <<
                          ledScales[12] << " " << ledScales[13] << " " << ledScales[14] << " " << ledScales[15];
            } else {
                qWarning("Could not write to file");
                emit errorQuit();
            }
            //emit success to the pi
            emit calibrationSuccess();
            createLoop=0;
        }
        serialClose(fd);
        serialFlush(fd);
        digitalWrite(8,1);
        QThread::usleep(500);
        digitalWrite(8,0);
}

void ImageCreator::doWork()
{

    int fd = serialOpen("/dev/ttyAMA0", 19200);
    if(fd == -1){
        //serialClose(fd);
        qWarning("No worko, error: %s\n", strerror(errno));
        emit errorQuit();
    }
    //set up the averaging array
    QVector<QVector<uint8_t>> slopeVectors;

    //while loop to check for new bytes goes here
        int mapIt = 1;
        char led[2];
        char mean[2];
        char std[2];
        bool reading = 0;
        digitalWrite(8,1);
        QThread::usleep(500);
        digitalWrite(8,0);
        serialFlush(fd);
        int slopeMeanArraySize = 300;
        //Case statement for mode select
        qWarning("Made it to While loop");
        while(createLoop)
        {
            while(true&&createLoop){
                QThread::usleep(100);
                if(serialDataAvail(fd)){
                    //Read LED number, aka decide which vector to put into it
                    led[0]= (uint8_t)serialGetchar(fd);
                    qWarning("LED reading: %d", led[0]);
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
                break;
            }
            if(reading){
                if(slopeVectors[15].size()>=slopeMeanArraySize){
                    slopeVectors[(uint8_t)led[0]].append((uint8_t)mean[0]);
                    slopeVectors[(uint8_t)led[0]].removeFirst();
                } else {
                    slopeVectors[(uint8_t)led[0]].append((uint8_t)mean[0]);
                }
            }
            if(mapIt==16){
                QVector<uint8_t> means(16);
                if(slopeVectors[15].size()>=slopeMeanArraySize){
                    for(int i=0; i<16; i++){
                        double earlySlope=0;
                        double laterSlope=0;
                        for(int j=0; j<5; j++){
                            earlySlope += slopeVectors[i][j];
                            laterSlope += slopeVectors[i][slopeVectors[i].size()-1-j];


                        }
                        earlySlope = earlySlope*10/slopeMeanArraySize;
                        laterSlope = laterSlope*10/slopeMeanArraySize;
                        means[i] = floor((laterSlope-earlySlope)/slopeVectors[i].size());
                    }
                }
                emit sendImg(means);
                mapIt=0;
            }
            mapIt++;
        }

        serialClose(fd);
        digitalWrite(8,1);
        QThread::usleep(500);
        digitalWrite(8,0);
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
