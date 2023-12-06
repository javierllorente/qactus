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
#ifndef PACKAGETREEWIDGET_H
#define PACKAGETREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include "packagelistmodel.h"
#include <QSortFilterProxyModel>

class PackageTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    PackageTreeWidget(QWidget *parent = nullptr);
    void createModel();
    void deleteModel();
    QStringList getPackageList() const;
    QString getCurrentPackage() const;
    bool removePackage(const QString &package);
    void clearModel();

public slots:
    void addPackage(const QString &package);
    void addPackageList(const QStringList &packageList);
    bool setCurrentPackage(const QString &package);
    void filterPackages(const QString &item);

private:
    PackageListModel *sourceModelPackages;
    QSortFilterProxyModel *proxyModelPackages;

signals:
    void updateStatusBar(QString message, bool progressBarHidden);

};

#endif // PACKAGETREEWIDGET_H
