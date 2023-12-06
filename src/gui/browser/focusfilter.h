/*
 * Copyright (C) 2020 Javier Llorente <javier@opensuse.org>
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
#ifndef FOCUSFILTER_H
#define FOCUSFILTER_H

#include <QObject>
#include <QEvent>

class FocusFilter : public QObject
{
    Q_OBJECT

public:
    explicit FocusFilter(QObject *parent = nullptr);

protected:
      bool eventFilter(QObject *obj, QEvent *event) override;

signals:
      void hasFocus(bool focus);

};

#endif // FOCUSFILTER_H
