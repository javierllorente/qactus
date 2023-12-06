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
#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QColor green(34, 153, 34);
    addedLineFormat.setForeground(green);
    rule.pattern = QRegularExpression("^(\\+.*)$|^(.*added)$|^(add.*)$");
    rule.format = addedLineFormat;
    highlightingRules.append(rule);

    QColor red(221, 68, 68);
    removedLineFormat.setForeground(red);
    rule.pattern = QRegularExpression("^(-.*)$|^(.*deleted)$|^(delete.*)$");
    rule.format = removedLineFormat;
    highlightingRules.append(rule);

    QColor gray(88, 90, 90);
    atLineFormat.setForeground(gray);
    rule.pattern = QRegularExpression("^(@.*)$");
    rule.format = atLineFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
