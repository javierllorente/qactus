/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2015-2020 Javier Llorente <javier@opensuse.org>
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

#include "requeststateeditor.h"
#include "ui_requeststateeditor.h"

RequestStateEditor::RequestStateEditor(QWidget *parent, OBS *obs, OBSRequest *request) :
    QDialog(parent),
    ui(new Ui::RequestStateEditor),
    m_obs(obs),
    m_request(request),
    m_document(new QTextDocument(this)),
    m_syntaxHighlighter(new SyntaxHighlighter(m_document))
{
    ui->setupUi(this);

    ui->commentsTextBrowser->setFocus();
    showTabBuildResults(false);

    // Fill in the fields
    ui->requestIdLabelText->setText(m_request->getId());
    ui->requesterLabel->setText("Created by " + m_request->getRequester());
    ui->sourceLabelText->setText(m_request->getSource());
    ui->targetLabelText->setText(m_request->getTarget());
    ui->dateLabelText->setText(m_request->getDate());

    connect(this, SIGNAL(changeSubmitRequest(QString,QString,bool)), m_obs, SLOT(slotChangeSubmitRequest(QString,QString,bool)));
    connect(m_obs, &OBS::finishedParsingRequestStatus, this, &RequestStateEditor::slotRequestStatusFetched);
    connect(m_obs, SIGNAL(srDiffFetched(QString)), this, SLOT(slotSrDiffFetched(QString)));
    connect(m_obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(slotAddBuildResults(OBSResult*)));

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

RequestStateEditor::~RequestStateEditor()
{
    delete ui;
}

void RequestStateEditor::setDiff(const QString &diff)
{
    m_document->setPlainText(diff);
    ui->diffTextBrowser->setDocument(m_document);
}

void RequestStateEditor::showTabBuildResults(bool show)
{
    if (show) {
        ui->tabWidget->addTab(ui->tabBuildResults, tr("Build results"));
    } else {
        ui->tabWidget->removeTab(1);
    }
}

void RequestStateEditor::on_acceptPushButton_clicked()
{
    qDebug() << "Accepting request" << m_request->getId();
    QProgressDialog progress(tr("Accepting request..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    emit changeSubmitRequest(m_request->getId(), ui->commentsTextBrowser->toPlainText(), true);
}

void RequestStateEditor::on_declinePushButton_clicked()
{
    qDebug() << "Declining request..." << m_request->getId();
    QProgressDialog progress(tr("Declining request..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    emit changeSubmitRequest(m_request->getId(), ui->commentsTextBrowser->toPlainText(), false);
}

void RequestStateEditor::slotRequestStatusFetched(OBSStatus *status)
{
   qDebug() << __PRETTY_FUNCTION__ << status->getCode();
   QString errorStr = tr("Error changing SR!");
   if (status->getCode() == "ok") {
       close();
   } else {
       QMessageBox::critical(this, errorStr, status->getCode(), QMessageBox::Ok);
   }
   delete status;
}

void RequestStateEditor::slotSrDiffFetched(const QString &diff)
{
    qDebug() << "RequestStateEditor::slotSrDiffFetched()\n" << diff;
    setDiff(diff);
}

void RequestStateEditor::slotAddBuildResults(OBSResult *obsResult)
{
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->treeBuildResults->model());
    if (model) {
        QStandardItem *itemRepository = new QStandardItem(obsResult->getRepository());
        QStandardItem *itemArch = new QStandardItem(obsResult->getArch());
        QStandardItem *itemBuildResult = new QStandardItem(obsResult->getStatus()->getCode());
        itemBuildResult->setForeground(Utils::getColorForStatus(itemBuildResult->text()));

        if (!obsResult->getStatus()->getDetails().isEmpty()) {
            QString details = obsResult->getStatus()->getDetails();
            details = Utils::breakLine(details, 250);
            itemBuildResult->setToolTip(details);
        }

        QList<QStandardItem *> items = {itemRepository, itemArch, itemBuildResult};
        model->appendRow(items);
    }
    delete obsResult;
    obsResult = nullptr;
}
