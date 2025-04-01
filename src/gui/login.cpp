/*
 * Copyright (C) 2013-2025 Javier Llorente <javier@opensuse.org>
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
#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    setTabOrder(ui->lineEdit_Username, ui->lineEdit_Password);
    setTabOrder(ui->lineEdit_Password, ui->buttonBox);

    readSettings();
}

Login::~Login()
{
    qDebug() << Q_FUNC_INFO << "Deleting Login";
    delete ui;
}

void Login::closeEvent(QCloseEvent *event)
{
    qDebug() << Q_FUNC_INFO << "Closing Login";
    event->accept();
}

QString Login::getUsername()
{
    return ui->lineEdit_Username->text();
}

void Login::setUsername(const QString& username)
{
    ui->lineEdit_Username->setText(username);
    if (!username.isEmpty()) {
        ui->lineEdit_Password->setFocus();
    }
}

QString Login::getPassword()
{
    return ui->lineEdit_Password->text();
}

void Login::readSettings()
{
    qDebug() << Q_FUNC_INFO;
    QSettings settings;
    settings.beginGroup("Auth");

    QString apiUrl = settings.value("ApiUrl").toString();
    QStringList apiUrlSplitted = apiUrl.split("https://");
    if (apiUrl.size()>1) {
        apiUrl = apiUrlSplitted.at(1);
    }

    QString username = settings.value("Username").toString();
    setUsername(username);
    Credentials *credentials = new Credentials();
    connect(credentials, &Credentials::credentialsRestored,
            this, &Login::onCredentialsRestored);
    credentials->readPassword(username);
    delete credentials;
    qDebug() << Q_FUNC_INFO << "AutoLogin:" << settings.value("AutoLogin").toBool();
    settings.endGroup();
}

void Login::writeSettings()
{
    qDebug() << Q_FUNC_INFO;
    QSettings settings;
    settings.beginGroup("Auth");
    settings.setValue("Username", getUsername());
    Credentials *credentials = new Credentials();
    credentials->writeCredentials(getUsername(), getPassword());
    delete credentials;
    settings.endGroup();
}

void Login::configureMode()
{
    ui->buttonBox->hide();
}

void Login::clearCredentials()
{
    ui->lineEdit_Username->clear();
    ui->lineEdit_Password->clear();
    ui->lineEdit_Username->setFocus();
}

void Login::on_buttonBox_accepted()
{
    if (getUsername().isEmpty() || getPassword().isEmpty()) {
        QMessageBox::warning(parentWidget(), tr("Error"), tr("Empty username/password"), QMessageBox::Ok);
    } else {
        writeSettings();
        emit login(getUsername(), getPassword());
        close();
    }
}

void Login::onCredentialsRestored(const QString &/*username*/, const QString &password)
{
    qDebug() << Q_FUNC_INFO;
    ui->lineEdit_Password->setText(password);
}
