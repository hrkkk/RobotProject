#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include "cmvcamera.h"
#include "mythread.h"



void MonitorWidget();
void MonitorinitWidget();
void MonitorsaveImage(QString format);
void Monitordisplay(const Mat* image);


void slot_display();
void slot_displayImage(QImage myImage);
void on_pb_find_clicked();
void on_pb_open_clicked();
void on_pb_close_clicked();
void on_pb_single_clicked();
void on_pb_start_clicked();
void on_pb_stop_clicked();
void on_pb_saveBmp_clicked();
void on_pb_savePng_clicked();





#endif // MONITORWIDGET_H
