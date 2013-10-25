/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2013 Javier Llorente <javier@opensuse.org>
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

#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QDialog>
#include <QDebug>
#include <QCheckBox>
#include <QTimer>
#include <QSpinBox>

namespace Ui {
    class Configure;
}

class Configure : public QDialog
{
    Q_OBJECT

public:
    explicit Configure(QWidget *parent = 0);
    ~Configure();

    void startTimer(const int&);
    void setTimerValue(const int&);
    int getTimerValue();
    bool isTimerActive();
    void setCheckedTimerCheckbox(bool);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::Configure *ui;
    QTimer *timer;
    void createTimer();
};

#endif // CONFIGURE_H
