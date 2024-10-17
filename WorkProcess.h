#ifndef WORKPROCESS_H
#define WORKPROCESS_H

#include <memory>

#include "IOController.h"
#include "RobotController.h"
#include "TrackController.h"


class WorkProcess
{
public:
    WorkProcess();

    void loading(int shelvesId, int boxId, int fixtureId);
    void unloading(int src, int dst);

private:
    std::unique_ptr<IOController> m_ioController;
    std::unique_ptr<RobotController> m_robotController;
    std::unique_ptr<TrackController> m_trackController;
};

#endif // WORKPROCESS_H
