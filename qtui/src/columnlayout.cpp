
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Qt User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2013, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

// UI includes:
#include "columnlayout.h"

#include <QFrame>
#include <QLabel>
#include <QTextDocument> // For Qt::escape().

ColumnLayout::ColumnLayout() : QHBoxLayout(), empty_(true) {
}

ColumnLayout::ColumnLayout(QWidget* widget) :
        QHBoxLayout(widget), empty_(true) {
}

void ColumnLayout::addLayout(QLayout* layout, const QString& title) {
    if (! empty_) {
        QFrame* divider = new QFrame();
        divider->setFrameStyle(QFrame::VLine | QFrame::Sunken);
        addWidget(divider);
    } else
        empty_ = false;

    QBoxLayout* col = new QVBoxLayout();

    QLabel* label = new QLabel(QString("<qt><b>%1</b></qt>")
        .arg(Qt::escape(title)));
    label->setAlignment(Qt::AlignCenter);
    col->addWidget(label);

    col->addLayout(layout, 1);

    QHBoxLayout::addLayout(col, 1);
}