#include "RobotController.h"

#include <QMessageBox>

RobotController::RobotController() : m_isConnected(false)
{
    _modbus = new MyModbus();
    connect(_modbus, &MyModbus::signal_stateChanged, this, &RobotController::slot_stateChanged);
    connect(_modbus, &MyModbus::signal_receivedData, this, &RobotController::slot_receivedData);
}

void RobotController::connectToPLC(const QString& ip, int port)
{
    _modbus->connectToModbus(ip, port);
}

void RobotController::disconnectWithPLC()
{
    _modbus->disconnect();
}

void RobotController::getFromShelves(int index)
{
    if (index <= 0 || index > 12) {
        return;
    }

    executeAction(ZWJQ1 + index - 1);
}

bool RobotController::resultOfGetFromShelves(int index)
{
    if (index <= 0 || index > 12) {
        return false;
    }

    return getFeedback(ZWJQ1 + index - 1);
}

void RobotController::putToFixture(int index)
{
    if (index <= 0 || index > 6) {
        return;
    }

    executeAction(WKXF1 + index - 1);
}

bool RobotController::resultOfPutToFixture(int index)
{
    if (index <= 0 || index > 6) {
        return false;
    }

    return getFeedback(WKXF1 + index - 1);
}

void RobotController::getFromFixture(int index)
{
    if (index <= 0 || index > 6) {
        return;
    }

    executeAction(WKXQ1 + index - 1);
}

bool RobotController::resultOfGetFromFixture(int index)
{
    if (index <= 0 || index > 6) {
        return false;
    }

    return getFeedback(WKXQ1 + index - 1);
}

void RobotController::putToShelves(int index)
{
    if (index <= 0 || index > 12) {
        return;
    }

    executeAction(ZWJF1 + index - 1);
}

bool RobotController::resultOfPutToShelves(int index)
{
    if (index <= 0 || index > 12) {
        return false;
    }

    return getFeedback(ZWJF1 + index - 1);
}

bool RobotController::requireTrackMove(int pos)
{
    if (pos == SHELVES_POSITION) {
        return getFeedback(ROBOT_TRACK1);
    } else if (pos == BOX_1_POSITION) {
        return getFeedback(ROBOT_TRACK2);
    } else if (pos == BOX_2_POSITION) {
        return getFeedback(ROBOT_TRACK3);
    } else if (pos == BOX_3_POSITION) {
        return getFeedback(ROBOT_TRACK4);
    }

    return false;
}

void RobotController::trackMoveArrived(int pos)
{
    if (pos == SHELVES_POSITION) {
        return executeAction(ROBOT_TRACK1);
    } else if (pos == BOX_1_POSITION) {
        return executeAction(ROBOT_TRACK2);
    } else if (pos == BOX_2_POSITION) {
        return executeAction(ROBOT_TRACK3);
    } else if (pos == BOX_3_POSITION) {
        return executeAction(ROBOT_TRACK4);
    }
}

void RobotController::executeAction(int bit)
{
    // uint64_t sendData = 1ULL << bit;

    // // 分两次写，因为一次只能写32位
    // _modbus->writeModbusData(2, ROBOT_WRITE_BASE_ADDRESS, sendData >> 32);
    // _modbus->writeModbusData(2, ROBOT_WRITE_BASE_ADDRESS + 1, sendData);
    // // _modbus->writeModbusData(2, ROBOT_WRITE_BASE_ADDRESS + 2, sendData >> 16);

    QList<quint16> values;

    if (bit >= 0 && bit < 16) {
        values.append(1 << bit);
        values.append(0);
        values.append(0);
        values.append(0);
    } else if (bit >= 16 && bit < 32) {
        values.append(0);
        values.append(1 << (bit - 16));
        values.append(0);
        values.append(0);
    } else if (bit >= 32 && bit < 48) {
        values.append(0);
        values.append(0);
        values.append(1 << (bit - 32));
        values.append(0);
    } else if (bit >= 48 && bit < 64) {
        values.append(0);
        values.append(0);
        values.append(0);
        values.append(1 << (bit - 48));
    }

    _modbus->writeMultiHoldingRegisters(ROBOT_WRITE_BASE_ADDRESS, values);
}

int RobotController::getFeedback(int bit)
{
    // 直接读取m_data即可
    return m_data[bit];
}

void RobotController::requestFeedback()
{
    if(!_modbus->readModbusData(3, ROBOT_READ_BASE_ADDRESS, 4)) {
        qDebug() << "数据读取失败";
    }
}


void RobotController::slot_receivedData(QVector<quint16> data)
{
    // 提取出有效载荷数据
    analyseFeedback(data);
}

void RobotController::analyseFeedback(const QVector<quint16>& data)
{
    // 将4个uint16转换成64个位
    uint64_t receivedData = 0;

    receivedData = data[0]
                    | (static_cast<uint64_t>(data[1]) << 16)
                    | (static_cast<uint64_t>(data[2]) << 32)
                    | (static_cast<uint64_t>(data[3]) << 48);

    QVector<quint16> actualData(64);
    for (int i = 0; i < 64; ++i) {
        actualData[i] = (receivedData >> i) & 1;
    }

    m_data = actualData;

    emit sig_updateData(actualData);
}

void RobotController::slot_stateChanged(bool flag)
{
    m_isConnected = flag;
    emit sig_stateChanged(flag);
}

bool RobotController::isConnected()
{
    return m_isConnected;
}

void RobotController::clearAllBits()
{
    QList<quint16> values;

    values.append(0);
    values.append(0);
    values.append(0);
    values.append(0);

    _modbus->writeMultiHoldingRegisters(ROBOT_WRITE_BASE_ADDRESS, values);
}
