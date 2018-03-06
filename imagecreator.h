#ifndef IMAGECREATOR_H
#define IMAGECREATOR_H

#include <QObject>
#include <QColor>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>

class ImageCreator : public QObject
{
    Q_OBJECT
public:
    explicit ImageCreator(QObject *parent = 0);
    bool* serveLoopEnder();


signals:
    void sendImgCalibrate(QVector<QRgb>);
    void sendImg(QVector<uint8_t>);
    void calibrationCompletionPercentage(float);
    void calibrationSuccess();
    void resultsReady();
    void errorQuit();

public slots:
    void doWork();
    void calibrate();
    void changeMode(int index=0);

private:
    bool createLoop;
    int mode;
    float ledScales[16];
};

#endif // IMAGECREATOR_H
