/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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

#include "monitor.h"
#include "ui_monitor.h"

Monitor::Monitor(QWidget *parent, OBS *obs) :
    QWidget(parent),
    ui(new Ui::Monitor),
    m_obs(obs)
{
    ui->setupUi(this);


    connect(m_obs, &OBS::finishedParsingPackage, ui->treeMonitor, &MonitorTreeWidget::slotInsertStatus);
    connect(ui->treeMonitor, &MonitorTreeWidget::obsUrlDropped, m_obs, &OBS::getAllBuildStatus);
    connect(m_obs, &OBS::finishedParsingResult, this, &Monitor::addDroppedPackage);
    connect(this, &Monitor::addDroppedPackage, ui->treeMonitor, &MonitorTreeWidget::addDroppedPackage);
    connect(m_obs, &OBS::finishedParsingResultList, ui->treeMonitor, &MonitorTreeWidget::finishedAddingPackages);

    connect(this, &Monitor::addRow, ui->treeMonitor, &MonitorTreeWidget::slotAddRow);
    connect(ui->treeMonitor, &MonitorTreeWidget::itemSelectionChanged, this, &Monitor::itemSelectionChanged);
    connect(this, &Monitor::removeRow, ui->treeMonitor, &MonitorTreeWidget::slotRemoveRow);
    connect(this, &Monitor::markAllRead, ui->treeMonitor, &MonitorTreeWidget::slotMarkAllRead);
    connect(ui->treeMonitor, &MonitorTreeWidget::notifyChanged, this, &Monitor::notifyChanged);
    connect(ui->treeMonitor, &MonitorTreeWidget::updateStatusBar, this, &Monitor::updateStatusBar);

    ui->treeMonitor->setOBS(m_obs);
}

Monitor::~Monitor()
{
    delete ui;
}

bool Monitor::hasSelection()
{
    QItemSelectionModel *treeMonitorSelectionModel = ui->treeMonitor->selectionModel();
    if (treeMonitorSelectionModel) {
        return treeMonitorSelectionModel->hasSelection();
    } else {
        return false;
    }
}

void Monitor::getBuildStatus()
{
    ui->treeMonitor->getBuildStatus();
}
