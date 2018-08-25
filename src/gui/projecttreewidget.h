/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#ifndef PROJECTTREEWIDGET_H
#define PROJECTTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QStringListModel>
#include <QSortFilterProxyModel>

class ProjectTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    ProjectTreeWidget(QWidget *parent = 0);
    QString getCurrentProject() const;
    bool setCurrentProject(const QString &project);
    void addProjectList(const QStringList &projectList);
    bool removeProject(const QString &project);
    void filterProjects(const QString &item);

private:
    QStringListModel *sourceModelProjects;
    QSortFilterProxyModel *proxyModelProjects;
    void scrollToCurrentIndex();
};

#endif // PROJECTTREEWIDGET_H
