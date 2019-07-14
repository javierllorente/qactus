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
