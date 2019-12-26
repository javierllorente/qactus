/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
