/*
 * Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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
