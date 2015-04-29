#include "requeststateeditor.h"
#include "ui_requeststateeditor.h"

RequestStateEditor::RequestStateEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestStateEditor)
{
    ui->setupUi(this);

    obsAccess = OBSaccess::getInstance();
}

RequestStateEditor::~RequestStateEditor()
{
    delete ui;
}

void RequestStateEditor::setRequestId(const QString& id)
{
    this->id = id;
    ui->requestIdLabel->setText("Request ID: " + this->id);
}

void RequestStateEditor::on_acceptPushButton_clicked()
{
    qDebug() << "accepting request" << this->id;
    obsAccess->acceptRequest(this->id, ui->commentsTextBrowser->toPlainText());
}

void RequestStateEditor::on_declinePushButton_clicked()
{
    qDebug() << "declining request" << this->id;
    obsAccess->declineRequest(this->id, ui->commentsTextBrowser->toPlainText());
}
