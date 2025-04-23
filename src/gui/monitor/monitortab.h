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
    virtual ~MonitorTab();
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
