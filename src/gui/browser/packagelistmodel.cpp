/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "packagelistmodel.h"

PackageListModel::PackageListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int PackageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_packages.count();
}

Qt::ItemFlags PackageListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
}

QVariant PackageListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const QString item = m_packages.at(index.row());
        return item;
    }
    return QVariant();
}

QVariant PackageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return tr("Package");
    } else {
        return QVariant();
    }
}

void PackageListModel::addPackage(const QString &package)
{
    int index = m_packages.count();
    beginInsertRows(QModelIndex(), index, index);
    m_packages.append(package);
    endInsertRows();
    m_packages.sort();
}

void PackageListModel::addPackageList(const QStringList &packages)
{
    beginResetModel();
    m_packages = packages;
    endResetModel();
}

bool PackageListModel::removePackage(const QString &package)
{
    int index = m_packages.indexOf(package);
    if (index==-1) {
        return false;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_packages.removeAt(index);
    endRemoveRows();
    return true;
}

QStringList PackageListModel::stringList() const
{
    return m_packages;
}
