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

#ifndef MONITORTAB_H
#define MONITORTAB_H

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

namespace Ui {
class MonitorTab;
}

class MonitorTab : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorTab(QWidget *parent = nullptr, const QString &title = "untitled", OBS *obs = nullptr);
    ~MonitorTab();
    virtual void refresh();
    virtual bool hasSelection();

protected:
    bool hasStatusChanged(const QString &oldStatus, const QString &newStatus);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    Ui::MonitorTab *ui;
    QString m_title;
    OBS *m_obs;

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void notifyChanged(bool change);
    void itemSelectionChanged();

public slots:
    void slotMarkAllRead();

private slots:
    void slotMarkRead(QTreeWidgetItem *item, int column);

};

#endif // MONITORTAB_H
