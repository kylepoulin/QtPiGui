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
    void sendImg(QVector<QRgb>&);
    void resultsReady();
    void errorQuit();

public slots:
    void doWork();

private:
    bool createLoop;
};

#endif // IMAGECREATOR_H
