#ifndef MYMODBUS_H
#define MYMODBUS_H

#include <QObject>
#include <QtSerialBus/QModbusTcpClient>
#include <QtSerialBus/QModbusDataUnit>
#include <QDebug>

#define LOGDEBUG qDebug()<<__FILE__<<__LINE__

class MyModbus : public QObject
{
    Q_OBJECT
public:
    explicit MyModbus(QObject *parent = nullptr);
    ~MyModbus();

    void connectToModbus(QString ip,int port);
    void disconnect();

    bool readModbusData(int typeNum, int startAdd, quint16 numbers);
    bool writeModbusData(int typeNum, int startAdd, int value);
    bool readHoldingRegisters(quint16 startAddress, quint16 quantity, QModbusDataUnit& result);
    bool writeSingleHoldingRegister(quint16 startAddress, quint16 value);
    bool writeMultiHoldingRegisters(quint16 startAddress, const QList<quint16>& values);
    bool getConnectionState();

signals:
    void signal_stateChanged(bool flag);
    void signal_errorOccurred();
    void signal_readCoils(QVector<quint16> vAllData);
    void signal_readRegisters(int resultNum);
    void signal_receivedData(const QVector<quint16>& data);

private slots:
    void slot_stateChanged();
    void slot_readReadyCoils();
    void slot_readReadyRegisters();

private:
    QModbusTcpClient* m_modbusClient;
};
#endif // MYMODBUS_H
