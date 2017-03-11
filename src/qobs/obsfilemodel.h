#ifndef OBSFILEMODEL_H
#define OBSFILEMODEL_H

#include <QAbstractItemModel>

class OBSFileModel : public QAbstractItemModel
{
public:
    OBSFileModel();
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif // OBSFILEMODEL_H
