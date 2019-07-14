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
