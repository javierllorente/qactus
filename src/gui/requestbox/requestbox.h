/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019-2020 Javier Llorente <javier@opensuse.org>
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

#ifndef REQUESTBOX_H
#define REQUESTBOX_H

#include <QWidget>
#include "obs.h"
#include "obsrequest.h"

namespace Ui {
class RequestBox;
}

class RequestBox : public QWidget
{
    Q_OBJECT

public:
    explicit RequestBox(QWidget *parent = nullptr, OBS *obs = nullptr);
    ~RequestBox();

private:
    Ui::RequestBox *ui;
    OBS *m_obs;
    void readSettings();
    void writeSettings();

signals:
    void updateStatusBar(const QString &message, bool progressBarHidden);
    void descriptionFetched(const QString &description);

public slots:
    void changeRequestState();

private slots:
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    void slotRequestStatusFetched(const QString &status);

};

#endif // REQUESTBOX_H
