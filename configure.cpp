/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2014 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
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

Configure::Configure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Configure)
{
    ui->setupUi(this);

    createTimer();
    connect(timer, SIGNAL(timeout()), parent, SLOT(refreshView()));
}

Configure::~Configure()
{
    delete ui;
}

void Configure::createTimer()
{
    timer = new QTimer(this);

    ui->spinBox->setMinimum(5);
    ui->spinBox->setMaximum(1440);
    ui->spinBox->setDisabled(true);

    connect(ui->checkBox_Timer, SIGNAL(toggled(bool)), ui->spinBox, SLOT(setEnabled(bool)));

}
void Configure::startTimer(const int& interval)
{
    qDebug() << "startTimer()";
    timer->start(interval);
}

void Configure::on_buttonBox_accepted()
{
    if (ui->checkBox_Timer->isChecked()) {
//        Start the timer if the checkbox is checked
//        and convert mins into msecs
        timer->start(ui->spinBox->value()*60000);
        qDebug() << "Timer set to" << ui->spinBox->value() << "minutes";

    } else if (timer->isActive()) {
        timer->stop();
        qDebug() << "The timer has been stopped";
    }
}

void Configure::on_buttonBox_rejected()
{
    ui->checkBox_Timer->setChecked(timer->isActive());
}

bool Configure::isTimerActive()
{
    return timer->isActive();
}

int Configure::getTimerValue()
{
    return ui->spinBox->value();
}

void Configure::setTimerValue(const int& value)
{
    ui->spinBox->setValue(value);
    qDebug() << "Timer value:" << ui->spinBox->value() << "minutes";
}

void Configure::setCheckedTimerCheckbox(bool check)
{
    ui->checkBox_Timer->setChecked(check);
}
