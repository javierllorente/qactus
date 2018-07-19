/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2018 Javier Llorente <javier@opensuse.org>
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
#include <QTreeWidget>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeWidgetItem>
#include "obs.h"
#include "obsresult.h"
#include <QDebug>
#include "utils.h"

class PackageTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    PackageTreeWidget(QWidget *parent = 0);
    void setOBS(OBS *obs);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void obsUrlDropped(const QString &project, const QString &package);

public slots:
    void addDroppedPackage(OBSResult *result);
    void finishedAddingPackages();

private:
    QString droppedProject;
    QString droppedPackage;

};

#endif // PACKAGETREEWIDGET_H
