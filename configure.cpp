/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2015 Javier Llorente <javier@opensuse.org>
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

#include "configure.h"
#include "ui_configure.h"

Configure::Configure(QWidget *parent, OBS *obs) :
    QDialog(parent),
    ui(new Ui::Configure),
    mOBS(obs)
{
    ui->setupUi(this);

    createTimer();
    connect(timer, SIGNAL(timeout()), parent, SLOT(refreshView()));
    proxySettingsSetup();
}

Configure::~Configure()
{
    delete ui;
}

void Configure::setApiUrl(QString apiUrlStr)
{
    if (apiUrlStr.isEmpty()) {
        apiUrlStr = "https://api.opensuse.org/";
    }

    if (apiUrlStr.endsWith("/")) {
        apiUrlStr = apiUrlStr.left(apiUrlStr.length()-1);
    }

    setOBSApiUrl(apiUrlStr);
    ui->lineEditApiUrl->setText(apiUrlStr + "/");
}

void Configure::createTimer()
{
    timer = new QTimer(this);

    ui->spinBoxTimer->setMinimum(5);
    ui->spinBoxTimer->setMaximum(1440);
    ui->spinBoxTimer->setDisabled(true);

    connect(ui->checkBoxTimer, SIGNAL(toggled(bool)), ui->spinBoxTimer, SLOT(setEnabled(bool)));    
}

void Configure::proxySettingsSetup()
{
    connect(ui->checkBoxProxy, SIGNAL(toggled(bool)), ui->comboBoxProxyType, SLOT(setEnabled(bool)));
    connect(ui->checkBoxProxy, SIGNAL(toggled(bool)), ui->lineEditProxyServer, SLOT(setEnabled(bool)));
    connect(ui->checkBoxProxy, SIGNAL(toggled(bool)), ui->spinBoxProxyPort, SLOT(setEnabled(bool)));
    connect(ui->checkBoxProxy, SIGNAL(toggled(bool)), ui->lineEditProxyUsername, SLOT(setEnabled(bool)));
    connect(ui->checkBoxProxy, SIGNAL(toggled(bool)), ui->lineEditProxyPassword, SLOT(setEnabled(bool)));

    ui->comboBoxProxyType->addItem("Socks 5");
    ui->comboBoxProxyType->addItem("HTTP");
}
void Configure::startTimer(const int& interval)
{
    if (interval >= 5) {
        qDebug() << "startTimer()";
//      Convert mins into msecs and start timer
        timer->start(interval*60000);
    } else {
        qDebug() << "Error starting timer: Wrong timer interval (smaller than 5)";
    }
}

void Configure::on_buttonBox_accepted()
{   
    if (ui->lineEditApiUrl->text() != mOBS->getApiUrl() + "/") {
        qDebug() << "Api has been changed!";
        emit apiChanged();
    }
    setApiUrl(ui->lineEditApiUrl->text());

    if (ui->checkBoxTimer->isChecked()) {
//      Start the timer if the checkbox is checked
        startTimer(ui->spinBoxTimer->value());
        qDebug() << "Timer set to" << ui->spinBoxTimer->value() << "minutes";

    } else if (timer->isActive()) {
        timer->stop();
        qDebug() << "The timer has been stopped";
    }

    toggleProxy(ui->checkBoxProxy->isChecked());

}

void Configure::on_buttonBox_rejected()
{
    ui->lineEditApiUrl->setText(mOBS->getApiUrl() + "/");

    ui->checkBoxTimer->setChecked(timer->isActive());
    ui->spinBoxTimer->setValue(timer->interval()/60000);

    bool check = proxy.applicationProxy() != QNetworkProxy::NoProxy;
    ui->checkBoxProxy->setChecked(check);

    // Restore previous values only if checkBoxProxy is checked
    if (check) {
        int index = proxy.type() == QNetworkProxy::Socks5Proxy ? 0 : 1;
        ui->comboBoxProxyType->setCurrentIndex(index);
        ui->spinBoxProxyPort->setValue(proxy.port());
        ui->lineEditProxyServer->setText(proxy.hostName());
        ui->lineEditProxyUsername->setText(proxy.user());
        ui->lineEditProxyPassword->setText(proxy.password());
    }
}

void Configure::setOBSApiUrl(const QString &apiUrlStr)
{
    mOBS->setApiUrl(apiUrlStr);
}

bool Configure::isTimerActive()
{
    return timer->isActive();
}

int Configure::getTimerValue()
{
    return ui->spinBoxTimer->value();
}

void Configure::setTimerValue(const int& value)
{
    ui->spinBoxTimer->setValue(value);
    qDebug() << "Timer value:" << ui->spinBoxTimer->value() << "minutes";
}

void Configure::setCheckedTimerCheckbox(bool check)
{
    ui->checkBoxTimer->setChecked(check);
}

void Configure::toggleProxy(bool enableProxy)
{
    if (enableProxy) {
        qDebug() << "Proxy has been enabled";
        QNetworkProxy::ProxyType proxyType = ui->comboBoxProxyType->currentIndex() == 0 ?
                    QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy;
        proxy.setType(proxyType);
        proxy.setHostName(ui->lineEditProxyServer->text());
        proxy.setPort(ui->spinBoxProxyPort->text().toInt());
        proxy.setUser(ui->lineEditProxyUsername->text());
        proxy.setPassword(ui->lineEditProxyPassword->text());
        QNetworkProxy::setApplicationProxy(proxy);
    } else {
        qDebug() << "Proxy has been disabled";
        proxy.setApplicationProxy(QNetworkProxy::NoProxy);
    }
}

bool Configure::isProxyEnabled()
{
    return ui->checkBoxProxy->isChecked();
}

void Configure::setCheckedProxyCheckbox(bool check)
{
    ui->checkBoxProxy->setChecked(check);
}

void Configure::setProxyType(const int &proxyType)
{
    ui->comboBoxProxyType->setCurrentIndex(proxyType == 1 ? 0 : 1);
}

int Configure::getProxyType() const
{
    QNetworkProxy::ProxyType proxyType = ui->comboBoxProxyType->currentIndex() == 0 ?
                QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy;
    return proxyType;
}

void Configure::setProxyServer(const QString &proxyServer)
{
    ui->lineEditProxyServer->setText(proxyServer);
}

QString Configure::getProxyServer() const
{
    return ui->lineEditProxyServer->text();
}

void Configure::setProxyPort(const int &proxyPort)
{
    ui->spinBoxProxyPort->setValue(proxyPort);
}

int Configure::getProxyPort() const
{
    return ui->spinBoxProxyPort->value();
}

void Configure::setProxyUsername(const QString &proxyUsername)
{
    ui->lineEditProxyUsername->setText(proxyUsername);
}

QString Configure::getProxyUsername() const
{
    return ui->lineEditProxyUsername->text();
}

void Configure::setProxyPassword(const QString &proxyPassword)
{
    ui->lineEditProxyPassword->setText(proxyPassword);
}

QString Configure::getProxyPassword() const
{
    return ui->lineEditProxyPassword->text();
}
