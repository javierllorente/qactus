/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#ifndef CREATEDIALOG_H
#define CREATEDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <QPushButton>
#include "obs.h"
#include "obsxmlwriter.h"

namespace Ui {
class CreateDialog;
}

class CreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDialog(OBS *obs, QWidget *parent = 0);
    ~CreateDialog();
    void setProjectMode(bool projectMode);
    void setProject(const QString &project);

signals:
    void createProject(QString, QByteArray);
    void createPackage(QString, QString, QByteArray);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void slotCreateResult(OBSStatus *obsStatus);

    void on_projectLineEdit_textChanged(const QString &project);

    void on_packageLineEdit_textChanged(const QString &package);

private:
    Ui::CreateDialog *ui;
    OBS *mOBS;
    bool projectMode;
};

#endif // CREATEDIALOG_H
