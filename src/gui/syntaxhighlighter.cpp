/*
 *  Qactus - A Qt based OBS notifier
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

#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{

}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat addedFormat;
    QColor green(34, 153, 34);
    addedFormat.setForeground(green);

    QTextCharFormat removedFormat;
    QColor red(221, 68, 68);
    removedFormat.setForeground(red);

    QRegularExpression expression("^(-.*)$|^(\\+.*)$");
    QRegularExpressionMatchIterator i = expression.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        setFormat(match.capturedStart(0), match.capturedLength(0), addedFormat);
        setFormat(match.capturedStart(1), match.capturedLength(1), removedFormat);
    }
}
