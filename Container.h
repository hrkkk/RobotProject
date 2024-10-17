#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

class Container : public QWidget
{
    Q_OBJECT
public:
    explicit Container(QWidget *parent = nullptr);

    void addWidget(QWidget* widget);
    void removeWidget(QWidget* widget);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void sig_addPallet(Container* self);
    void sig_removePallet(Container* self);
    void sig_dragStart(const QString& sourceWidget);
    void sig_dragFinish(const QString& targetWidget);

private:
    QVBoxLayout* m_layout;
};
