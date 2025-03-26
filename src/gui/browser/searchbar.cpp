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
#include "searchbar.h"
#include "ui_searchbar.h"

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchBar)
    , model(new QStringListModel(this))
    , completer(new QCompleter(model, this))
    , timer(new QTimer(this))
{
    ui->setupUi(this);

    QIcon searchIcon(QIcon::fromTheme("search"));
    ui->lineEdit->addAction(searchIcon, QLineEdit::LeadingPosition);

    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    ui->lineEdit->setCompleter(completer);

    timer->setSingleShot(true);

    connect(ui->lineEdit, &QLineEdit::textEdited, this, [=](const QString &text) {
        currentText = text;
        timer->start(200);
    });

    connect(timer, &QTimer::timeout, this, [=]() {
        emit search(currentText);
    });

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, [=]() {
        emit returnPressed(ui->lineEdit->text());
    });
}

SearchBar::~SearchBar()
{
    delete ui;
}

void SearchBar::setFocus()
{
    ui->lineEdit->setFocus();
}

void SearchBar::loadSearchResults(const QStringList &searchResults)
{
    bool containsPartial = false;
    for (const QString &item : searchResults) {
        if (item.contains(ui->lineEdit->text(), Qt::CaseInsensitive)) {
            containsPartial = true;
            break;
        }
    }

    if (!containsPartial) {
        return;
    }

    model->setStringList(searchResults);
    completer->complete();
}
