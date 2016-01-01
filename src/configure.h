/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013, 2015 Javier Llorente <javier@opensuse.org>
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

#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QDialog>
#include <QDebug>
#include <QCheckBox>
#include <QTimer>
#include <QSpinBox>
#include <QNetworkProxy>
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
    void startTimer(const int&);
    void setTimerValue(const int&);
    int getTimerValue();
    bool isTimerActive();
    void setCheckedTimerCheckbox(bool check);
    void toggleProxy(bool enableProxy);
    bool isProxyEnabled();
    void setCheckedProxyCheckbox(bool check);
    void setProxyType(const int &proxyType);
    int getProxyType() const;
    void setProxyServer(const QString &proxyServer);
    QString getProxyServer() const;
    void setProxyPort(const int &proxyPort);
    int getProxyPort() const;
    void setProxyUsername(const QString &proxyUsername);
    QString getProxyUsername() const;
    void setProxyPassword(const QString &proxyPassword);
    QString getProxyPassword() const;

signals:
    void apiChanged();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::Configure *ui;
    OBS *mOBS;
    void setOBSApiUrl(const QString &apiUrlStr);
    QTimer *timer;
    void createTimer();
    void proxySettingsSetup();
    QNetworkProxy proxy;
};

#endif // CONFIGURE_H
