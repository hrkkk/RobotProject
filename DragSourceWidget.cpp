#include "DragSourceWidget.h"

#include <QVBoxLayout>
#include <QLabel>

DragSourceWidget::DragSourceWidget(QWidget *parent) :
    QWidget(parent),
    // m_dragStartPosition(QPoint(0, 0)),
    m_drag(new QDrag(this)),
    m_mimeData(new QMimeData)
{
    setAcceptDrops(true);
    this->setStyleSheet("background-color: rgb(85, 0, 255); width: 20px; height: 20px;");
    QVBoxLayout* layout = new QVBoxLayout();
    QLabel* label = new QLabel("hello");
    layout->addWidget(label);
    this->setLayout(layout);
}

void DragSourceWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << "pressed";
        // m_dragStartPosition = event->pos() - frameGeometry().topLeft();
        m_mimeData->setText("DragData");
        m_drag->setMimeData(m_mimeData);
        m_drag->exec(Qt::MoveAction);
        m_lastPos = event->globalPos() - this->pos();
        m_isDragging = true;
    }
}

void DragSourceWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        // qDebug() << "drag";

        // return;
        move(event->globalPos() - m_lastPos);
    }
}

void DragSourceWidget::mouseReleaseEvent(QMouseEvent *event)
{

}
