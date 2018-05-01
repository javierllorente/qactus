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

    includeHomeProjects = false;
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

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        ui->listWidget->item(i)->setSizeHint(QSize(97,60));
    }

    ui->listWidget->setMaximumWidth(ui->listWidget->sizeHintForColumn(0)+4);
    ui->listWidget->setDragEnabled(false);

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    ui->listWidget->setCurrentRow(0);

    readProxySettings();
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
    connect(ui->radioButtonManualProxy, SIGNAL(toggled(bool)), ui->comboBoxProxyType, SLOT(setEnabled(bool)));
    connect(ui->radioButtonManualProxy, SIGNAL(toggled(bool)), ui->lineEditProxyServer, SLOT(setEnabled(bool)));
    connect(ui->radioButtonManualProxy, SIGNAL(toggled(bool)), ui->spinBoxProxyPort, SLOT(setEnabled(bool)));
    connect(ui->radioButtonManualProxy, SIGNAL(toggled(bool)), ui->lineEditProxyUsername, SLOT(setEnabled(bool)));
    connect(ui->radioButtonManualProxy, SIGNAL(toggled(bool)), ui->lineEditProxyPassword, SLOT(setEnabled(bool)));

    ui->buttonGroupProxyType->setId(ui->radioButtonNoProxy, 0);
    ui->buttonGroupProxyType->setId(ui->radioButtonSystemProxy, 1);
    ui->buttonGroupProxyType->setId(ui->radioButtonManualProxy, 2);
    ui->comboBoxProxyType->addItem("Socks 5");
    ui->comboBoxProxyType->addItem("HTTP");
}

void Configure::writeSettings()
{
    qDebug() << "Configure::writeSettings()";
    QSettings settings;

    writeProxySettings();

    settings.beginGroup("Auth");
    settings.setValue("ApiUrl", mOBS->getApiUrl());
    settings.endGroup();

    settings.beginGroup("Timer");
    settings.setValue("Active", ui->checkBoxTimer->isChecked());
    settings.setValue("Value", ui->spinBoxTimer->value());
    settings.endGroup();

    settings.beginGroup("Browser");
    settings.setValue("IncludeHomeProjects", ui->checkBoxHomeProjects->isChecked());
    settings.endGroup();
}

void Configure::writeProxySettings()
{
    qDebug() << "Configure::writeProxySettings()";

    QSettings settings;
    settings.beginGroup("Proxy");

    int checkedId = ui->buttonGroupProxyType->checkedId();

    switch (checkedId) {
    case NoProxy:
        settings.setValue("Type", QNetworkProxy::NoProxy);
        break;
    case SystemProxy:
        settings.setValue("Type", QNetworkProxy::DefaultProxy);
        break;
    case ManualProxy: {
        int proxyType = ui->comboBoxProxyType->currentIndex() == 0 ?
                    QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy;
        settings.setValue("Type", proxyType);
        settings.setValue("Server", ui->lineEditProxyServer->text());
        settings.setValue("Port", ui->spinBoxProxyPort->text());
        settings.setValue("Username", ui->lineEditProxyUsername->text());
        settings.setValue("Password", ui->lineEditProxyPassword->text());
        break;
    }
    default:
        break;
    }

    settings.endGroup();
}

void Configure::readSettings()
{
    qDebug() << "Configure::readSettings()";
    QSettings settings;

    settings.beginGroup("Auth");
    setApiUrl(settings.value("ApiUrl").toString());
    settings.endGroup();

    settings.beginGroup("Browser");
    ui->checkBoxHomeProjects->setChecked(settings.value("IncludeHomeProjects").toBool());
    settings.endGroup();
}

void Configure::readProxySettings()
{
    qDebug() << "Configure::readProxySettings()";

    QSettings settings;
    settings.beginGroup("Proxy");

    int proxyType = settings.value("Type").toInt();
    bool manualProxy = false;

    switch (proxyType) {
    case QNetworkProxy::NoProxy:
        ui->buttonGroupProxyType->button(ProxyType::NoProxy)->setChecked(true);
        break;
    case QNetworkProxy::DefaultProxy:
        ui->buttonGroupProxyType->button(ProxyType::SystemProxy)->setChecked(true);
        break;
    case QNetworkProxy::Socks5Proxy:
        ui->comboBoxProxyType->setCurrentIndex(0);
        manualProxy = true;
        break;
    case QNetworkProxy::HttpProxy:
        ui->comboBoxProxyType->setCurrentIndex(1);
        manualProxy = true;
        break;
    default:
        break;
    }

    ui->buttonGroupProxyType->button(ProxyType::ManualProxy)->setChecked(manualProxy);
    ui->comboBoxProxyType->setEnabled(manualProxy);
    ui->lineEditProxyServer->setText(settings.value("Server").toString());
    ui->spinBoxProxyPort->setValue(settings.value("Port").toInt());
    ui->lineEditProxyUsername->setText(settings.value("Username").toString());
    ui->lineEditProxyPassword->setText(settings.value("Password").toString());
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

    writeSettings();
    emit proxyChanged();

    if (includeHomeProjects!=ui->checkBoxHomeProjects->isChecked()) {
        emit includeHomeProjectsChanged();
    }
    includeHomeProjects = ui->checkBoxHomeProjects->isChecked();

    login->writeSettings();
    emit timerChanged();
}

void Configure::on_buttonBox_rejected()
{
    // Don't do anything
}

void Configure::setOBSApiUrl(const QString &apiUrlStr)
{
    mOBS->setApiUrl(apiUrlStr);
}
