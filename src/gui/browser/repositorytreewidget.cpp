/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2020 Javier Llorente <javier@opensuse.org>
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

#include "repositorytreewidget.h"
#include <QStringList>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

RepositoryTreeWidget::RepositoryTreeWidget(QWidget *parent) :
    QTreeView(parent),
    model(new QStandardItemModel(this))
{
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    QStringList headers = QStringList() << tr("Repository") << tr("Arch") << tr("Path");
    model->setHorizontalHeaderLabels(headers);
    setModel(model);
    setColumnWidth(0, 180);

    connect(model, &QStandardItemModel::itemChanged, this, &RepositoryTreeWidget::itemChanged);
}

void RepositoryTreeWidget::addRepository(OBSRepository *repository)
{
    for (auto arch : repository->getArchs()) {
        QStandardItem *itemName = new QStandardItem();
        itemName->setText(repository->getName());

        QStandardItem *itemArch = new QStandardItem();
        itemArch->setText(arch);
        QStandardItem *itemPath = new QStandardItem();
        itemPath->setText(repository->getProject() + "/" + repository->getRepository());

        QList<QStandardItem *> items = QList<QStandardItem *>() << itemName << itemArch<< itemPath;
        model->appendRow(items);
    }

    if (repository->getArchs().size()==0) {
        QStandardItem *itemName = new QStandardItem();
        QStandardItem *itemArch = new QStandardItem();
        QStandardItem *itemPath = new QStandardItem();
        QList<QStandardItem *> items = QList<QStandardItem *>() << itemName << itemArch<< itemPath;
        model->appendRow(items);
    }
}

bool RepositoryTreeWidget::removeRepository(const QString &repository)
{
    QModelIndexList itemList = model->match(model->index(0, 0),
                                              Qt::DisplayRole, QVariant::fromValue(QString(repository)),
                                              1, Qt::MatchExactly);
    if (!itemList.isEmpty()) {
        auto itemIndex = itemList.at(0);
        QString removedRepository = itemIndex.data(Qt::DisplayRole).toString();
        model->removeRow(itemIndex.row(), itemIndex.parent());
        qDebug() << __PRETTY_FUNCTION__ << removedRepository;
        emit repositoryRemoved(removedRepository);
        return true;
    }
    return false;
}

bool RepositoryTreeWidget::removeRow(const QModelIndex &index)
{
    QModelIndex repositoryIndex = model->index(index.row(), 0);
    QString removedRepository = repositoryIndex.data(Qt::DisplayRole).toString();
    qDebug() << __PRETTY_FUNCTION__ << removedRepository;

    bool result = model->removeRow(index.row(), index.parent());
    emit repositoryRemoved(removedRepository);

    return result;
}

int RepositoryTreeWidget::rowCount() const
{
    return model->rowCount();
}

QWidget *RepositoryTreeWidget::createButtonBar()
{
        QWidget *mainWidget = new QWidget(parentWidget());
        QWidget *widgetButtonBar = new QWidget(parentWidget());

        QPushButton *buttonAdd = new QPushButton(this);
        buttonAdd->setIcon(QIcon::fromTheme("list-add"));
        buttonAdd->setMaximumSize(25, 25);
        connect(buttonAdd, &QPushButton::clicked, this, [&]() {
            addRepository(new OBSRepository);
            scrollToBottom();
            QModelIndex itemIndex = model->index(rowCount()-1, 0);
            setCurrentIndex(itemIndex);
        });

        QPushButton *buttonRemove = new QPushButton(this);
        buttonRemove->setIcon(QIcon::fromTheme("list-remove"));
        buttonRemove->setMaximumSize(25, 25);
        connect(buttonRemove, &QPushButton::clicked, this, [&]() {
            removeRow(currentIndex());
        });

        QHBoxLayout *layoutButtonBar = new QHBoxLayout();
        layoutButtonBar->setSpacing(0);
        layoutButtonBar->setContentsMargins(0, 0, 0, 0);
        layoutButtonBar->setAlignment(Qt::AlignLeft);
        layoutButtonBar->addWidget(buttonAdd);
        layoutButtonBar->addWidget(buttonRemove);
        widgetButtonBar->setLayout(layoutButtonBar);

        QVBoxLayout *layoutTab = new QVBoxLayout();
        layoutTab->setSpacing(0);
        layoutTab->setContentsMargins(0, 0, 0, 2);
        layoutTab->addWidget(this);
        layoutTab->addWidget(widgetButtonBar);
        mainWidget->setLayout(layoutTab);

        return mainWidget;
}

QList<OBSRepository *> RepositoryTreeWidget::getRepositories() const
{
    QList<OBSRepository *> repositoryList;

    int rows = rowCount();
    for (int i=0; i<rows; i++) {
        OBSRepository *repository = new OBSRepository();
        for (int j=0; j<3; j++) {
            QModelIndex index = model->index(i, j);
            switch (j) {
            case 0: {
                QString name = model->data(index, Qt::DisplayRole).toString();
                repository->setName(name);
                break;
            }
            case 1: {
                QString arch = model->data(index, Qt::DisplayRole).toString();
                repository->appendArch(arch);
                break;
            }
            case 2: {
                QString path = model->data(index, Qt::DisplayRole).toString();
                QStringList fullPath = path.split("/");
                if (fullPath.count()>1) {
                    repository->setProject(fullPath.at(0));
                    repository->setRepository(fullPath.at(1));
                }
                break;
            }
            }
        }
        repositoryList.append(repository);
    }

    return repositoryList;
}
