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

#ifndef MONITORREPOSITORYTAB_H
#define MONITORREPOSITORYTAB_H

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "obs.h"
#include "obsresult.h"
#include <QDebug>
#include "monitortab.h"

class MonitorRepositoryTab : public MonitorTab
{
    Q_OBJECT

public:
    explicit MonitorRepositoryTab(QWidget *parent = nullptr, const QString &title = "untitled", OBS *obs = nullptr);
    virtual ~MonitorRepositoryTab();
    void refresh();
    bool hasSelection();

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void notifyChanged(bool change);

public slots:
    void slotAddResultList(const QList<OBSResult *> &resultList);

private:
    void checkForResultListChanges(const QList<OBSResult *> &resultList);
    QList<OBSResult *> m_resultList;

};

#endif // MONITORREPOSITORYTAB_H
