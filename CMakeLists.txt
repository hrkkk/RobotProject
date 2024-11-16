cmake_minimum_required(VERSION 3.5)

project(RobotProject VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialBus SerialPort)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RobotProject
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET RobotProject APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(RobotProject SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(RobotProject
            ${PROJECT_SOURCES}
            TrackController.h TrackController.cpp
            RobotController.h RobotController.cpp
            ModbusCom.h ModbusCom.cpp
            IOController.h IOController.cpp
            GlobalVariable.cpp
            GlobalVariable.h
            res.qrc
            tinyxml_2_6_2/tinyxml/tinystr.h
            tinyxml_2_6_2/tinyxml/tinyxml.h
            tinyxml_2_6_2/tinyxml/tinystr.cpp
            tinyxml_2_6_2/tinyxml/tinyxml.cpp
            tinyxml_2_6_2/tinyxml/tinyxmlerror.cpp
            tinyxml_2_6_2/tinyxml/tinyxmlparser.cpp
            cmvcamera.cpp
            cmvcamera.h
            mythread.cpp
            mythread.h
            HCNetSDK.h
            Monitor1.cpp
            Monitor2.cpp
            Monitor1.h
            Monitor2.h
            MonitorWidget.cpp
            MonitorWidget.h
            plaympeg4.h
            TaskListModel.h
            TaskListModel.cpp
        )
    endif()
endif()

# OpenCV链接库
set(OpenCV_DIR "D:/OpenCV/opencv/opencvbuild/install/x64/mingw/lib")
find_package(OpenCV REQUIRED)
include_directories(RobotProject ${OpenCV_INCLUDE_DIRS})
link_libraries("D:/OpenCV/opencv/build/x64/vc16/lib/opencv_world481d.lib")

# MVS链接库C:/Users/panrou/Desktop/RobotProject/SDK/Lib
set(HAIKANG_INCLUDE "${PROJECT_SOURCE_DIR}/SDK/Includes")
set(HAIKANG_LIBS_DIR "${PROJECT_SOURCE_DIR}/SDK/Lib")
#set(HAIKANG_LIBS MvCameraControl)
#set(SOURCE_FILES main.cpp)
include_directories(${HAIKANG_INCLUDE})
link_directories(${HAIKANG_LIBS_DIR})

# 配置海康监控静态链接库
link_libraries("HCNetSDK.lib")
link_directories("C:/Users/panrou/Desktop/RobotProject/MonSDK")


target_link_libraries(RobotProject
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt5::SerialBus Qt5::SerialPort
    PRIVATE ${OpenCV_LIBS}
    PRIVATE D:/OpenCV/opencv/build/x64/vc16/lib/opencv_world481d.lib
    PRIVATE ${PROJECT_SOURCE_DIR}/SDK/Lib/MvCameraControl.lib
    PRIVATE ${PROJECT_SOURCE_DIR}/MonSDK/HCNetSDK.lib)


# target_link_libraries(RobotProject
#     PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt5::SerialBus Qt5::SerialPort)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.RobotProject)
endif()
set_target_properties(RobotProject PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS RobotProject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RobotProject)
endif()

if (MSVC)
    add_compile_options(/utf-8)
endif()


# win32:CONFIG(release, debug|release): LIBS += -L$$PWD/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/release/ -lHCNetSDK
# else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug/ -lHCNetSDK

# INCLUDEPATH += $$PWD/''
# DEPENDPATH += $$PWD/''

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/release/libHCNetSDK.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug/libHCNetSDK.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/release/HCNetSDK.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug/HCNetSDK.lib

