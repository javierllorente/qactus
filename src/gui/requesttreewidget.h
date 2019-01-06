/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef REQUESTTREEWIDGET_H
#define REQUESTTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include "obsrequest.h"
#include "requestitemmodel.h"
#include "requeststateeditor.h"
#include "autotooltipdelegate.h"

class RequestTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit RequestTreeWidget(QWidget *parent = nullptr);
    OBSRequest *currentRequest();
    int getRequestType() const;

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void descriptionFetched(const QString &description);
    void changeRequestState();

public slots:
    void addIncomingRequest(OBSRequest *request);
    void irListFetched();
    void addOutgoingRequest(OBSRequest *request);
    void orListFetched();
    bool removeIncomingRequest(const QString &id);
    bool removeOutgoingRequest(const QString &id);
    void requestTypeChanged(int index);

private:
     RequestItemModel *irModel;
     RequestItemModel *orModel;
     QMenu *m_menu;
     int m_requestType;

private slots:
    void slotContextMenuRequests(const QPoint &point);

};

#endif // REQUESTTREEWIDGET_H
