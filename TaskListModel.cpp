#include "TaskListModel.h"

TaskListModel::TaskListModel(QObject *parent)
    : QAbstractTableModel{parent}
{}

void TaskListModel::addData(const Task &data)
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.push_back(data);
    endInsertRows();
}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

int TaskListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 8;
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_data.size() || index.column() < 0 || index.column() >= columnCount()) {
        return QVariant();
    }

    if (role == DisplayRole) {
        auto iter = std::next(m_data.begin(), index.row());
        return (*iter)[index.column()];
    }
    return QVariant();
}

QVariant TaskListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        // 设置列标题
        switch (section) {
            case 0: return u8"取料位";
            case 1: return u8"放料位";
            case 2: return u8"设备号";
            case 3: return u8"单元号";
            case 4: return u8"通道号";
            case 5: return u8"工步文件";
            case 6: return u8"启动时间";
            case 7: return u8"状态";
        }
    }
    return tr("%1").arg(section);
}

// bool TaskListModel::setData(const QModelIndex &index, const QVariant &value, int role)
// {
//     if (!index.isValid() || index.row() < 0 || index.row() >= m_data.size() || index.column() < 0 || index.column() >= columnCount()) {
//         return false;
//     }

//     if (role == Qt::EditRole) {
//         auto iter = std::next(m_data.begin(), index.row());
//         switch (index.column()) {
//         case 0: iter->shelvesID = value.toInt(); break;
//         case 1: iter->fixtureID = value.toInt(); break;
//         case 2: iter->deviceID = value.toInt(); break;
//         case 3: iter->cellID = value.toInt(); break;
//         case 4: iter->channelID = value.toInt(); break;
//         case 5: iter->stepFilename = value.toString().toStdString(); break;
//         default: return false;
//         }

//         emit dataChanged(index, index, {role});
//         return true;
//     }
//     return false;
// }

Qt::ItemFlags TaskListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

void TaskListModel::removeData(int row)
{
    if (row < 0 || row >= m_data.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    // 使用迭代器访问指定索引的元素
    auto iter = std::next(m_data.begin(), row);
    // 删除该元素
    m_data.erase(iter);
    endRemoveRows();
}

void TaskListModel::modifyData(int row, const Task& newData)
{
    if (row < 0 || row >= m_data.size()) {
        return;
    }

    auto iter = std::next(m_data.begin(), row);
    *iter = newData;

    QModelIndex index = this->index(row, 0, QModelIndex());
    emit dataChanged(index, index, {Qt::DisplayRole});
}

Task TaskListModel::getData(int row) const
{
    if (row < 0 || row >= m_data.size()) {
        return {};
    }

    return *(std::next(m_data.begin(), row));
}
