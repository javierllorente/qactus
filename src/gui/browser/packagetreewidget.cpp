/*
 * Copyright (C) 2018-2024 Javier Llorente <javier@opensuse.org>
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
#include "packagetreewidget.h"
#include <QDebug>

PackageTreeWidget::PackageTreeWidget(QWidget *parent) :
    QTreeView(parent),
    sourceModelPackages(new PackageListModel(this)),
    proxyModelPackages(new QSortFilterProxyModel(this))
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    createModel();
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

void PackageTreeWidget::addPackage(const QString &package)
{
    sourceModelPackages->addPackage(package);
}

void PackageTreeWidget::addPackageList(const QStringList &packageList)
{
    qDebug() << __PRETTY_FUNCTION__;
    selectionModel()->clear(); // Emits selectionChanged() and currentChanged()
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

bool PackageTreeWidget::setCurrentPackage(const QString &package)
{
    QModelIndexList itemList = model()->match(model()->index(0, 0),
                                              Qt::DisplayRole,
                                              QVariant::fromValue(QString(package)), 1, Qt::MatchExactly);
    if (!itemList.isEmpty()) {
        auto itemIndex = itemList.at(0);
        selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        scrollTo(itemIndex, QAbstractItemView::PositionAtTop);
        return true;
    } else {
        qDebug() << __PRETTY_FUNCTION__ << "package" << package << "not found";
        emit packageNotFound(package);
    }

    return false;
}

void PackageTreeWidget::filterPackages(const QString &item)
{
    qDebug() << __PRETTY_FUNCTION__ << item;
    proxyModelPackages->setFilterRegularExpression(QRegularExpression(item));
    proxyModelPackages->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModelPackages->setFilterKeyColumn(0);
}

bool PackageTreeWidget::removePackage(const QString &package)
{
    return sourceModelPackages->removePackage(package);
}

void PackageTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
}
