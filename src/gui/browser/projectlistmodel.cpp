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
    if (!m_projects.contains(project)) {
        int index = m_projects.count();
        beginInsertRows(QModelIndex(), index, index);
        m_projects.append(project);
        endInsertRows();
        m_projects.sort();
    }
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
