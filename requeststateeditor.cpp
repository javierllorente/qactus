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

void RequestStateEditor::setDiff(const QString& diff)
{
    ui->diffTextBrowser->setText(diff);

}

void RequestStateEditor::on_acceptPushButton_clicked()
{
    qDebug() << "Accepting request" << this->id;
    QProgressDialog progress(tr("Accepting request..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    result = obsAccess->acceptRequest(this->id, ui->commentsTextBrowser->toPlainText());
    if (result=="ok") {
        close();
    } else {
        QMessageBox::critical(this, tr("Error accepting request!"), result, QMessageBox::Ok );
    }
    qDebug() << "Accepting result:" << result;
}

void RequestStateEditor::on_declinePushButton_clicked()
{
    qDebug() << "Declining request..." << this->id;
    QProgressDialog progress(tr("Declining request..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    result = obsAccess->declineRequest(this->id, ui->commentsTextBrowser->toPlainText());
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
