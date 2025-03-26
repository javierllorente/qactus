/*
 * Copyright (C) 2025 Javier Llorente <javier@opensuse.org>
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
#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include <QStringListModel>
#include <QCompleter>
#include <QTimer>

namespace Ui {
class SearchBar;
}

class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);
    ~SearchBar();
    void setFocus();

private:
    Ui::SearchBar *ui;
    QStringListModel *model;
    QCompleter *completer;
    QTimer *timer;
    QString currentText;

signals:
    void search(const QString &term);
    void returnPressed(const QString &location);

public slots:
    void loadSearchResults(const QStringList &searchResults);

};

#endif // SEARCHBAR_H
