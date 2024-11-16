#ifndef MYMODBUS_H
#define MYMODBUS_H

#include <QObject>
#include <QtSerialBus/QModbusTcpClient>
#include <QtSerialBus/QModbusDataUnit>
#include <QDebug>
#include <QEventLoop>

#include <string>

#define LOGDEBUG qDebug()<<__FILE__<<__LINE__

class MyModbus : public QObject
{
    Q_OBJECT
public:
    explicit MyModbus(const std::string& deviceName, QObject *parent = nullptr);
    ~MyModbus();

    void connectToModbus(QString ip,int port);
    void disconnect();

    bool readModbusData(int typeNum, int startAdd, quint16 numbers);
    bool writeModbusData(int typeNum, int startAdd, int value);
    bool readHoldingRegisters(quint16 startAddress, quint16 quantity, QModbusDataUnit& result);
    bool writeSingleHoldingRegister(quint16 startAddress, quint16 value);
    bool writeMultiHoldingRegisters(quint16 startAddress, const QList<quint16>& values);
    bool getConnectionState();

    // 同步写一个或多个寄存器
    bool syncWriteRegister(quint16 startAddress, const QVector<quint16>& values);
    // 同步读多个寄存器
    bool syncReadHoldingReister(quint16 startAddress, quint16 quantity, QVector<quint16>& result);
    bool syncReadInputReister(quint16 startAddress, quint16 quantity, QVector<quint16>& result);

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
    std::string m_deviceName;
};
#endif // MYMODBUS_H
