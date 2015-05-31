/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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

/*
 * If text is truncated on a QTreeWidgetItem,
 * AutoToolTipDelegate displays it as a tooltip on mouse over.
 * Based on KCachegrind's custom item delegate.
 * http://quickgit.kde.org/?p=kcachegrind.git
 * Copyright (C) 2012 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>
*/

#ifndef AUTOTOOLTIPDELEGATE_H
#define AUTOTOOLTIPDELEGATE_H

#include <QHelpEvent>
#include <QAbstractItemView>
#include <QToolTip>
#include <QStyledItemDelegate>

class AutoToolTipDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    AutoToolTipDelegate(QObject* parent = 0);
    ~AutoToolTipDelegate();

public slots:
    bool helpEvent(QHelpEvent* e, QAbstractItemView* view,
                   const QStyleOptionViewItem& option, const QModelIndex& index);
};

#endif // AUTOTOOLTIPDELEGATE_H
