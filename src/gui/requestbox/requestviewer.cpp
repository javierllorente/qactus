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
#include "requestviewer.h"
#include "ui_requestviewer.h"

RequestViewer::RequestViewer(QWidget *parent, OBS *obs, QSharedPointer<OBSRequest> request) :
    QDialog(parent),
    ui(new Ui::RequestViewer),
    m_obs(obs),
    m_request(request),
    m_document(new QTextDocument(this)),
    m_syntaxHighlighter(new SyntaxHighlighter(m_document))
{
    ui->setupUi(this);

    setWindowTitle("Request " + m_request->getId());
    ui->commentsTextBrowser->setFocus();
    showTabBuildResults(false);

    // Fill in the fields
    ui->requestIdLabelText->setText(m_request->getId());
    ui->requesterLabel->setText("Created by " + m_request->getRequester());
    ui->sourceLabelText->setText(m_request->getSource());
    ui->targetLabelText->setText(m_request->getTarget());
    ui->dateLabelText->setText(m_request->getDate());

    connect(this, &RequestViewer::changeSubmitRequest, m_obs, &OBS::slotChangeSubmitRequest);
    connect(m_obs, &OBS::finishedParsingRequestStatus, this, &RequestViewer::slotRequestStatusFetched);
    connect(m_obs, &OBS::srDiffFetched, this, &RequestViewer::slotSrDiffFetched);
    connect(m_obs, &OBS::finishedParsingResult, this, &RequestViewer::slotAddBuildResults);

    if (m_request->getActionType()=="submit") {
        // Get SR diff
        m_obs->getRequestDiff(m_request->getId());

        // Setup build results tree view
        QStandardItemModel *sourceModelBuildResults = new QStandardItemModel(ui->treeBuildResults);
        QStringList treeBuildResultsHeaders = {tr("Repository"), tr("Arch"), tr("Status")};
        sourceModelBuildResults->setHorizontalHeaderLabels(treeBuildResultsHeaders);
        ui->treeBuildResults->setModel(sourceModelBuildResults);
        ui->treeBuildResults->setColumnWidth(0, 250);

        // Get package build results
        showTabBuildResults(true);
        m_obs->getAllBuildStatus(m_request->getSourceProject(), m_request->getSourcePackage());
    } else {
        setDiff(m_request->getActionType() + " " + m_request->getTarget());
    }

    bool writeMode = m_request->getState()=="new";
    ui->acceptPushButton->setEnabled(writeMode);
    ui->declinePushButton->setEnabled(writeMode);
    ui->commentsTextBrowser->setEnabled(writeMode);
}

RequestViewer::~RequestViewer()
{
    delete ui;
}

void RequestViewer::setDiff(const QString &diff)
{
    m_document->setPlainText(diff);
    ui->diffTextBrowser->setDocument(m_document);
}

void RequestViewer::showTabBuildResults(bool show)
{
    if (show) {
        ui->tabWidget->addTab(ui->tabBuildResults, tr("Build results"));
    } else {
        ui->tabWidget->removeTab(1);
    }
}

void RequestViewer::on_acceptPushButton_clicked()
{
    qDebug() << "Accepting request" << m_request->getId();
    QProgressDialog progress(tr("Accepting request..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    emit changeSubmitRequest(m_request->getId(), ui->commentsTextBrowser->toPlainText(), true);
}

void RequestViewer::on_declinePushButton_clicked()
{
    qDebug() << "Declining request..." << m_request->getId();
    QProgressDialog progress(tr("Declining request..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    emit changeSubmitRequest(m_request->getId(), ui->commentsTextBrowser->toPlainText(), false);
}

void RequestViewer::slotRequestStatusFetched(QSharedPointer<OBSStatus> status)
{
   qDebug() << __PRETTY_FUNCTION__ << status->getCode();
   QString errorStr = tr("Error changing SR!");
   if (status->getCode() == "ok") {
       close();
   } else {
       QMessageBox::critical(this, errorStr, status->getCode(), QMessageBox::Ok);
   }
}

void RequestViewer::slotSrDiffFetched(const QString &diff)
{
    qDebug() << Q_FUNC_INFO;
    setDiff(diff);
}

void RequestViewer::slotAddBuildResults(QSharedPointer<OBSResult> result)
{
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->treeBuildResults->model());
    if (model) {
        QStandardItem *itemRepository = new QStandardItem(result->getRepository());
        QStandardItem *itemArch = new QStandardItem(result->getArch());
        QStandardItem *itemBuildResult = new QStandardItem(result->getStatus()->getCode());
        itemBuildResult->setForeground(Utils::getColorForStatus(itemBuildResult->text()));

        if (!result->getStatus()->getDetails().isEmpty()) {
            QString details = result->getStatus()->getDetails();
            details = Utils::breakLine(details, 250);
            itemBuildResult->setToolTip(details);
        }

        QList<QStandardItem *> items = {itemRepository, itemArch, itemBuildResult};
        model->appendRow(items);
    }
}
