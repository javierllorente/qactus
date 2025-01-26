/*
 * Copyright (C) 2021-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef MONITORPACKAGESTAB_H
#define MONITORPACKAGESTAB_H

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QSharedPointer>
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
    void slotInsertStatus(QSharedPointer<OBSStatus> status, int row);
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
