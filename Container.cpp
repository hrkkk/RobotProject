#include "Container.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QDrag>

Container::Container(QWidget *parent)
    : QWidget{parent},
    m_layout(new QVBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    this->setAcceptDrops(true);
}

void Container::addWidget(QWidget *widget)
{
    m_layout->addWidget(widget);
}

void Container::removeWidget(QWidget* widget)
{
    m_layout->removeWidget(widget);
}

void Container::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        emit sig_addPallet(this);
    } else if (event->buttons() & Qt::RightButton) {
        emit sig_removePallet(this);
    }
}

void Container::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        // 这些事件声明也可以放在mousePressEvent(QMouseEvent *a_event)函数中
        QDrag *drag = new QDrag(this);
        //拖拽时也必须声明一个MimeData可以用来传递信息，如果不需要传递信息也要声明并且调用drag的setMimeData()函数，否则会报错。
        QMimeData *mimeData = new QMimeData;
        // QString labelText = this->text();
        QString a ="我开始被拖拽了，发送的拖拽内容是:";
        // this->setText(a);
        //可以调用mimeData的各种方法设置想要mimeData传递的信息
        mimeData->setText(a);
        emit sig_dragStart(this->objectName());
        drag->setMimeData(mimeData);
        drag->exec();//最后调用exec()即可
    }
}

void Container::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText())
    {
        event->acceptProposedAction();
    }
}

void Container::dropEvent(QDropEvent* event)
{
    emit sig_dragFinish(this->objectName());
}

void Container::paintEvent(QPaintEvent* event) {
    if (this->objectName().contains("shelves")) {
        QPainter painter(this);
        QPixmap pixmap(":/resources/add.png");
        painter.drawPixmap(QRect(rect().center().x() - rect().height() / 4, rect().center().y() - rect().height() / 4, rect().height() / 2, rect().height() / 2), pixmap);
    }
}
