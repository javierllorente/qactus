/*
 * Copyright (C) 2019-2020 Javier Llorente <javier@opensuse.org>
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
#include "iconbar.h"
#include <QStyledItemDelegate>
#include <QPainter>

class Delegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit Delegate(QListWidget *parent = nullptr)
        : QStyledItemDelegate(parent),
          m_listWidget(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!index.isValid() || !index.internalPointer()) {
            return;
        }

        painter->save();

        QStyleOptionViewItem opt(*static_cast<const QStyleOptionViewItem *>(&option));
        m_listWidget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

        const int textHeight = painter->fontMetrics().height();
        const int spacing = m_listWidget->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
        const int y = opt.rect.y() + (opt.rect.height() - iconSize - spacing - textHeight) / 2;

        opt.icon = index.data(Qt::DecorationRole).value<QIcon>();
        opt.icon.paint(painter, QRect(opt.rect.x(), y - spacing, opt.rect.width(), iconSize),
                       Qt::AlignCenter, QIcon::Normal, QIcon::On);

        painter->drawText(QRect(opt.rect.x(), y + iconSize, opt.rect.width(), textHeight),
                          index.data(Qt::DisplayRole).toString(), { Qt::AlignCenter });

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
     {
        Q_UNUSED(option)
        if (!index.isValid() || !index.internalPointer()) {
            return QSize();
        }

        return QSize(m_listWidget->width() - (m_listWidget->style()->pixelMetric(QStyle::PM_FocusFrameVMargin)*2), 70);
 }

private:
    QListWidget *m_listWidget = nullptr;
    const int iconSize = 32;
};

IconBar::IconBar(QWidget *parent) :
    QListWidget(parent)
{
    setItemDelegate(new Delegate(this));
}

#include "iconbar.moc"
