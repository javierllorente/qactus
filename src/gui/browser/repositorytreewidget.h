/*
 * Copyright (C) 2020 Javier Llorente <javier@opensuse.org>
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
#ifndef REPOSITORYTREEWIDGET_H
#define REPOSITORYTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QStandardItemModel>
#include "obsrepository.h"

class RepositoryTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    RepositoryTreeWidget(QWidget *parent = nullptr);
    void addRepository(OBSRepository *repository);
    bool removeRepository(const QString &repository);
    bool removeRow(const QModelIndex &index);
    int rowCount() const;
    QList<OBSRepository *> getRepositories() const;
    QWidget *createButtonBar();

private:
    QStandardItemModel *model;

signals:
    void repositoryRemoved(const QString &repository);
    void itemChanged(QStandardItem *item);

};

#endif // REPOSITORYTREEWIDGET_H
