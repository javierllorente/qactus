/*
 * Copyright (C) 2025 Javier Llorente <javier@opensuse.org>
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
#include "requestswidget.h"
#include "ui_requestswidget.h"
#include "requestviewer.h"
#include <QSettings>

RequestsWidget::RequestsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RequestsWidget),
    firstTimeRevisionListDisplayed(true),
    obs(nullptr)
{
    ui->setupUi(this);

    ui->verticalSplitter->setSizes((QList<int>({240, 400})));

    connect(ui->requestTreeWidget, &RequestTreeWidget::descriptionFetched, this, &RequestsWidget::descriptionFetched);
    connect(ui->requestTreeWidget, &RequestTreeWidget::changeRequestState, this, &RequestsWidget::changeRequestState);
    connect(ui->requestTreeWidget, &RequestTreeWidget::descriptionFetched, ui->textBrowser, &QTextBrowser::setText);
    connect(ui->requestTreeWidget, &RequestTreeWidget::updateStatusBar, this, &RequestsWidget::updateStatusBar);

    itemModel = new RequestItemModel(this);
    setModel(itemModel);
}

RequestsWidget::~RequestsWidget()
{
    delete ui;
}

void RequestsWidget::setModel(QAbstractItemModel *model)
{
    QItemSelectionModel *m = ui->requestTreeWidget->selectionModel();
    ui->requestTreeWidget->setModel(model);
    // Keep description column hidden
    ui->requestTreeWidget->setColumnHidden(7, true);
    delete m;
}

void RequestsWidget::setOBS(OBS *obs)
{
    this->obs = obs;
}

QSharedPointer<OBSRequest> RequestsWidget::getCurrentRequest()
{
    return ui->requestTreeWidget->getCurrentRequest();
}

void RequestsWidget::clearModel()
{
    itemModel->clear();
    ui->requestTreeWidget->selectionModel()->clearSelection();
    m_dataLoaded = false;
}

void RequestsWidget::clearDescription()
{
    ui->textBrowser->clear();
}

void RequestsWidget::readSettings()
{
    QSettings settings;
    settings.beginGroup("RequestsWidget");
    ui->verticalSplitter->restoreState(settings.value("verticalSplitterSizes").toByteArray());
    settings.endGroup();
}

void RequestsWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("RequestsWidget");
    settings.setValue("verticalSplitterSizes", ui->verticalSplitter->saveState());
    settings.endGroup();
}

void RequestsWidget::addRequest(QSharedPointer<OBSRequest> request)
{
    itemModel->appendRequest(request);
}

void RequestsWidget::requestsAdded(const QString &project, const QString &package)
{
    if (firstTimeRevisionListDisplayed) {
        ui->requestTreeWidget->model()->sort(0, Qt::DescendingOrder);
        ui->requestTreeWidget->header()->setSortIndicator(0, Qt::DescendingOrder);
        firstTimeRevisionListDisplayed = false;
    } else {
        ui->requestTreeWidget->model()->sort(logicalIndex, order);
        ui->requestTreeWidget->header()->setSortIndicator(logicalIndex, order);
    }
    logicalIndex = ui->requestTreeWidget->header()->sortIndicatorSection();
    order = ui->requestTreeWidget->header()->sortIndicatorOrder();

    ui->requestTreeWidget->selectionModel()->clear(); // Emits selectionChanged() and currentChanged()
    this->project = project;
    this->package = package;
    m_dataLoaded = true;
    emit updateStatusBar(tr("Done"), true);
}

void RequestsWidget::changeRequestState()
{
    qDebug() << Q_FUNC_INFO;
    QSharedPointer<OBSRequest> request = ui->requestTreeWidget->getCurrentRequest();
    QScopedPointer<RequestViewer> requestViewer(new RequestViewer(this, obs, request));
    requestViewer->exec();
}
