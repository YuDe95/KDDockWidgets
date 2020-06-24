/*
  This file is part of KDDockWidgets.

  Copyright (C) 2018-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Sérgio Martins <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "FloatingWindowWidget_p.h"
#include "Logging_p.h"
#include "Utils_p.h"
#include "DropArea_p.h"
#include "TitleBar_p.h"
#include "MainWindowBase.h"

#include <QApplication>
#include <QPainter>
#include <QVBoxLayout>
#include <QWindowStateChangeEvent>

using namespace KDDockWidgets;

static QWidget* hackFindParentHarder(Frame *frame, MainWindowBase *candidateParent)
{
    MainWindowBase *mw = FloatingWindow::hackFindParentHarder(frame, candidateParent);
    return mw ? mw->asQWidget() : nullptr;
}

FloatingWindowWidget::FloatingWindowWidget(MainWindowBase *parent)
    : QWidget(parent->asQWidget(), KDDockWidgets::usesNativeDraggingAndResizing() ? Qt::Window : Qt::Tool)
    , Layouting::Widget_qwidget(this)
    , FloatingWindow((Layouting::Widget_qwidget*)(this))
    , m_vlayout(new QVBoxLayout(this))
{
    init();
}

FloatingWindowWidget::FloatingWindowWidget(Frame *frame, MainWindowBase *parent)
    : QWidget(::hackFindParentHarder(frame, parent), KDDockWidgets::usesNativeDraggingAndResizing() ? Qt::Window : Qt::Tool)
    , Layouting::Widget_qwidget(this)
    , FloatingWindow((Layouting::Widget_qwidget*)(this), frame)
    , m_vlayout(new QVBoxLayout(this))
{
    init();
}

void FloatingWindowWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(0x666666);
    p.drawRect(QWidget::rect().adjusted(0, 0, -1, -1));
}

bool FloatingWindowWidget::event(QEvent *ev)
{
    if (ev->type() == QEvent::WindowStateChange)
        Q_EMIT windowStateChanged(static_cast<QWindowStateChangeEvent*>(ev));

    return QWidget::event(ev);
}

void FloatingWindowWidget::init()
{
    m_vlayout->setSpacing(0);
    m_vlayout->setContentsMargins(0, 0, 0, 0);
    m_vlayout->addWidget(m_titleBar->asQWidget());
    m_vlayout->addWidget(m_dropArea);

    if (!KDDockWidgets::usesNativeDraggingAndResizing())
        m_vlayout->setContentsMargins(4, 4, 4, 4);
}
