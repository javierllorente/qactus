/*
 * Copyright (C) 2015-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef REQUESTVIEWER_H
#define REQUESTVIEWER_H

#include <QDialog>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardItemModel>
#include <QSharedPointer>
#include "obs.h"
#include "syntaxhighlighter.h"
#include "utils.h"

namespace Ui {
class RequestViewer;
}

class RequestViewer : public QDialog
{
    Q_OBJECT

public:
    explicit RequestViewer(QWidget *parent = nullptr, OBS *obs = nullptr,
                                QSharedPointer<OBSRequest> request = nullptr);
    ~RequestViewer();

    void setDiff(const QString &diff);
    void showTabBuildResults(bool show);

signals:
    void changeRequest(const QString &id, const QString &comments, bool accepted);

private slots:
    void on_acceptPushButton_clicked();
    void on_declinePushButton_clicked();
    void slotRequestStatusFetched(QSharedPointer<OBSStatus> status);
    void onRequestDiffFetched(const QString &diff);
    void slotAddBuildResults(QSharedPointer<OBSResult> result);

private:
    Ui::RequestViewer *ui;
    OBS *m_obs;
    QSharedPointer<OBSRequest> m_request;
    QTextDocument *m_document;
    SyntaxHighlighter *m_syntaxHighlighter;
};

#endif // REQUESTVIEWER_H
