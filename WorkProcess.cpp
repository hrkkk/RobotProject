#include "WorkProcess.h"

WorkProcess::WorkProcess() :
    m_ioController(std::make_unique<IOController>()),
    m_robotController(std::make_unique<RobotController>()),
    m_trackController(std::make_unique<TrackController>())
{}

void WorkProcess::loading(int shelvesId, int boxId, int fixtureId)
{
    // // 0. 检查指定温控箱是否在工作，内部是否有托盘，门是否为锁紧状态，机械臂是否为原点状态

    // // 1. 温控箱开门
    // m_ioController->boxOpen(boxId);

    // // 2. 等待温控箱开门完毕信号，给机械臂下发叉取托盘命令
    // while (!m_ioController->getBoxLockState(boxId)) {}
    m_robotController->getFromShelves(shelvesId);

    // // 3. 等待机械臂返回地轨移动请求，移动地轨
    while (!m_robotController->requireTrackMove(SHELVES_POSITION)) {}
    m_trackController->moveToTarget(SHELVES_POSITION);

    // // 4. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {}
    m_robotController->trackMoveArrived(SHELVES_POSITION);
    // while (!m_robotController->resultOfGetFromShelves(shelvesId)) {}
    // if (boxId == 0) {
    //     m_trackController->moveToTarget(BOX_1_POSITION);
    // } else if (boxId == 1) {
    //     m_trackController->moveToTarget(BOX_2_POSITION);
    // } else if (boxId == 2) {
    //     m_trackController->moveToTarget(BOX_3_POSITION);
    // }

    // // 5. 等待机械臂返回叉取完成信号，给机械臂下发放置命令
    while (!m_robotController->resultOfGetFromShelves(shelvesId)) {}
    m_robotController->putToFixture(fixtureId);
    // while (!m_trackController->getMoveState()) {}
    // m_robotController->putToFixture(fixtureId);

    // // 6. 等待机械臂返回地轨移动请求，移动地轨
    while (!m_robotController->requireTrackMove(BOX_1_POSITION)) {}
    m_trackController->moveToTarget(BOX_1_POSITION);
    // while (!m_robotController->resultOfPutToFixture(fixtureId)) {}
    // m_trackController->moveToTarget(ORIGIN_POSITION);

    // // 7. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {}
    m_robotController->trackMoveArrived(BOX_1_POSITION);
    // while (!m_trackController->getMoveState()) {}
    // m_ioController->fixtureSet(fixtureId, 1);

    // // 8. 等待机械臂返回放置完成信号，地轨回原点
    while (!m_robotController->resultOfPutToFixture(fixtureId)) {}
    m_trackController->moveToTarget(ORIGIN_POSITION);
    // while (!m_ioController->getFixtureState(fixtureId)) {}
    // m_ioController->boxClose(boxId);

    // // 9. 等待地轨回原点完成，治具下压
    while (!m_trackController->getArrivedSignal()) {}
    qDebug() << "finished";

    // while (m_ioController->getBoxOpenState(boxId)) {}
    // m_ioController->boxLocking(boxId);

    // // 10. 等待治具下压完成信号，温控箱关门

    // // 11. 等待温控箱关门完成信号，温控箱锁门
    // while (!m_ioController->getBoxLockState(boxId)) {}

    // // 12. 等待温控箱锁门完成信号，开始检测
}

void WorkProcess::unloading(int src, int dst)
{
    // 0. 检查指定温控箱是否处于空闲状态，指定货架位置是否有托盘

    // 1. 温控箱解锁

    // 2. 等待温控箱解锁完成信号，温控箱开门

    // 3. 等待温控箱开门完成信号，治具气缸上抬

    // 4. 等待治具气缸上抬完成信号，地轨运动到温控箱取料位

    // 5. 等待地轨运动到位信号，机械臂叉取托盘

    // 6. 等待机械臂叉取完成信号，地轨运动到货架放料位

    // 7. 等待地轨运动到位信号，机械臂放置托盘到指定货架

    // 8. 等待机械臂放置完成信号，地轨运动到原点

    // 9. 等待地轨运动到原点信号，卸料完成
}
