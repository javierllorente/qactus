/*
 * Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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
#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QShortcut>
#include <QKeyEvent>
#include "loghighlighter.h"

namespace Ui {
class SearchWidget;
}

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr, QPlainTextEdit *plainTextEdit = nullptr);
    ~SearchWidget();
    void setup();

private slots:
    void findNext();
    void findPrevious();
    void highlightText(bool check);
    void clearAll();
    void clearAll(const QString &text);

private:
    Ui::SearchWidget *ui;
    QPlainTextEdit *m_plainTextEdit;
    LogHighlighter *logHighlighter;
    QString lastSearch;
    void findText(const QString &text, QTextDocument::FindFlags findFlags);
    void setLineEditStyle(bool found);
    void clearHighlight();
    void clearSelection();
    void keyPressEvent(QKeyEvent *event);
};

#endif // SEARCHWIDGET_H
