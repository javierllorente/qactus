/*
 * Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    if (!m_packages.contains(package)) {
        int index = m_packages.count();
        beginInsertRows(QModelIndex(), index, index);
        m_packages.append(package);
        endInsertRows();
        m_packages.sort();
    }
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
