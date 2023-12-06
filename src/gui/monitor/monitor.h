/*
 * Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
