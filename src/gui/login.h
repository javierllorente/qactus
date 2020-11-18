/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2020 Javier Llorente <javier@opensuse.org>
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
    void slotCredentialsRestored(const QString &username, const QString &password);

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
