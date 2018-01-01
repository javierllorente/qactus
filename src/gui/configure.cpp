/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2018 Javier Llorente <javier@opensuse.org>
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

    ui->spinBoxTimer->setMinimum(5);
    ui->spinBoxTimer->setMaximum(1440);
    ui->spinBoxTimer->setDisabled(true);
    connect(ui->checkBoxTimer, SIGNAL(toggled(bool)), ui->spinBoxTimer, SLOT(setEnabled(bool)));

    ui->checkBoxHomeProjects->setChecked(includeHomeProjects);
    proxySettingsSetup();

//    loginDialog = new Login(ui->authPage);
//    ui->stackedWidget->insertWidget(0, loginDialog);

    login = new Login();
    login->configureMode();
    QListWidgetItem *loginItem = new QListWidgetItem();
    loginItem->setText("Authentication");
    loginItem->setIcon(QIcon(":/icons/dialog-password.png"));

    // Note: Ownership of widget is passed onto the stacked/list widget
    ui->stackedWidget->insertWidget(0, login);
    ui->listWidget->insertItem(0, loginItem);

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    ui->listWidget->setCurrentRow(0);

    readSettings();
    readTimerSettings();
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

void Configure::writeSettings()
{
    qDebug() << "Configure::writeSettings()";
    QSettings settings;

    settings.beginGroup("Proxy");
    settings.setValue("Enabled", isProxyEnabled());
    settings.setValue("Type", getProxyType());
    settings.setValue("Server", getProxyServer());
    settings.setValue("Port", getProxyPort());
    settings.setValue("Username", getProxyUsername());
    settings.setValue("Password", getProxyPassword());
    settings.endGroup();

    settings.beginGroup("Auth");
    settings.setValue("ApiUrl", mOBS->getApiUrl());
    settings.endGroup();

    settings.beginGroup("Timer");
    settings.setValue("Active", ui->checkBoxTimer->isChecked());
    settings.setValue("Value", ui->spinBoxTimer->value());
    settings.endGroup();

    settings.beginGroup("Browser");
    settings.setValue("IncludeHomeProjects", isIncludeHomeProjects());
    settings.endGroup();
}

void Configure::readSettings()
{
    qDebug() << "Configure::readSettings()";
    QSettings settings;

    settings.beginGroup("Proxy");
    if (settings.value("Enabled").toBool()) {
        setCheckedProxyCheckbox(true);
        setProxyType(settings.value("Type").toInt());
        setProxyServer(settings.value("Server").toString());
        setProxyPort(settings.value("Port").toInt());
        setProxyUsername(settings.value("Username").toString());
        setProxyPassword(settings.value("Password").toString());
        // FIX-ME: If proxy is enabled on a non-proxy environment you have
        // to edit Qactus.conf and set Enabled=false to get Qactus to log in
    }
    settings.endGroup();

    settings.beginGroup("Auth");
    setApiUrl(settings.value("ApiUrl").toString());
    settings.endGroup();

    settings.beginGroup("Browser");
    setIncludeHomeProjects(settings.value("IncludeHomeProjects").toBool());
    settings.endGroup();
}

void Configure::readTimerSettings()
{
    QSettings settings;
    settings.beginGroup("Timer");
    ui->checkBoxTimer->setChecked(settings.value("Active").toBool());
    ui->spinBoxTimer->setValue(settings.value("Value").toInt());
    settings.endGroup();
}

void Configure::on_buttonBox_accepted()
{   
    if (ui->lineEditApiUrl->text() != mOBS->getApiUrl() + "/") {
        qDebug() << "Api has been changed!";
        emit apiChanged();
    }
    setApiUrl(ui->lineEditApiUrl->text());

    toggleProxy(ui->checkBoxProxy->isChecked());

    if (includeHomeProjects!=ui->checkBoxHomeProjects->isChecked()) {
        emit includeHomeProjectsChanged();
    }
    includeHomeProjects = ui->checkBoxHomeProjects->isChecked();

    writeSettings();
    login->writeSettings();
    emit timerChanged();
}

void Configure::on_buttonBox_rejected()
{
    ui->lineEditApiUrl->setText(mOBS->getApiUrl() + "/");
    readTimerSettings();

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

    ui->checkBoxHomeProjects->setChecked(includeHomeProjects);
}

void Configure::setOBSApiUrl(const QString &apiUrlStr)
{
    mOBS->setApiUrl(apiUrlStr);
}

void Configure::toggleProxy(bool enableProxy)
{
    qDebug() << "Configure::toggleProxy()" << enableProxy;
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

bool Configure::isIncludeHomeProjects()
{
    return includeHomeProjects;
}

void Configure::setIncludeHomeProjects(bool check)
{
    includeHomeProjects = check;
}
