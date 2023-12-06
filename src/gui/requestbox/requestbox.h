/*
 * Copyright (C) 2019-2020 Javier Llorente <javier@opensuse.org>
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
    void slotRequestStatusFetched(OBSStatus *status);

};

#endif // REQUESTBOX_H
