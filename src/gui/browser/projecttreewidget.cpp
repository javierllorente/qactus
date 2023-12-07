/*
 * Copyright (C) 2018-2023 Javier Llorente <javier@opensuse.org>
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
#include "projecttreewidget.h"

ProjectTreeWidget::ProjectTreeWidget(QWidget *parent) :
    QTreeView(parent),
    sourceModelProjects(new ProjectListModel(this)),
    proxyModelProjects(new QSortFilterProxyModel(this))
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    proxyModelProjects->setSourceModel(sourceModelProjects);
    setModel(proxyModelProjects);
}

void ProjectTreeWidget::scrollToCurrentIndex()
{
    scrollTo(currentIndex(), QAbstractItemView::PositionAtTop);
}

QString ProjectTreeWidget::getCurrentProject() const
{
    return currentIndex().data().toString();
}

bool ProjectTreeWidget::setCurrentProject(const QString &project)
{
    QModelIndexList itemList = model()->match(model()->index(0, 0),
                                              Qt::DisplayRole,
                                              QVariant::fromValue(QString(project)), 1, Qt::MatchExactly);
    if (!itemList.isEmpty()) {
        auto itemIndex = itemList.at(0);
        selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        scrollTo(itemIndex, QAbstractItemView::PositionAtTop);
        return true;
    }
    return false;
}

void ProjectTreeWidget::addProjectList(const QStringList &projectList)
{
    selectionModel()->clear(); // Emits selectionChanged() and currentChanged()
    sourceModelProjects->addProjectList(projectList);
}

void ProjectTreeWidget::addProject(const QString &project)
{
    sourceModelProjects->addProject(project);
}

QStringList ProjectTreeWidget::getProjectList() const
{
    return sourceModelProjects->stringList();
}

bool ProjectTreeWidget::removeProject(const QString &project)
{
    return sourceModelProjects->removeProject(project);
}

void ProjectTreeWidget::filterProjects(const QString &item)
{
    proxyModelProjects->setFilterRegularExpression(QRegularExpression(item));
    proxyModelProjects->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModelProjects->setFilterKeyColumn(0);

    scrollToCurrentIndex();
}

