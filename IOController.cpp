#include "IOController.h"

IOController::IOController() : m_isConnected(false), m_lastIP("192.168.1.7"), m_lastPort(8234)
{
    _modbus = new MyModbus("IO Module");
    connect(_modbus, &MyModbus::signal_stateChanged, this, &IOController::slot_stateChanged);
    connect(_modbus, &MyModbus::signal_receivedData, this, &IOController::slot_receivedData);
    connect(_modbus, &MyModbus::signal_errorOccurred, this, [=]() {
        // reconnectToPLC();
        // disconnectWithPLC();

        qDebug() << "error error " << isConnected();

        // 发送信号给UI
        emit sig_errorOccurred();
    });
}

void IOController::connectToPLC(const QString& ip, int port)
{
    _modbus->connectToModbus(ip, port);
    m_lastIP = ip;
    m_lastPort = port;
}

void IOController::disconnectWithPLC()
{
    _modbus->disconnect();
}

void IOController::reconnectToPLC()
{
    // 断开连接
    if (isConnected()) {
        _modbus->disconnect();
    }
    // 等待断开
    while (_modbus->getConnectionState()) {}
    // 重新连接
    _modbus->connectToModbus(m_lastIP, m_lastPort);
    qDebug() << "IO module reconnect";
}

void IOController::boxOpen(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return;
    }

    // 查询温控箱是否为锁紧状态，如若是，则不执行开门动作
    if (getBoxLockState(index) != LOCK_STATE::UNLOCKING) {
        return;
    }

    if (index == 1) {
        writeBit(DOOR_1_OUT, 1);
    } else if (index == 2) {
        writeBit(DOOR_2_OUT, 1);
    } else if (index == 3) {
        writeBit(DOOR_3_OUT, 1);
    }
}

void IOController::boxClose(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return;
    }

    // 查询温控箱是否为锁紧状态，如若是，则不执行关门动作
    if (getBoxLockState(index) != LOCK_STATE::UNLOCKING) {
        return;
    }

    if (index == 1) {
        writeBit(DOOR_1_OUT, 0);
    } else if (index == 2) {
        writeBit(DOOR_2_OUT, 0);
    } else if (index == 3) {
        writeBit(DOOR_3_OUT, 0);
    }
}

void IOController::boxLocking(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return;
    }

    if (index == 1) {
        writeBit(DOOR_LOCK_1_OUT, 0);
    } else if (index == 2) {
        writeBit(DOOR_LOCK_2_OUT, 0);
    } else if (index == 3) {
        writeBit(DOOR_LOCK_3_OUT, 0);
    }
}

void IOController::boxUnlocking(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return;
    }

    if (index == 1) {
        writeBit(DOOR_LOCK_1_OUT, 1);
    } else if (index == 2) {
        writeBit(DOOR_LOCK_2_OUT, 1);
    } else if (index == 3) {
        writeBit(DOOR_LOCK_3_OUT, 1);
    }
}

/*
 * 获取检测箱门是否打开
 */
DOOR_STATE IOController::getBoxOpenState(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return DOOR_STATE::OTHER;
    }

    int ret1 = 0, ret2 = 0;
    if (index == 1) {
        ret1 = readBit(DOOR_1_IN1);
        ret2 = readBit(DOOR_1_IN2);
    } else if (index == 2) {
        ret1 = readBit(DOOR_2_IN1);
        ret2 = readBit(DOOR_2_IN2);
    } else if (index == 3) {
        ret1 = readBit(DOOR_3_IN1);
        ret2 = readBit(DOOR_3_IN2);
    }

    if (ret1 == 0 && ret2 == 1) {
        return DOOR_STATE::OPENING;
    } else if (ret1 == 1 && ret2 == 0) {
        return DOOR_STATE::CLOSING;
    }

    return DOOR_STATE::OTHER;
}

/*
 * 获取检测箱门是否上锁
 */
LOCK_STATE IOController::getBoxLockState(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return LOCK_STATE::OTHER;
    }

    int ret1 = 0, ret2 = 0;
    if (index == 1) {
        ret1 = readBit(DOOR_LOCK_1_IN1);
        ret2 = readBit(DOOR_LOCK_1_IN2);
    } else if (index == 2) {
        ret1 = readBit(DOOR_LOCK_2_IN1);
        ret2 = readBit(DOOR_LOCK_2_IN2);
    } else if (index == 3) {
        ret1 = readBit(DOOR_LOCK_3_IN1);
        ret2 = readBit(DOOR_LOCK_3_IN2);
    }

    if (ret1 == 0 && ret2 == 1) {
        return LOCK_STATE::LOCKING;
    } else if (ret1 == 1 && ret2 == 0) {
        return LOCK_STATE::UNLOCKING;
    }

    return LOCK_STATE::OTHER;
}

// 置1下压
int IOController::fixtureSet(int index, int value)
{
    if (index <= 0 || index > 6) {
        return -1;
    }

    switch (index) {
    case 1:
        writeBit(FIXTURE_1_OUT, value);
        break;
    case 2:
        writeBit(FIXTURE_2_OUT, value);
        break;
    case 3:
        writeBit(FIXTURE_3_OUT, value);
        break;
    case 4:
        writeBit(FIXTURE_4_OUT, value);
        break;
    case 5:
        writeBit(FIXTURE_5_OUT, value);
        break;
    case 6:
        writeBit(FIXTURE_6_OUT, value);
        break;
    }

    return 1;
}

/*
 * 获取治具是否为上升状态
 */
FIXTURE_STATE IOController::getFixtureState(int index)
{
    if (index <= 0 || index > 6) {
        return FIXTURE_STATE::OTHER;
    }

    int ret1 = 0, ret2 = 0;
    switch (index) {
    case 1:
        ret1 = readBit(FIXTURE_1_IN1);      // 左汽缸上位传感器  为1时代表治具上升
        ret2 = readBit(FIXTURE_1_IN2);      // 右气缸上位传感器  为1时代表治具上升
        break;
    case 2:
        ret1 = readBit(FIXTURE_2_IN1);
        ret2 = readBit(FIXTURE_2_IN2);
        break;
    case 3:
        ret1 = readBit(FIXTURE_3_IN1);
        ret2 = readBit(FIXTURE_3_IN2);
        break;
    case 4:
        ret1 = readBit(FIXTURE_4_IN1);
        ret2 = readBit(FIXTURE_4_IN2);
        break;
    case 5:
        ret1 = readBit(FIXTURE_5_IN1);
        ret2 = readBit(FIXTURE_5_IN2);
        break;
    case 6:
        ret1 = readBit(FIXTURE_6_IN1);
        ret2 = readBit(FIXTURE_6_IN2);
        break;
    }

    if (ret1 == 1 && ret2 == 1) {
        return FIXTURE_STATE::UP;
    } else {
        return FIXTURE_STATE::DOWN;
    }
}

void IOController::setWarningLight(bool flag)
{
    if (flag) {
        writeBit(WARNING_LIGHT_OUT, 1);
    } else {
        writeBit(WARNING_LIGHT_OUT, 0);
    }
}

void IOController::slot_stateChanged(bool flag)
{
    m_isConnected = flag;
    emit sig_stateChanged(flag);
}

bool IOController::isConnected()
{
    return m_isConnected;
}

void IOController::slot_receivedData(const QVector<quint16>& data)
{
    // if (data.size() == IO_READ_NUMS) {
    //     _data = data;
    // }
    // emit sig_updateData(data);
}

void IOController::resetAll()
{
    // 上电自检
    // 1. 三个门是否是关闭的
    if (getBoxOpenState(1) != DOOR_STATE::CLOSING ||
        getBoxOpenState(2) != DOOR_STATE::CLOSING ||
        getBoxOpenState(3) != DOOR_STATE::CLOSING) {
        // 报警
        return;
    }
    // 2. 三个门锁是否是解锁的
    if (getBoxLockState(1) != LOCK_STATE::LOCKING ||
        getBoxLockState(2) != LOCK_STATE::LOCKING ||
        getBoxLockState(3) != LOCK_STATE::LOCKING) {
        // 报警
        return;
    }
    // 3. 如果都是关的，则全置为0
    QList<quint16> values;
    for (int i = 0; i < 12; ++i) {
        values.append(0);
    }
    if (isConnected()) {
        _modbus->writeMultiHoldingRegisters(0, values);
    }
}

// 写单个保持寄存器  指令：06
// 从机地址： 1
// 寄存器地址： 0 - (OUTPUT_NUMS - 1)
// 寄存器值： 0  1
void IOController::writeBit(int port, int value)
{
    // if (!isConnected()) {
    //     return;
    // }
    // if (!_modbus->writeSingleHoldingRegister(port, value)) {
    //     qDebug() << "数据发送失败";
    // }

    if (!isConnected()) {
        return;
    }
    QVector<quint16> request(1, value);
    if (!_modbus->syncWriteRegister(port, request)) {
        qDebug() << QStringLiteral("数据发送失败");
    }
}

int IOController::readBit(int port)
{
    // 直接解析_data中的数据即可
    return _data[port];

    // QVector<quint16> result;
    // if (_modbus->syncReadInputReister(IO_BASE_ADDRESS, IO_READ_NUMS, result)) {
    //     if (result.size() == IO_READ_NUMS) {
    //         return result[port];
    //     }
    // }
    // return -1;
}

/*
 * 读取 IO 模块的输入输出状态
 */
void IOController::requestFeedback(int flag)
{
    if (!isConnected()) {
        return;
    }
    // if (flag == 1) {
    //     // 读取输入寄存器 指令：04H
    //     // 从机地址：1
    //     // 寄存器起始地址：0
    //     // 寄存器个数：24
    //     // 返回载荷字节数： 48（每个寄存器16位）
    //     if (!_modbus->readModbusData(4, IO_BASE_ADDRESS, IO_READ_NUMS)) {
    //         qDebug() << "数据读取失败";
    //     }
    // } else {
    //     // 读取保持寄存器  指令：03H
    //     // 从机地址：1
    //     // 寄存器起始地址：0
    //     // 寄存器个数：12
    //     // 返回载荷字节数： 24（每个寄存器16位）
    //     if (!_modbus->readModbusData(3, IO_BASE_ADDRESS, IO_WRITE_NUMS)) {
    //         qDebug() << "数据读取失败";
    //     }
    // }

    if (flag == 1) {
        QVector<quint16> result;
        if (_modbus->syncReadInputReister(IO_BASE_ADDRESS, IO_READ_NUMS, result)) {
            if (result.size() == IO_READ_NUMS) {
                _data = result;
                emit sig_updateData(result);
            }
        }
    } else {
        QVector<quint16> result;
        if (_modbus->syncReadHoldingReister(IO_BASE_ADDRESS, IO_WRITE_NUMS, result)) {
            if (result.size() == IO_WRITE_NUMS) {
                emit sig_updateData(result);
            }
        }
    }
}
