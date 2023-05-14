/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019-2023 Javier Llorente <javier@opensuse.org>
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

#include "requestbox.h"
#include "ui_requestbox.h"
#include "requeststateeditor.h"
#include <QSettings>

RequestBox::RequestBox(QWidget *parent, OBS *obs) :
    QWidget(parent),
    ui(new Ui::RequestBox),
    m_obs(obs)
{
    ui->setupUi(this);

    ui->horizontalSplitter->setSizes((QList<int>({100, 500})));
    ui->verticalSplitter->setSizes((QList<int>({240, 400})));

    connect(ui->treeRequests, &RequestTreeWidget::descriptionFetched, this, &RequestBox::descriptionFetched);
    connect(ui->treeRequests, &RequestTreeWidget::changeRequestState, this, &RequestBox::changeRequestState);
    connect(ui->treeRequests, &RequestTreeWidget::descriptionFetched, ui->textBrowser, &QTextBrowser::setText);
    connect(ui->treeRequests, &RequestTreeWidget::updateStatusBar, this, &RequestBox::updateStatusBar);

    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::requestTypeChanged, ui->treeRequests, &RequestTreeWidget::requestTypeChanged);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getIncomingRequests, this, &RequestBox::getIncomingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getOutgoingRequests, this, &RequestBox::getOutgoingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getDeclinedRequests, this, &RequestBox::getDeclinedRequests);

    connect(m_obs, &OBS::finishedParsingIncomingRequest, ui->treeRequests, &RequestTreeWidget::addIncomingRequest);
    connect(m_obs, &OBS::finishedParsingIncomingRequestList, ui->treeRequests, &RequestTreeWidget::irListFetched);
    connect(m_obs, &OBS::finishedParsingOutgoingRequest, ui->treeRequests, &RequestTreeWidget::addOutgoingRequest);
    connect(m_obs, &OBS::finishedParsingOutgoingRequestList, ui->treeRequests, &RequestTreeWidget::orListFetched);
    connect(m_obs, &OBS::finishedParsingDeclinedRequest, ui->treeRequests, &RequestTreeWidget::addDeclinedRequest);
    connect(m_obs, &OBS::finishedParsingDeclinedRequestList, ui->treeRequests, &RequestTreeWidget::orListFetched);
    connect(m_obs, &OBS::finishedParsingRequestStatus, this, &RequestBox::slotRequestStatusFetched);

    readSettings();
}

RequestBox::~RequestBox()
{
    writeSettings();
    delete ui;
}

void RequestBox::readSettings()
{
    QSettings settings;
    settings.beginGroup("RequestBox");
    ui->horizontalSplitter->restoreState(settings.value("horizontalSplitterSizes").toByteArray());
    ui->verticalSplitter->restoreState(settings.value("verticalSplitterSizes").toByteArray());
    settings.endGroup();
}

void RequestBox::writeSettings()
{
    QSettings settings;
    settings.beginGroup("RequestBox");
    settings.setValue("horizontalSplitterSizes", ui->horizontalSplitter->saveState());
    settings.setValue("verticalSplitterSizes", ui->verticalSplitter->saveState());
    settings.endGroup();
}

void RequestBox::changeRequestState()
{
    qDebug() << __PRETTY_FUNCTION__;
    OBSRequest *request = ui->treeRequests->currentRequest();
    RequestStateEditor *reqStateEditor = new RequestStateEditor(this, m_obs, request);

    reqStateEditor->exec();

    delete reqStateEditor;
    delete request;
}

void RequestBox::getIncomingRequests()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->textBrowser->clear();
    m_obs->getIncomingRequests();
}

void RequestBox::getOutgoingRequests()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->textBrowser->clear();
    m_obs->getOutgoingRequests();
}

void RequestBox::getDeclinedRequests()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->textBrowser->clear();
    m_obs->getDeclinedRequests();
}

void RequestBox::slotRequestStatusFetched(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (status->getCode()=="ok") {
        OBSRequest *request = ui->treeRequests->currentRequest();
        ui->treeRequests->removeIncomingRequest(request->getId());
        ui->textBrowser->clear();
        delete request;
    }
}
