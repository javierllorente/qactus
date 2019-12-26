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

class PackageActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PackageActionDialog(QWidget *parent = nullptr, OBS *obs = nullptr, const QString &sourcePrj = QString(), const QString &sourcePkg = QString());
    ~PackageActionDialog();

public slots:
    void addProjectList(const QStringList &projectList);

private:
    Ui::PackageActionDialog *ui;
    OBS *m_obs;

private slots:
    void toggleOkButton();
    void on_buttonBox_accepted();
    void revisionFetched(OBSRevision *revision);
    void slotCannotCopyPackage(OBSStatus *status);

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void showTrayMessage(const QString &title, const QString &message);
};

#endif // PACKAGEACTIONDIALOG_H
