/*
 * Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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
#ifndef PROJECTTREEWIDGET_H
#define PROJECTTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include "projectlistmodel.h"
#include <QSortFilterProxyModel>

class ProjectTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    ProjectTreeWidget(QWidget *parent = nullptr);
    QString getCurrentProject() const;
    void addProjectList(const QStringList &projectList);
    void addProject(const QString &project);
    QStringList getProjectList() const;
    bool removeProject(const QString &project);
    void filterProjects(const QString &item);

private:
    ProjectListModel *sourceModelProjects;
    QSortFilterProxyModel *proxyModelProjects;
    void scrollToCurrentIndex();

public slots:
    bool setCurrentProject(const QString &project);
};

#endif // PROJECTTREEWIDGET_H
