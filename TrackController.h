#ifndef TRACKCONTROLLER_H
#define TRACKCONTROLLER_H

#include <QObject>

#include "ModbusCom.h"
#include "GlobalVariable.h"

class TrackController : public QObject
{
    Q_OBJECT
public:
    TrackController();

    void connectToPLC(const QString& ip, int port);
    void disconnectWithPLC();
    void reset(bool flag);
    void runToOrigin();
    void runForward();
    void runBackward();
    void moveToTarget(int pos);
    void stopMoving();
    void setMovingSpeed(int speed);
    void requestFeedback(int flag);

    void setRunToOrigin(bool flag);
    void setRunForward(bool flag);
    void setRunBackward(bool flag);
    void setPosition(int pos);
    void setMove(bool flag);
    bool getArrivedSignal();

    bool isConnected();

signals:
    void sig_stateChanged(int flag);
    void sig_updateData(const QVector<quint16>& data);

private slots:
    void slot_stateChanged(bool flag);
    void slot_receivedData(const QVector<quint16>& data);

private:
    void analyseFeedback(const QVector<quint16>& data);

private:
    MyModbus* _modbus;
    QVector<quint16> m_data;
    bool m_isConnected;
};

#endif // TRACKCONTROLLER_H
