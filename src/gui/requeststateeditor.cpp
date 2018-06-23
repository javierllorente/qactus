/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2018 Javier Llorente <javier@opensuse.org>
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

RequestStateEditor::RequestStateEditor(QWidget *parent, OBS *obs) :
    QDialog(parent),
    ui(new Ui::RequestStateEditor),
    mOBS(obs)
{
    ui->setupUi(this);
    ui->commentsTextBrowser->setFocus();
    ui->diffTextBrowser->setFocusPolicy(Qt::NoFocus);

    // Setup build results tree view
    QStandardItemModel *sourceModelBuildResults = new QStandardItemModel(ui->treeBuildResults);
    QStringList treeBuildResultsHeaders;
    treeBuildResultsHeaders << tr("Repository") << tr("Arch") << tr("Status");
    sourceModelBuildResults->setHorizontalHeaderLabels(treeBuildResultsHeaders);
    ui->treeBuildResults->setModel(sourceModelBuildResults);
    ui->treeBuildResults->setColumnWidth(0, 250);

    connect(this, SIGNAL(changeSubmitRequest(QString,QString,bool)), mOBS, SLOT(changeSubmitRequestSlot(QString,QString,bool)));
    connect(mOBS, SIGNAL(srStatus(QString)), this, SLOT(srStatusSlot(QString)));
    connect(mOBS, SIGNAL(srDiffFetched(QString)), this, SLOT(srDiffFetchedSlot(QString)));
    connect(mOBS, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(slotAddBuildResults(OBSResult*)));
}

RequestStateEditor::~RequestStateEditor()
{
    delete ui;
}

void RequestStateEditor::setRequestId(const QString& id)
{
    this->id = id;
    ui->requestIdLabelText->setText(id);
}

void RequestStateEditor::setRequester(const QString& requester)
{
    ui->requesterLabel->setText("Created by " + requester);
}

void RequestStateEditor::setSource(const QString& source)
{
    ui->sourceLabelText->setText(source);
}

void RequestStateEditor::setTarget(const QString& target)
{
    ui->targetLabelText->setText(target);
}

void RequestStateEditor::setDate(const QString& date)
{
    ui->dateLabelText->setText(date);
}

void RequestStateEditor::setDiff(const QString &diff)
{
    textDocument = new QTextDocument(this);
    textDocument->setPlainText(diff);
    syntaxHighlighter = new SyntaxHighlighter(textDocument);
    ui->diffTextBrowser->setDocument(textDocument);
}

void RequestStateEditor::on_acceptPushButton_clicked()
{
    qDebug() << "Accepting request" << id;
    QProgressDialog progress(tr("Accepting request..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    emit changeSubmitRequest(id, ui->commentsTextBrowser->toPlainText(), true);
}

void RequestStateEditor::on_declinePushButton_clicked()
{
    qDebug() << "Declining request..." << id;
    QProgressDialog progress(tr("Declining request..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    emit changeSubmitRequest(id, ui->commentsTextBrowser->toPlainText(), false);
}

void RequestStateEditor::srStatusSlot(const QString &status)
{
   qDebug() << "RequestStateEditor::srStatusSlot()" << status;
   QString errorStr = tr("Error changing SR!");
   if (status == "ok") {
       close();
   } else {
       QMessageBox::critical(this, errorStr, status, QMessageBox::Ok);
   }
}

void RequestStateEditor::srDiffFetchedSlot(const QString &diff)
{
    qDebug() << "RequestStateEditor::srDiffFetchedSlot()\n" << diff;
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

        QList<QStandardItem*> items;
        items << itemRepository << itemArch << itemBuildResult;
        model->appendRow(items);
    }
    delete obsResult;
    obsResult = nullptr;
}
