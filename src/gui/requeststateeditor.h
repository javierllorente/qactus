/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2017 Javier Llorente <javier@opensuse.org>
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

#ifndef REQUESTSTATEEDITOR_H
#define REQUESTSTATEEDITOR_H

#include <QDialog>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include "obs.h"

namespace Ui {
class RequestStateEditor;
}

class RequestStateEditor : public QDialog
{
    Q_OBJECT

public:
    explicit RequestStateEditor(QWidget *parent = 0, OBS *obs = 0);
    ~RequestStateEditor();

    void setRequestId(const QString& id);
    void setSource(const QString& source);
    void setRequester(const QString& requester);
    void setTarget(const QString& target);
    void setDate(const QString& date);
    void setDiff(const QString &diff);

signals:
    void changeSubmitRequest(const QString &id, const QString &comments, bool accepted);

private slots:
    void on_acceptPushButton_clicked();
    void on_declinePushButton_clicked();
    void srStatusSlot(const QString &status);
    void srDiffFetchedSlot(const QString &diff);

private:
    Ui::RequestStateEditor *ui;
    OBS *mOBS;
    QString id;
};

#endif // REQUESTSTATEEDITOR_H
