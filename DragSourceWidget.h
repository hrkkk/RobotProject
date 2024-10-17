#pragma once

#include <QWidget>
#include <QDrag>
#include <QMouseEvent>
#include <QMimeData>
#include <QDebug>

class DragSourceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DragSourceWidget(QWidget *parent = nullptr);

signals:

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // QPoint m_dragStartPosition;
    QDrag* m_drag;
    QMimeData* m_mimeData;
    QPoint m_lastPos;
    bool m_isDragging;
};
