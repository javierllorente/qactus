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

#ifndef REQUESTBOXTREEWIDGET_H
#define REQUESTBOXTREEWIDGET_H

#include <QObject>
#include <QTreeWidget>
#include <QDebug>

class RequestBoxTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit RequestBoxTreeWidget(QWidget *parent = nullptr);

signals:
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    void requestTypeChanged(int index);

private:
    int oldIndex;

};

#endif // REQUESTBOXTREEWIDGET_H
