/*
 * Copyright (C) 2018-2025 Javier Llorente <javier@opensuse.org>
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
    void linkFetched(QSharedPointer<OBSLink> link);
    void autocompletedProject_activated(const QString &project);
    void autocompletedPackage_activated(const QString &package);
    void toggleOkButton();
    void on_buttonBox_accepted();
};

#endif // CREATEREQUESTDIALOG_H
