/********************************************************************************
** Form generated from reading UI file 'configure.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGURE_H
#define UI_CONFIGURE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_Configure
{
public:
    QDialogButtonBox *buttonBox;
    QCheckBox *checkBox_Timer;
    QLabel *label;
    QSpinBox *spinBox;

    void setupUi(QDialog *Configure)
    {
        if (Configure->objectName().isEmpty())
            Configure->setObjectName(QString::fromUtf8("Configure"));
        Configure->setWindowModality(Qt::ApplicationModal);
        Configure->resize(400, 300);
        buttonBox = new QDialogButtonBox(Configure);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        checkBox_Timer = new QCheckBox(Configure);
        checkBox_Timer->setObjectName(QString::fromUtf8("checkBox_Timer"));
        checkBox_Timer->setGeometry(QRect(20, 110, 121, 21));
        label = new QLabel(Configure);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 140, 201, 16));
        spinBox = new QSpinBox(Configure);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));
        spinBox->setGeometry(QRect(170, 140, 81, 24));

        retranslateUi(Configure);
        QObject::connect(buttonBox, SIGNAL(accepted()), Configure, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Configure, SLOT(reject()));

        QMetaObject::connectSlotsByName(Configure);
    } // setupUi

    void retranslateUi(QDialog *Configure)
    {
        Configure->setWindowTitle(QApplication::translate("Configure", "Configure Qactus", 0, QApplication::UnicodeUTF8));
        checkBox_Timer->setText(QApplication::translate("Configure", "Enable timer", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Configure", "Check for changes every", 0, QApplication::UnicodeUTF8));
        spinBox->setSuffix(QApplication::translate("Configure", " Min", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Configure: public Ui_Configure {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGURE_H
