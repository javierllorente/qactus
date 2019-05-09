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

#include "packagetreewidget.h"

PackageTreeWidget::PackageTreeWidget(QWidget *parent) :
    QTreeView(parent),
    sourceModelPackages(new PackageListModel(this)),
    proxyModelPackages(new QSortFilterProxyModel(this))
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    obs = nullptr;
    createModel();
}

void PackageTreeWidget::setOBS(OBS *obs)
{
    this->obs = obs;
    connect(obs, &OBS::finishedParsingPackageList, this, &PackageTreeWidget::addPackageList);
}

void PackageTreeWidget::createModel()
{
    proxyModelPackages = new QSortFilterProxyModel(this);
    setModel(proxyModelPackages);
}

void PackageTreeWidget::deleteModel()
{
    if (proxyModelPackages) {
        delete proxyModelPackages;
        proxyModelPackages = nullptr;
    }
}

void PackageTreeWidget::addPackageList(const QStringList &packageList)
{
    qDebug() << "PackageTreeWidget::addPackageList()";
    sourceModelPackages->addPackageList(packageList);
    proxyModelPackages->setSourceModel(sourceModelPackages);
    setModel(proxyModelPackages);

    emit updateStatusBar(tr("Done"), true);
}

QStringList PackageTreeWidget::getPackageList() const
{
    return sourceModelPackages->stringList();
}

QString PackageTreeWidget::getCurrentPackage() const
{
    return currentIndex().data().toString();
}

void PackageTreeWidget::filterPackages(const QString &item)
{
    qDebug() << "PackageTreeWidget::filterPackages()" << item;
    proxyModelPackages->setFilterRegExp(QRegExp(item, Qt::CaseInsensitive));
    proxyModelPackages->setFilterKeyColumn(0);
}

bool PackageTreeWidget::removePackage(const QString &package)
{
    QModelIndexList itemList = model()->match(model()->index(0, 0),
                                              Qt::DisplayRole, QVariant::fromValue(QString(package)),
                                              1, Qt::MatchExactly);
    if(!itemList.isEmpty()) {
        auto itemIndex = itemList.at(0);
        model()->removeRow(itemIndex.row(), itemIndex.parent());
        return true;
    }
    return false;
}

void PackageTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
}
