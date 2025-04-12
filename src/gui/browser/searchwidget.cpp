/*
 * Copyright (C) 2018-2025 Javier Llorente <javier@opensuse.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

    ui->lineEdit->setClearButtonEnabled(true);

    QShortcut *findNextAction = new QShortcut(QKeySequence("F3"), this);
    QShortcut *findPreviousAction = new QShortcut(QKeySequence("Shift+F3"), this);

    connect(findNextAction, &QShortcut::activated, this, &SearchWidget::findNext);
    connect(findPreviousAction, &QShortcut::activated, this, &SearchWidget::findPrevious);
    connect(ui->pushButtonNext, &QPushButton::clicked, this, &SearchWidget::findNext);
    connect(ui->pushButtonPrevious, &QPushButton::clicked, this, &SearchWidget::findPrevious);
    connect(ui->lineEdit, &QLineEdit::textEdited, this, QOverload<const QString &>::of(&SearchWidget::clearAll));
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &SearchWidget::findNext);
    connect(ui->pushButtonHighlight, &QPushButton::toggled, this, &SearchWidget::highlightText);
    connect(ui->pushButtonClose, &QPushButton::clicked, this, QOverload<>::of(&SearchWidget::clearAll));
    connect(ui->pushButtonClose, &QPushButton::clicked, this, &SearchWidget::close);

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
