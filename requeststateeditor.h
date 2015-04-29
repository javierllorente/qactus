#ifndef REQUESTSTATEEDITOR_H
#define REQUESTSTATEEDITOR_H

#include <QDialog>
#include <QDebug>
#include "obsaccess.h"

namespace Ui {
class RequestStateEditor;
}

class RequestStateEditor : public QDialog
{
    Q_OBJECT

public:
    explicit RequestStateEditor(QWidget *parent = 0);
    ~RequestStateEditor();

    void setRequestId(const QString& id);

private slots:
    void on_acceptPushButton_clicked();
    void on_declinePushButton_clicked();

private:
    Ui::RequestStateEditor *ui;
    OBSaccess *obsAccess;
    QString id;
};

#endif // REQUESTSTATEEDITOR_H
