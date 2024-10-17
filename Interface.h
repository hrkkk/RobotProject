#ifndef INTERFACE_H
#define INTERFACE_H

#include "IOController.h"
#include "GlobalVariable.cpp"

class Interface
{
public:
    Interface();

    // 温控箱开门
    int boxOpen(int index);
    // 温控箱关门
    int boxClose(int index);
    // 温控箱锁紧
    int boxLocking(int index);
    // 温控箱解锁
    int boxUnlocking(int index);
    // 查询温控箱门的开关状态
    bool getBoxOpenState(int index);
    // 查询温控箱门的锁紧状态
    bool getBoxLockState(int index);

private:
    IOController* m_ioController;
};

#endif // INTERFACE_H
