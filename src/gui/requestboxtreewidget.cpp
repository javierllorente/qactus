/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#include "requestboxtreewidget.h"

RequestBoxTreeWidget::RequestBoxTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    QTreeWidgetItem *incomingItem = new QTreeWidgetItem(this);
    incomingItem->setText(0, "Incoming");
    incomingItem->setIcon(0, QIcon::fromTheme("mail-folder-inbox"));

    QTreeWidgetItem *outgoingItem = new QTreeWidgetItem(this);
    outgoingItem->setText(0, "Outgoing");
    outgoingItem->setIcon(0, QIcon::fromTheme("mail-folder-outbox"));

    QTreeWidgetItem *declinedItem = new QTreeWidgetItem(this);
    declinedItem->setText(0, "Declined");
    declinedItem->setIcon(0, QIcon::fromTheme("dialog-cancel"));

    addTopLevelItem(incomingItem);
    addTopLevelItem(outgoingItem);
    addTopLevelItem(declinedItem);

    if (selectedItems().size()==0 && topLevelItemCount()>0) {
        topLevelItem(0)->setSelected(true);
        oldIndex = 0;
    }

    connect(this, &RequestBoxTreeWidget::itemClicked, [=](QTreeWidgetItem *item){
        int index = indexOfTopLevelItem(item);

        if (index==oldIndex) {
            return;
        }

        switch (index) {
        case 0:
            emit getIncomingRequests();
            break;
        case 1:
            emit getOutgoingRequests();
            break;
        case 2:
            emit getDeclinedRequests();
            break;
        }
        emit requestTypeChanged(index);
        oldIndex = index;
    });
}
