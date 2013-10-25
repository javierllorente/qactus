/********************************************************************************
** Form generated from reading UI file 'login.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Login
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLineEdit *lineEdit_Username;
    QLabel *label_Password;
    QLineEdit *lineEdit_Password;
    QLabel *label_Username;
    QPushButton *pushButton_Login;

    void setupUi(QDialog *Login)
    {
        if (Login->objectName().isEmpty())
            Login->setObjectName(QString::fromUtf8("Login"));
        Login->setWindowModality(Qt::ApplicationModal);
        Login->resize(299, 149);
        gridLayoutWidget = new QWidget(Login);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 30, 281, 71));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        lineEdit_Username = new QLineEdit(gridLayoutWidget);
        lineEdit_Username->setObjectName(QString::fromUtf8("lineEdit_Username"));

        gridLayout->addWidget(lineEdit_Username, 0, 1, 1, 1);

        label_Password = new QLabel(gridLayoutWidget);
        label_Password->setObjectName(QString::fromUtf8("label_Password"));

        gridLayout->addWidget(label_Password, 1, 0, 1, 1);

        lineEdit_Password = new QLineEdit(gridLayoutWidget);
        lineEdit_Password->setObjectName(QString::fromUtf8("lineEdit_Password"));
        lineEdit_Password->setInputMethodHints(Qt::ImhHiddenText|Qt::ImhNoAutoUppercase|Qt::ImhNoPredictiveText);
        lineEdit_Password->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(lineEdit_Password, 1, 1, 1, 1);

        label_Username = new QLabel(gridLayoutWidget);
        label_Username->setObjectName(QString::fromUtf8("label_Username"));

        gridLayout->addWidget(label_Username, 0, 0, 1, 1);

        pushButton_Login = new QPushButton(Login);
        pushButton_Login->setObjectName(QString::fromUtf8("pushButton_Login"));
        pushButton_Login->setGeometry(QRect(200, 120, 91, 24));

        retranslateUi(Login);

        QMetaObject::connectSlotsByName(Login);
    } // setupUi

    void retranslateUi(QDialog *Login)
    {
        Login->setWindowTitle(QApplication::translate("Login", "Login", 0, QApplication::UnicodeUTF8));
        label_Password->setText(QApplication::translate("Login", "Password:", 0, QApplication::UnicodeUTF8));
        label_Username->setText(QApplication::translate("Login", "Username:", 0, QApplication::UnicodeUTF8));
        pushButton_Login->setText(QApplication::translate("Login", "Login", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Login: public Ui_Login {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGIN_H
