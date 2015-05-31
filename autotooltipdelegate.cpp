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

#include "autotooltipdelegate.h"

AutoToolTipDelegate::AutoToolTipDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{}

AutoToolTipDelegate::~AutoToolTipDelegate()
{}

bool AutoToolTipDelegate::helpEvent(QHelpEvent* e, QAbstractItemView* view,
                                    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (!e || !view)
        return false;

    if (e->type() != QEvent::ToolTip)
        return QStyledItemDelegate::helpEvent(e, view, option, index);

    QRect rect = view->visualRect(index);
    QSize size = sizeHint(option, index);

    if (rect.width() < size.width()) {
        QVariant tooltip = index.data(Qt::DisplayRole);
        if (tooltip.canConvert<QString>()) {
            QToolTip::showText(QPoint(e->globalX(), e->globalY()-30), tooltip.toString(), view);
            return true;
        }
    }

    if (!QStyledItemDelegate::helpEvent(e, view, option, index))
        QToolTip::hideText();

    return true;
}
