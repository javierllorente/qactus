/*
 * Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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
#include "requestboxtreewidget.h"

RequestBoxTreeWidget::RequestBoxTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    QTreeWidgetItem *incomingItem = new QTreeWidgetItem(this);
    incomingItem->setText(0, tr("Incoming"));
    incomingItem->setIcon(0, QIcon::fromTheme("mail-folder-inbox"));

    QTreeWidgetItem *outgoingItem = new QTreeWidgetItem(this);
    outgoingItem->setText(0, tr("Outgoing"));
    outgoingItem->setIcon(0, QIcon::fromTheme("mail-folder-outbox"));

    QTreeWidgetItem *declinedItem = new QTreeWidgetItem(this);
    declinedItem->setText(0, tr("Declined"));
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
