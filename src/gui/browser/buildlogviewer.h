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
#ifndef BUILDLOGVIEWER_H
#define BUILDLOGVIEWER_H

#include <QDialog>
#include <QScrollBar>
#include <QAction>
#include "searchwidget.h"

namespace Ui {
class BuildLogViewer;
}

class BuildLogViewer : public QDialog
{
    Q_OBJECT

public:
    explicit BuildLogViewer(QWidget *parent = nullptr);
    ~BuildLogViewer();

    void setText(const QString &text);

private:
    Ui::BuildLogViewer *ui;
    void scrollToBottom();

private slots:
    void findText();
};

#endif // BUILDLOGVIEWER_H
