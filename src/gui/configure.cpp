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
#include "configure.h"
#include "ui_configure.h"
#include "credentials.h"

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

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        ui->listWidget->item(i)->setSizeHint(QSize(115, 70));
    }

    ui->listWidget->setMaximumWidth(ui->listWidget->sizeHintForColumn(0)+4);
    ui->listWidget->setDragEnabled(false);

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    ui->listWidget->setCurrentRow(0);

    readAuthSettings();
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

    writeAuthSettings();
    writeProxySettings();

    settings.beginGroup("Timer");
    settings.setValue("Active", ui->checkBoxTimer->isChecked());
    settings.setValue("Value", ui->spinBoxTimer->value());
    settings.endGroup();

    settings.beginGroup("Browser");
    settings.setValue("Homepage", ui->lineEditHomepage->text());
    settings.setValue("IncludeHomeProjects", ui->checkBoxHomeProjects->isChecked());
    settings.endGroup();
}

void Configure::writeAuthSettings()
{
    qDebug() << __PRETTY_FUNCTION__;
    QSettings settings;
    settings.beginGroup("Auth");
    settings.setValue("ApiUrl", mOBS->getApiUrl());
    settings.setValue("Username", ui->lineEditUsername->text());
    Credentials *credentials = new Credentials();
    credentials->writeCredentials(ui->lineEditUsername->text(), ui->lineEditPassword->text());
    delete credentials;
    settings.setValue("AutoLogin", ui->checkBoxAutoLogin->isChecked());
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

    settings.beginGroup("Browser");
    homepage = settings.value("Homepage").toString();
    ui->lineEditHomepage->setText(homepage);
    includeHomeProjects = settings.value("IncludeHomeProjects").toBool();
    ui->checkBoxHomeProjects->setChecked(includeHomeProjects);
    settings.endGroup();
}

void Configure::readAuthSettings()
{
    qDebug() << "Configure::readAuthSettings()";
    QSettings settings;

    settings.beginGroup("Auth");
    setApiUrl(settings.value("ApiUrl").toString());

    QString username = settings.value("Username").toString();
    ui->lineEditUsername->setText(username);
    Credentials *credentials = new Credentials();
    connect(credentials, &Credentials::credentialsRestored,
            [&](const QString &/*username*/, const QString &password) {
        ui->lineEditPassword->setText(password);
    });
    credentials->readPassword(username);
    delete credentials;
    ui->checkBoxAutoLogin->setChecked((settings.value("AutoLogin", true).toBool()));

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

    writeAuthSettings();
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
