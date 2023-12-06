/*
 * Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
#ifndef PACKAGEACTIONDIALOG_H
#define PACKAGEACTIONDIALOG_H

#include <QDialog>
#include "obs.h"

namespace Ui {
class PackageActionDialog;
}

enum class PackageAction {
    Unknown = -1,
    LinkPackage,
    CopyPackage
};

class PackageActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PackageActionDialog(QWidget *parent = nullptr, OBS *obs = nullptr,
                                 const QString &srcProject = QString(), const QString &srcPackage = QString(),
                                 PackageAction action = PackageAction::Unknown);
    ~PackageActionDialog();

public slots:
    void addProjectList(const QStringList &projectList);

private:
    Ui::PackageActionDialog *ui;
    OBS *m_obs;
    PackageAction m_action;


private slots:
    void toggleOkButton();
    void on_buttonBox_accepted();
    void revisionFetched(OBSRevision *revision);
    void slotCannot(OBSStatus *status);

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void showTrayMessage(const QString &title, const QString &message);
};

#endif // PACKAGEACTIONDIALOG_H
