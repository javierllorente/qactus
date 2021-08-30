/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2021 Javier Llorente <javier@opensuse.org>
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

#include "monitortab.h"
#include "ui_monitortab.h"

MonitorTab::MonitorTab(QWidget *parent, const QString &title, OBS *obs) :
    QWidget(parent),
    ui(new Ui::MonitorTab),
    m_title(title),
    m_obs(obs)
{
    ui->setupUi(this);

    ui->treeWidget->setColumnCount(5);
    ui->treeWidget->setColumnWidth(0, 200); // Project
    ui->treeWidget->setColumnWidth(1, 200); // Package
    ui->treeWidget->setColumnWidth(2, 210); // Repository
    ui->treeWidget->setColumnWidth(3, 75); // Arch
    ui->treeWidget->setColumnWidth(4, 100); // Status

    ui->treeWidget->setItemDelegate(new AutoToolTipDelegate(this));

    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MonitorTab::slotMarkRead);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MonitorTab::itemSelectionChanged);
}

MonitorTab::~MonitorTab()
{

}

void MonitorTab::refresh()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_obs->getProjectResults(m_title);
}

bool MonitorTab::hasSelection()
{
    QItemSelectionModel *treeWidgetSelectionModel = ui->treeWidget->selectionModel();
    if (treeWidgetSelectionModel) {
        return treeWidgetSelectionModel->hasSelection();
    } else {
        return false;
    }
}

void MonitorTab::slotMarkAllRead()
{
    qDebug() << __PRETTY_FUNCTION__;
    for (int i=0; i<ui->treeWidget->topLevelItemCount(); i++) {
        if (ui->treeWidget->topLevelItem(i)->font(0).bold()) {
            Utils::setItemBoldFont(ui->treeWidget->topLevelItem(i), false);
        }
    }

    emit notifyChanged(false);
}

bool MonitorTab::hasStatusChanged(const QString &oldStatus, const QString &newStatus)
{
    qDebug() << __PRETTY_FUNCTION__
             << "Old status:" << oldStatus << "New status:" << newStatus;
    bool change = false;
    if (!oldStatus.isEmpty() && oldStatus != newStatus) {
        change = true;
        qDebug() << __PRETTY_FUNCTION__ << change;
        emit notifyChanged(change);
    }
    return change;
}

void MonitorTab::slotMarkRead(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    qDebug() << __PRETTY_FUNCTION__ << "Row: " + QString::number(ui->treeWidget->indexOfTopLevelItem(item));
    for (int i=0; i<ui->treeWidget->columnCount(); i++) {
        if (item->font(0).bold()) {
            Utils::setItemBoldFont(item, false);
        }
    }

    emit notifyChanged(false);
}

void MonitorTab::dragEnterEvent(QDragEnterEvent *event)
{

}

void MonitorTab::dragMoveEvent(QDragMoveEvent *event)
{

}

void MonitorTab::dropEvent(QDropEvent *event)
{

}
