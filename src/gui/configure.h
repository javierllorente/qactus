/*
 * Copyright (C) 2013-2024 Javier Llorente <javier@opensuse.org>
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
#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QDialog>
#include <QDebug>
#include <QCheckBox>
#include <QSpinBox>
#include <QNetworkProxy>
#include <QSettings>
#include "obs.h"

namespace Ui {
    class Configure;
}

class Configure : public QDialog
{
    Q_OBJECT

public:
    explicit Configure(QWidget *parent = 0, OBS *obs = 0);
    ~Configure();

    void setApiUrl(QString apiUrlStr);
    void toggleProxy(bool enableProxy);
    bool isProxyEnabled();
    void setCheckedProxyCheckbox(bool check);
    void readSettings();

signals:
    void apiChanged();
    void proxyChanged();
    void includeHomeProjectsChanged();
    void timerChanged();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::Configure *ui;
    OBS *mOBS;
    void setOBSApiUrl(const QString &apiUrlStr);
    void readAuthSettings();
    void readProxySettings();
    void readTimerSettings();
    void proxySettingsSetup();
    QNetworkProxy proxy;
    QString homepage;
    bool includeHomeProjects;
    enum ProxyType { NoProxy, SystemProxy, ManualProxy };
    void writeSettings();
    void writeAuthSettings();
    void writeProxySettings();
};

#endif // CONFIGURE_H
