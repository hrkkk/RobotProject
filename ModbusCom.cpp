#include "ModbusCom.h"

MyModbus::MyModbus(QObject *parent) : QObject(parent)
{
    m_modbusClient = new QModbusTcpClient();
    connect(m_modbusClient, &QModbusClient::stateChanged, this, &MyModbus::slot_stateChanged);
    connect(m_modbusClient, &QModbusClient::errorOccurred, this, [=]() {
        emit signal_errorOccurred();
        qDebug() << "error occurred";
    });
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
    //else
    //{
    //    myClient->disconnectDevice();
    //}
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
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    //确定寄存器类型
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
        // LOGDEBUG << "读取寄存器类型错误";
        return false;
    }
    // LOGDEBUG << "readModbusData typeNum:" << typeNum;

    //多读
    if(auto *reply = m_modbusClient->sendReadRequest(ReadUnit, 1)) {
        if(!reply->isFinished()) {
            if((typeNum == 1) || (typeNum == 2)) {
                QObject::connect(reply, &QModbusReply::finished, this, &MyModbus::slot_readReadyCoils);   //读取线圈
            }
            if((typeNum == 3) || (typeNum == 4)) {
                QObject::connect(reply, &QModbusReply::finished, this, &MyModbus::slot_readReadyRegisters);   //读取寄存器
            }
            //reply->deleteLater();
            return true;
        } else {
            reply->deleteLater();
            return false;
        }
    } else {
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
        //LOGDEBUG<<"uData16[0]:"<<uData16[0]<<"   uData16[1]:"<<uData16[1]<<"   writeNum:"<<writeNum;
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

// 写入单个保持寄存器
bool MyModbus::writeSingleHoldingRegister(quint16 startAddress, quint16 value)
{
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, 1);
    request.setValue(0, value);
    QModbusReply* reply = m_modbusClient->sendWriteRequest(request, 1);

    if (!reply) {
        qDebug() << "Failed to send write request.";
        return false;
    }

    QObject::connect(reply, &QModbusReply::finished, this, [reply, this]() {
        if (reply->error() != QModbusDevice::NoError) {
            qDebug() << "Error writing data to Modbus: " << reply->errorString();
            emit signal_errorOccurred();
        }
        delete reply;
    });

    return true;
}

// 读取保持寄存器
bool MyModbus::readHoldingRegisters(quint16 startAddress, quint16 quantity, QModbusDataUnit &result)
{
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, quantity);
    QModbusReply* reply = m_modbusClient->sendReadRequest(request, 1);

    if (!reply) {
        qDebug() << "Failed to send read request";
        return false;
    }

    QObject::connect(reply, &QModbusReply::finished, this, [this, &result, reply]() {
        if (reply->error() == QModbusDevice::NoError) {
            result = reply->result();
        } else {
            qDebug() << "Error reading data from Modbus: " << reply->errorString();
            emit signal_errorOccurred();
        }
        delete reply;
    });

    return true;
}

// 写入多个保持寄存器
bool MyModbus::writeMultiHoldingRegisters(quint16 startAddress, const QList<quint16> &values)
{
    if(m_modbusClient->state() != QModbusDevice::ConnectedState) {
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, values.size());
    for (int i = 0; i < values.size(); ++i) {
        request.setValue(i, values[i]);
    }
    QModbusReply* reply = m_modbusClient->sendWriteRequest(request, 1);

    if (!reply) {
        qDebug() << "Failed to send write request.";
        return false;
    }

    QObject::connect(reply, &QModbusReply::finished, this, [reply, this]() {
        if (reply->error() != QModbusDevice::NoError) {
            qDebug() << "Error writing data to Modbus: " << reply->errorString();
            emit signal_errorOccurred();
        }
        delete reply;
    });

    return true;
}

//监听TCP连接的状态,若状态发生改变,发出对应的信号
void MyModbus::slot_stateChanged()
{
    // LOGDEBUG<<m_modbusClient->state();
    if(m_modbusClient->state() == QModbusDevice::ConnectedState){
        emit signal_stateChanged(true);
    } else if(m_modbusClient->state() == QModbusDevice::UnconnectedState) {
        emit signal_stateChanged(false);
    }
}

//接收到读取线圈/离散输入寄存器请求后执行的槽函数
void MyModbus::slot_readReadyCoils()
{
    QVector<quint16> vAllData;
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if(!reply) {
        // LOGDEBUG<<"读取线圈/离散输入寄存器错误";
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
    // qDebug() << reply->rawResult();      // 数据组成：  指令（1字节）   长度（1字节）   实际载荷
    if (!reply) {
        // LOGDEBUG<<"读取保持/输入寄存器错误";
        return;
    }
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QVector<quint16> valueList = unit.values();
        // 发送数据到调用层
        emit signal_receivedData(valueList);
        // qDebug() << valueList;      // 数据组成：  实际载荷（长度为寄存器个数）
    } else {
        LOGDEBUG << "Reply error:" << reply->error();
        emit signal_errorOccurred();
    }
    reply->deleteLater();
}
