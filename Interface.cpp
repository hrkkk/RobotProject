#include "Interface.h"

Interface::Interface() {}

int Interface::boxOpen(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return -1;
    }

    // 查询温控箱是否为锁紧状态，如若是，则不执行开门动作
    if (getBoxLockState(index)) {
        return -1;
    }

    if (index == 1) {
        m_ioController->writeBit(DOOR_1_OUT, 1);
    } else if (index == 2) {
        m_ioController->writeBit(DOOR_2_OUT, 1);
    } else if (index == 3) {
        m_ioController->writeBit(DOOR_3_OUT, 1);
    }

    return 1;
}

int Interface::boxClose(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return -1;
    }

    if (index == 1) {
        m_ioController->writeBit(DOOR_1_OUT, 0);
    } else if (index == 2) {
        m_ioController->writeBit(DOOR_2_OUT, 0);
    } else if (index == 3) {
        m_ioController->writeBit(DOOR_3_OUT, 0);
    }
}

int Interface::boxLocking(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return -1;
    }

    if (index == 1) {
        m_ioController->writeBit(DOOR_LOCK_1_OUT, 1);
    } else if (index == 2) {
        m_ioController->writeBit(DOOR_LOCK_2_OUT, 1);
    } else if (index == 3) {
        m_ioController->writeBit(DOOR_LOCK_3_OUT, 1);
    }
}

int Interface::boxUnlocking(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return -1;
    }

    if (index == 1) {
        m_ioController->writeBit(DOOR_LOCK_1_OUT, 0);
    } else if (index == 2) {
        m_ioController->writeBit(DOOR_LOCK_2_OUT, 0);
    } else if (index == 3) {
        m_ioController->writeBit(DOOR_LOCK_3_OUT, 0);
    }
}

bool Interface::getBoxOpenState(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return -1;
    }

    int ret1 = 0, ret2 = 0;
    if (index == 1) {
        ret1 = m_ioController->readBit(DOOR_1_IN1);
        ret2 = m_ioController->readBit(DOOR_1_IN2);
    } else if (index == 2) {
        ret1 = m_ioController->readBit(DOOR_2_IN1);
        ret2 = m_ioController->readBit(DOOR_2_IN2);
    } else if (index == 3) {
        ret1 = m_ioController->readBit(DOOR_2_IN1);
        ret2 = m_ioController->readBit(DOOR_2_IN2);
    }

    if (ret1 == 1 && ret2 == 0) {
        return true;
    } else {
        return false;
    }
}

bool Interface::getBoxLockState(int index)
{
    if (index != 1 && index != 2 && index != 3) {
        return -1;
    }

    int ret1 = 0, ret2 = 0;
    if (index == 1) {
        ret1 = m_ioController->readBit(DOOR_LOCK_1_IN1);
        ret2 = m_ioController->readBit(DOOR_LOCK_1_IN2);
    } else if (index == 2) {
        ret1 = m_ioController->readBit(DOOR_LOCK_2_IN1);
        ret2 = m_ioController->readBit(DOOR_LOCK_2_IN2);
    } else if (index == 3) {
        ret1 = m_ioController->readBit(DOOR_LOCK_2_IN1);
        ret2 = m_ioController->readBit(DOOR_LOCK_2_IN2);
    }

    if (ret1 == 1 && ret2 == 0) {
        return true;
    } else {
        return false;
    }
}
