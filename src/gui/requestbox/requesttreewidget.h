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
#ifndef REQUESTTREEWIDGET_H
#define REQUESTTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QSharedPointer>
#include "obsrequest.h"

class RequestTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit RequestTreeWidget(QWidget *parent = nullptr);
    QSharedPointer<OBSRequest> currentRequest();

signals:
    void updateStatusBar(const QString &message, bool progressBarHidden);
    void descriptionFetched(const QString &description);
    void changeRequestState();

private:
    QMenu *m_menu;

private slots:
    void onContextMenuRequested(const QPoint &point);

};

#endif // REQUESTTREEWIDGET_H
