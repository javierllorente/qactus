/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#include "searchwidget.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(QWidget *parent, QPlainTextEdit *plainTextEdit) :
    QWidget(parent),
    ui(new Ui::SearchWidget),
    m_plainTextEdit(plainTextEdit)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

#if QT_VERSION >= 0x050200
    ui->lineEdit->setClearButtonEnabled(true);
#endif

    QShortcut *findNextAction = new QShortcut(QKeySequence("F3"), this);
    QShortcut *findPreviousAction = new QShortcut(QKeySequence("Shift+F3"), this);

    connect(findNextAction, SIGNAL(activated()), this, SLOT(findNext()));
    connect(findPreviousAction, SIGNAL(activated()), this, SLOT(findPrevious()));
    connect(ui->pushButtonNext, SIGNAL(clicked(bool)), this, SLOT(findNext()));
    connect(ui->pushButtonPrevious, SIGNAL(clicked(bool)), this, SLOT(findPrevious()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(clearAll(QString)));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));
    connect(ui->pushButtonHighlight, SIGNAL(toggled(bool)), this, SLOT(highlightText(bool)));
    connect(ui->pushButtonClose, SIGNAL(clicked(bool)), this, SLOT(clearAll()));
    connect(ui->pushButtonClose, SIGNAL(clicked(bool)), this, SLOT(close()));

    QTextCursor cursor = m_plainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_plainTextEdit->setTextCursor(cursor);

    logHighlighter = nullptr;
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

void SearchWidget::setup()
{
    ui->lineEdit->selectAll();
    ui->lineEdit->setFocus();
}

void SearchWidget::findNext()
{
    findText(ui->lineEdit->text(), QTextDocument::FindFlags());
}

void SearchWidget::findPrevious()
{
    findText(ui->lineEdit->text(), QTextDocument::FindBackward);
}

void SearchWidget::findText(const QString &text, QTextDocument::FindFlags findFlags)
{
    bool found = m_plainTextEdit->find(text, findFlags);
    highlightText(ui->pushButtonHighlight->isChecked());

    if (text.isEmpty()) {
        found = true;
    }
    setLineEditStyle(found);
}

void SearchWidget::setLineEditStyle(bool found)
{
    QString styleSheet = found ? "" : "QLineEdit { background-color: #ff6666 }";
    ui->lineEdit->setStyleSheet(styleSheet);
}

void SearchWidget::clearHighlight()
{
    if (logHighlighter) {
        delete logHighlighter;
        logHighlighter = nullptr;
        lastSearch = "";
    }
}

void SearchWidget::clearSelection()
{
    QTextCursor cursor = m_plainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::EndOfWord);
    m_plainTextEdit->setTextCursor(cursor);
    ui->lineEdit->setStyleSheet("");
}

void SearchWidget::highlightText(bool check)
{
    if (check) {
        if (!logHighlighter) {
            logHighlighter = new LogHighlighter(m_plainTextEdit->document(), ui->lineEdit->text());
            lastSearch = ui->lineEdit->text();
        } else {
            if (ui->lineEdit->text()!=lastSearch) {
                delete logHighlighter;
                logHighlighter = new LogHighlighter(m_plainTextEdit->document(), ui->lineEdit->text());
                lastSearch = ui->lineEdit->text();
            }
        }
    } else {
        clearHighlight();
    }
}

void SearchWidget::clearAll()
{
    clearHighlight();
    clearSelection();
}

void SearchWidget::clearAll(const QString &text)
{
    if (text.isEmpty()) {
        clearAll();
    }
}

void SearchWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        clearAll();
        close();
    }
}
