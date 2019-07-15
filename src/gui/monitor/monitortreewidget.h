/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef MONITORTREEWIDGET_H
#define MONITORTREEWIDGET_H

#include <QObject>
#include <QTreeWidget>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeWidgetItem>
#include "obs.h"
#include "obsresult.h"
#include <QDebug>
#include "utils.h"
#include "autotooltipdelegate.h"
#include "roweditor.h"

class MonitorTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    MonitorTreeWidget(QWidget *parent = 0);
    ~MonitorTreeWidget();
    void setOBS(OBS *obs);
    void getBuildStatus();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void obsUrlDropped(const QString &project, const QString &package);
    void updateStatusBar(QString message, bool progressBarHidden);
    void notifyChanged(bool change);

public slots:
    void addDroppedPackage(OBSResult *result);
    void finishedAddingPackages();
    void slotInsertStatus(OBSStatus *obsStatus, int row);
    void slotAddRow();
    void slotRemoveRow();
    void slotMarkAllRead();

private:
    QString droppedProject;
    QString droppedPackage;
    OBS *obs;
    void readSettings();
    void writeSettings();
    bool hasStatusChanged(const QString &oldStatus, const QString &newStatus);

private slots:
    void slotEditRow(QTreeWidgetItem *item, int column);
    void slotMarkRead(QTreeWidgetItem *item, int column);

};

#endif // MONITORTREEWIDGET_H
