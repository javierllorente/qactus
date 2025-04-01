/*
 * Copyright (C) 2013-2020 Javier Llorente <javier@opensuse.org>
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
#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include "credentials.h"

namespace Ui {
    class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    void closeEvent(QCloseEvent *event);
    void configureMode();
    void writeSettings();

public slots:
    void clearCredentials();

private slots:
    void on_buttonBox_accepted();
    void onCredentialsRestored(const QString &username, const QString &password);

signals:
    void login(const QString &username, const QString &password);

private:
    Ui::Login *ui;
    QString getUsername();
    void setUsername(const QString&);
    QString getPassword();
    bool isAutoLoginEnabled();
    void setAutoLoginEnabled(bool check);
    void readSettings();

};


#endif // LOGIN_H
