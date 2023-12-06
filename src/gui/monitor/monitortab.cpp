/*
 * Copyright (C) 2021 Javier Llorente <javier@opensuse.org>
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
