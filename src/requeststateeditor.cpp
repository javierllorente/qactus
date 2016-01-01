/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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

void RequestStateEditor::setDiff(const QString& diff)
{
    ui->diffTextBrowser->setText(diff);

}

void RequestStateEditor::on_acceptPushButton_clicked()
{
    qDebug() << "Accepting request" << id;
    QProgressDialog progress(tr("Accepting request..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    result = mOBS->acceptRequest(id, ui->commentsTextBrowser->toPlainText());
    if (result=="ok") {
        close();
    } else {
        QMessageBox::critical(this, tr("Error accepting request!"), result, QMessageBox::Ok );
    }
    qDebug() << "Accepting result:" << result;
}

void RequestStateEditor::on_declinePushButton_clicked()
{
    qDebug() << "Declining request..." << id;
    QProgressDialog progress(tr("Declining request..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    result = mOBS->declineRequest(id, ui->commentsTextBrowser->toPlainText());
    if (result=="ok") {
        close();
    } else {
        QMessageBox::critical(this, tr("Error declining request!"), result, QMessageBox::Ok );
    }
    qDebug() << "Declining result:" << result;
}

QString RequestStateEditor::getResult()
{
    return result;
}
