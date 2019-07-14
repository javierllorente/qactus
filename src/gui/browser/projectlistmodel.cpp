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

#include "projectlistmodel.h"

ProjectListModel::ProjectListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int ProjectListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_projects.count();
}

Qt::ItemFlags ProjectListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
}

QVariant ProjectListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const QString item = m_projects.at(index.row());
        return item;
    }
    return QVariant();
}

QVariant ProjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return tr("Projects");
    } else {
        return QVariant();
    }
}

void ProjectListModel::addProject(const QString &project)
{
    int index = m_projects.count();
    beginInsertRows(QModelIndex(), index, index);
    m_projects.append(project);
    endInsertRows();
}

void ProjectListModel::addProjectList(const QStringList &projects)
{
    beginResetModel();
    m_projects = projects;
    endResetModel();
}

bool ProjectListModel::removeProject(const QString &project)
{
    int index = m_projects.indexOf(project);
    if (index==-1) {
        return false;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_projects.removeAt(index);
    endRemoveRows();
    return true;
}

QStringList ProjectListModel::stringList() const
{
    return m_projects;
}
