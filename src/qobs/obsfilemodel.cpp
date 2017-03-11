#include "obsfilemodel.h"

OBSFileModel::OBSFileModel()
{

}

Qt::ItemFlags OBSFileModel::flags(const QModelIndex &index) const
{
    // Just selectable items
    return QAbstractItemModel::flags(index);
}

QModelIndex OBSFileModel::parent(const QModelIndex &index) const
{
    // No children
    return QModelIndex();
}

int OBSFileModel::rowCount(const QModelIndex &parent) const
{
    return children().count();
}
int OBSFileModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant OBSFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }

//    return QAbstractItemModel::data(&index.column(), role);
    return QVariant();
}
