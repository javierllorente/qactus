/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
