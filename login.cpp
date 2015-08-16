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

#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    setTabOrder(ui->lineEdit_Username, ui->lineEdit_Password);
    setTabOrder(ui->lineEdit_Password, ui->checkBox_AutoLogin);
    setTabOrder(ui->checkBox_AutoLogin, ui->pushButton_Login);

    connect(ui->pushButton_Login, SIGNAL(clicked()), parent, SLOT(pushButton_Login_clicked()) );
    connect(ui->lineEdit_Password, SIGNAL(returnPressed()), parent, SLOT(lineEdit_Password_returnPressed()));
}

Login::~Login()
{
    delete ui;
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

bool Login::isAutoLoginEnabled()
{
    return ui->checkBox_AutoLogin->isChecked();
}

void Login::setAutoLoginEnabled(bool check)
{
    ui->checkBox_AutoLogin->setChecked(check);
}

void Login::clearCredentials()
{
    ui->lineEdit_Username->clear();
    ui->lineEdit_Password->clear();
    ui->lineEdit_Username->setFocus();
}

