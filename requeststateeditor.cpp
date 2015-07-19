#include "requeststateeditor.h"
#include "ui_requeststateeditor.h"

RequestStateEditor::RequestStateEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestStateEditor)
{
    ui->setupUi(this);
    ui->commentsTextBrowser->setFocus();
    ui->diffTextBrowser->setFocusPolicy(Qt::NoFocus);
    obsAccess = OBSAccess::getInstance();
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
    result = obsAccess->acceptRequest(id, ui->commentsTextBrowser->toPlainText());
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
    result = obsAccess->declineRequest(id, ui->commentsTextBrowser->toPlainText());
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
