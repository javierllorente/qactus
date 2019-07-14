/*
 *  Qactus - A Qt-based OBS client
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

#ifndef CREATEREQUESTDIALOG_H
#define CREATEREQUESTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QStringListModel>
#include <QCompleter>
#include <QDebug>
#include "obsxmlwriter.h"
#include "obsrequest.h"
#include "obs.h"
#include "obsstatus.h"

namespace Ui {
class CreateRequestDialog;
}

class CreateRequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateRequestDialog(OBSRequest *request = nullptr, OBS *obs = nullptr, QWidget *parent = nullptr);
    ~CreateRequestDialog();

signals:
    void createRequest(QByteArray data);

public slots:
    void addProjectList(const QStringList &projectList);
    void addPackageList(const QStringList &packageList);

private:
    Ui::CreateRequestDialog *ui;
    OBSRequest *m_request;
    OBS *m_obs;

    QStringListModel *m_projectModel;
    QCompleter *m_projectCompleter;

    QStringListModel *m_packageModel;
    QCompleter *m_packageCompleter;

private slots:
    void linkFetched(OBSLink *link);
    void autocompletedProject_activated(const QString &project);
    void autocompletedPackage_activated(const QString &package);
    void toggleOkButton();
    void on_buttonBox_accepted();
};

#endif // CREATEREQUESTDIALOG_H
