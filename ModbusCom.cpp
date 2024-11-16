#include "ModbusCom.h"

MyModbus::MyModbus(const std::string& deviceName, QObject *parent) : QObject(parent)
{
    m_deviceName = deviceName;
    m_modbusClient = new QModbusTcpClient();
    connect(m_modbusClient, &QModbusClient::stateChanged, this, &MyModbus::slot_stateChanged);
    connect(m_modbusClient, &QModbusClient::errorOccurred, this, [=]() {
        emit signal_errorOccurred();

        qDebug() << QString::fromStdString(m_deviceName) << " error: " << "error occurred";
    });
    // m_modbusClient->setNumberOfRetries(3);
    // m_modbusClient->setTimeout(5000);
}

MyModbus::~MyModbus()
{}

//连接到modbus设备
void MyModbus::connectToModbus(QString ip, int port)
{
    if(!m_modbusClient) {
        return;
    }

    //判断当前连接状态是否为断开状态
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        //配置ModbusTcp的连接参数IP+Port
        m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ip);
        m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
        m_modbusClient->connectDevice();
    }
}

void MyModbus::disconnect()
{
    if (!m_modbusClient) {
        return;
    }

    m_modbusClient->disconnectDevice();
}

bool MyModbus::getConnectionState()
{
    if (!m_modbusClient) {
        return false;
    }

    return m_modbusClient->state() == QModbusDevice::ConnectedState;
}

//读取modbus设备各寄存器数据
//typeNum:1_线圈 2_离散输入 3_保持 4_输入
bool MyModbus::readModbusData(int typeNum, int startAdd, quint16 numbers)
{
    // 检查Modbus TCP连接是否正常
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    // 确定寄存器类型
    QModbusDataUnit ReadUnit;
    if(typeNum == 1) {
        ReadUnit = QModbusDataUnit(QModbusDataUnit::Coils, startAdd, numbers);    // 读取线圈状态
    } else if(typeNum == 2) {
        ReadUnit = QModbusDataUnit(QModbusDataUnit::DiscreteInputs, startAdd, numbers);   // 读取离散输入寄存器
    } else if(typeNum == 3) {
        ReadUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAdd, numbers);     // 读取保持寄存器
    } else if(typeNum == 4) {
        ReadUnit = QModbusDataUnit(QModbusDataUnit::InputRegisters, startAdd, numbers);       // 读取输入寄存器
    } else {
        return false;
    }

    // 发送读取请求数据包
    QModbusReply* reply = m_modbusClient->sendReadRequest(ReadUnit, 1);
    if(reply) {     // 如果发送成功
        if(!reply->isFinished()) {
            if((typeNum == 1) || (typeNum == 2)) {  // 读取线圈
                QObject::connect(reply, &QModbusReply::finished, this, &MyModbus::slot_readReadyCoils);
            }
            if((typeNum == 3) || (typeNum == 4)) {  // 读取寄存器
                QObject::connect(reply, &QModbusReply::finished, this, &MyModbus::slot_readReadyRegisters);
            }
            return true;
        } else {
            reply->deleteLater();
            LOGDEBUG << "read request reply is not finished";
            return false;
        }
    } else {    // 如果发送失败
        LOGDEBUG << "Read Error:" + m_modbusClient->errorString();
        emit signal_errorOccurred();
        return false;
    }
}

//对modbus设备各寄存器写入数据
//typeNum:1_线圈 2_保持 (这两类寄存器可读可写,其余的只读)
bool MyModbus::writeModbusData(int typeNum, int startAdd, int value)
{
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    //确定寄存器类型
    QModbusDataUnit writeUnit;
    if(typeNum == 1) {
        writeUnit = QModbusDataUnit(QModbusDataUnit::Coils, startAdd, 1);   // 写入一个线圈数据
        writeUnit.setValue(0, value);

        //单写
        //bool ok;
        //quint16 hexData = writeData.toInt(&ok,16);   //转16进制
    } else if(typeNum == 2) {
        writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAdd, 2);   // 写入保存寄存器数据
        quint16 uData16[2] = {0};
        uData16[0] = value & 0xffff;
        uData16[1] = (value >> 16) & 0xffff;
        writeUnit.setValue(0, uData16[0]);
        writeUnit.setValue(1, uData16[1]);
    } else {
        // LOGDEBUG << "写入寄存器类型错误";
        return false;
    }
    //LOGDEBUG<<"writeModbusData typeNum:"<<typeNum<<"   writeNum:"<<writeNum;
    if (auto *reply = m_modbusClient->sendWriteRequest(writeUnit, 1)) {
        if(!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [reply, this]() {
                if(reply->error() == QModbusDevice::NoError) {
                    reply->deleteLater();
                    return true;
                } else {
                    LOGDEBUG << "Write Error:" << reply->error();
                    emit signal_errorOccurred();
                    reply->deleteLater();
                    return false;
                }
            });
        } else {
            reply->deleteLater();
            return false;
        }
    } else {
        LOGDEBUG << "Write Error:" + m_modbusClient->errorString();
        emit signal_errorOccurred();
        return false;
    }
    return true;
}

/*
 * 写入单个保持寄存器
 */
bool MyModbus::writeSingleHoldingRegister(quint16 startAddress, quint16 value)
{
    // 检查Modbus TCP连接是否正常
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    // 发送写入请求数据包
    QModbusDataUnit request(QModbusDataUnit::InputRegisters, startAddress, 1);
    request.setValue(0, value);
    QModbusReply* reply = m_modbusClient->sendWriteRequest(request, 1);

    // 处理写入响应
    if (!reply) {
        qDebug() << "Failed to send write request.";
        return false;
    } else {
        QObject::connect(reply, &QModbusReply::finished, this, [reply, this]() {
            if (reply->error() != QModbusDevice::NoError) {     // 如果发送了错误
                qDebug() << QString::fromStdString(m_deviceName) << " error: " << "Error writing single data to Modbus: " << reply->errorString();
                emit signal_errorOccurred();
            }
            reply->deleteLater();
        });
    }

    return true;
}

bool MyModbus::syncWriteRegister(quint16 startAddress,const QVector<quint16> &values)
{
    bool isOk = false;
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, startAddress, values.size());
    for (int i = 0; i < values.size(); ++i) {
        writeUnit.setValue(i, values[i]);
    }
    auto reply = m_modbusClient->sendWriteRequest(writeUnit, 1);
    if (!reply) {
        qDebug() << "write request error: " << m_modbusClient->errorString();
        isOk = false;
    } else {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (reply->error() == QModbusDevice::NoError) {
            isOk = true;
        } else {
            qDebug() << "write response error: " << reply->errorString();
            isOk = false;
        }
        reply->deleteLater();
    }
    return isOk;
}

bool MyModbus::syncReadHoldingReister(quint16 startAddress, quint16 quantity, QVector<quint16>& result)
{
    bool isOk = false;
    if (m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return isOk;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddress, quantity);
    auto reply = m_modbusClient->sendReadRequest(readUnit, 1);

    if (!reply) {
        return isOk;
    } else {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit unit = reply->result();
            result = unit.values();
            isOk = true;
        } else {
            isOk = false;
        }
        delete reply;
    }
    return isOk;
}

bool MyModbus::syncReadInputReister(quint16 startAddress, quint16 quantity, QVector<quint16>& result)
{
    bool isOk = false;
    if (m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return isOk;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::InputRegisters, startAddress, quantity);
    auto reply = m_modbusClient->sendReadRequest(readUnit, 1);

    if (!reply) {
        return isOk;
    } else {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit unit = reply->result();
            result = unit.values();
            isOk = true;
        } else {
            isOk = false;
        }
        delete reply;
    }
    return isOk;
}

// 读取保持寄存器
bool MyModbus::readHoldingRegisters(quint16 startAddress, quint16 quantity, QModbusDataUnit &result)
{
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    // 发送读取请求
    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, quantity);
    QModbusReply* reply = m_modbusClient->sendReadRequest(request, 1);

    // 处理读取响应
    if (!reply) {
        qDebug() << QString::fromStdString(m_deviceName) << " error: " << "Failed to send read request";
        return false;
    } else {
        QObject::connect(reply, &QModbusReply::finished, this, [this, &result, reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                result = reply->result();
            } else {
                qDebug() << QString::fromStdString(m_deviceName) << " error: " << "Error reading data from Modbus: " << reply->errorString();
                emit signal_errorOccurred();
            }
            reply->deleteLater();
        });
    }

    return true;
}

// 写入多个保持寄存器
bool MyModbus::writeMultiHoldingRegisters(quint16 startAddress, const QList<quint16> &values)
{
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    // 发送写入请求
    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, values.size());
    for (int i = 0; i < values.size(); ++i) {
        request.setValue(i, values[i]);
    }
    QModbusReply* reply = m_modbusClient->sendWriteRequest(request, 1);

    // 处理写入响应
    if (!reply) {
        qDebug() << QString::fromStdString(m_deviceName) << " error: " << "Failed to send write request.";
        return false;
    } else {
        QObject::connect(reply, &QModbusReply::finished, this, [reply, this]() {
            if (reply->error() != QModbusDevice::NoError) {
                qDebug() << QString::fromStdString(m_deviceName) << " error: " << "Error writing multi data to Modbus: " << reply->errorString();
                emit signal_errorOccurred();
            }
            reply->deleteLater();
        });
    }

    return true;
}

//监听TCP连接的状态,若状态发生改变,发出对应的信号
void MyModbus::slot_stateChanged()
{
    if(m_modbusClient->state() == QModbusDevice::ConnectedState){
        qDebug() << QString::fromStdString(m_deviceName) << ": " << "Modbus Connected";

        emit signal_stateChanged(true);
    } else if(m_modbusClient->state() == QModbusDevice::UnconnectedState) {
        qDebug() << QString::fromStdString(m_deviceName) << ": " << "Modbus Unconnected";

        emit signal_stateChanged(false);
    }
}

//接收到读取线圈/离散输入寄存器请求后执行的槽函数
void MyModbus::slot_readReadyCoils()
{
    QVector<quint16> vAllData;
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if(!reply) {
        return;
    }
    if(reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        vAllData = unit.values();
        emit signal_readCoils(vAllData);
    } else {
        // LOGDEBUG<<"线圈/离散输入寄存器回复错误:"<<reply->error();
    }
    reply->deleteLater();
}

//接收到读取保持/输入寄存器请求后执行的槽函数
void MyModbus::slot_readReadyRegisters()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());

    if (!reply) {
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QVector<quint16> valueList = unit.values();

        // 发送数据到调用层
        emit signal_receivedData(valueList);
    } else {
        qDebug() << QString::fromStdString(m_deviceName) << " error: "  << "Error reading data from Modbus: " << reply->errorString();

        emit signal_errorOccurred();
    }
    reply->deleteLater();
}
