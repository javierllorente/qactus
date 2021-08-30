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

#ifndef MONITOR_H
#define MONITOR_H

#include <QWidget>
#include "obs.h"
#include "monitortab.h"

namespace Ui {
class Monitor;
}

class Monitor : public QWidget
{
    Q_OBJECT

public:
    explicit Monitor(QWidget *parent = nullptr, OBS *obs = nullptr);
    ~Monitor();
    bool hasPackageSelection();
    void refresh();
    bool tabWidgetContains(const QString &tabText);
    int addTab(const QString &title);

private:
    Ui::Monitor *ui;
    OBS *m_obs;
    void readSettings();
    void writeSettings();
    void setupTabConnections(MonitorTab *tab);
    void setupPackagesTab();
    void setupConnections(MonitorTab *monitorTab);

signals:
    void addDroppedPackage(OBSResult *result);
    void addRow();
    void itemSelectionChanged();
    void currentTabChanged(int index);
    void removeRow();
    void markAllRead();
    void notifyChanged(bool change);
    void updateStatusBar(QString message, bool progressBarHidden);

private slots:
    void closeTab(int index);
};

#endif // MONITOR_H
