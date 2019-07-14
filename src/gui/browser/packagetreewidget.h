/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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
    void addPackageList(const QStringList &packageList);
    void filterPackages(const QString &item);

private:
    PackageListModel *sourceModelPackages;
    QSortFilterProxyModel *proxyModelPackages;

signals:
    void updateStatusBar(QString message, bool progressBarHidden);

};

#endif // PACKAGETREEWIDGET_H
