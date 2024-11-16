#pragma once

#include <QAbstractTableModel>
#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QString>

#include <list>

enum class TaskState {
    WAITING,    // 等待中
    READY_LOAD, // 等待上料
    LOADING,    // 上料中
    RUNNING,    // 检测中
    READY_UNLOAD, // 等待卸料
    UNLOADING,  // 卸料中
    FINISHING   // 检测完成
};

struct Task {
    int shelvesID;
    int fixtureID;
    int deviceID;
    int cellID;
    int channelID;
    std::string stepFilename;
    QDateTime date;
    TaskState status;

    QString statusToString(const TaskState& status) const {
        switch (status) {
            case TaskState::WAITING: return u8"等待中";
            case TaskState::READY_LOAD: return u8"等待上料";
            case TaskState::LOADING: return u8"上料中";
            case TaskState::RUNNING: return u8"检测中";
            case TaskState::READY_UNLOAD: return u8"等待下料";
            case TaskState::UNLOADING: return u8"下料中";
            case TaskState::FINISHING: return u8"检测完成";
        }
    }

    QVariant operator [](int index) const {
        switch (index) {
            case 0: return QVariant::fromValue(shelvesID);
            case 1: return QVariant::fromValue(fixtureID);
            case 2: return QVariant::fromValue(deviceID);
            case 3: return QVariant::fromValue(cellID);
            case 4: return QVariant::fromValue(channelID);
            case 5: return QVariant::fromValue(QString::fromStdString(stepFilename));
            case 6: return QVariant::fromValue(date);
            case 7: return QVariant::fromValue(statusToString(status));
            default: return QVariant();
        }
    }
};

class TaskListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Roles {
        DisplayRole = Qt::DisplayRole
    };

    explicit TaskListModel(QObject *parent = nullptr);

    void addData(const Task& data);
    void removeData(int row);
    void modifyData(int row, const Task& newData);
    Task getData(int row) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    // bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    std::list<Task> m_data;
};
