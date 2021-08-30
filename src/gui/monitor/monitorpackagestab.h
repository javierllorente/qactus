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

#ifndef MONITORPACKAGESTAB_H
#define MONITORPACKAGESTAB_H

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>
#include "obs.h"
#include "obsresult.h"
#include "monitortab.h"

class MonitorPackagesTab : public MonitorTab
{
    Q_OBJECT

public:
    explicit MonitorPackagesTab(QWidget *parent = nullptr, const QString &title = "untitled", OBS *obs = nullptr);
    virtual ~MonitorPackagesTab();
    void refresh();
    bool hasSelection();

signals:
    void obsUrlDropped(const QString &project, const QString &package);
    void updateStatusBar(QString message, bool progressBarHidden);
    void notifyChanged(bool change);
    void itemSelectionChanged();

public slots:
    void addDroppedPackage(OBSResult *result);
    void finishedAddingPackages();
    void slotInsertStatus(OBSStatus *obsStatus, int row);
    void slotAddRow();
    void slotRemoveRow();

private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    QString droppedProject;
    QString droppedPackage;
    void readSettings();
    void writeSettings();

private slots:
    void slotEditRow(QTreeWidgetItem *item, int column);
    void slotMarkRead(QTreeWidgetItem *item, int column);

};

#endif // MONITORPACKAGESTAB_H
