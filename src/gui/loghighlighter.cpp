/*
 *  Qactus - A Qt-based OBS client
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

#include "loghighlighter.h"

LogHighlighter::LogHighlighter(QTextDocument *parent, const QString &searchText)
    : QSyntaxHighlighter(parent)
{
    QColor yellow(255, 255, 0);
    format.setBackground(yellow);
    pattern = QRegularExpression(searchText, QRegularExpression::CaseInsensitiveOption);
}

void LogHighlighter::highlightBlock(const QString &text)
{
    QRegularExpressionMatchIterator matchIterator = pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        setFormat(match.capturedStart(), match.capturedLength(), format);
    }
}
