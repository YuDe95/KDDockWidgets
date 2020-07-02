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

#include "TitleBarWidget_p.h"
#include "DragController_p.h"
#include "Frame_p.h"
#include "FloatingWindow_p.h"
#include "Logging_p.h"
#include "WindowBeingDragged_p.h"
#include "Utils_p.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

using namespace KDDockWidgets;

Button::~Button() {}

TitleBarWidget::TitleBarWidget(Frame *parent)
    : QWidget(parent->asQWidget())
    , Layouting::Widget_qwidget(this)
    , TitleBar((Layouting::Widget_qwidget*)(this), parent)
    , m_layout(new QHBoxLayout(this))
{
    init();
}

TitleBarWidget::TitleBarWidget(FloatingWindow *parent)
    : QWidget(parent->asQWidget())
    , Layouting::Widget_qwidget(this)
    , TitleBar((Layouting::Widget_qwidget*)(this), parent)
    , m_layout(new QHBoxLayout(this))
{
    init();
}

void TitleBarWidget::init()
{
    setFixedHeight(30);
    qCDebug(creation) << "TitleBarWidget" << this;
    m_dockWidgetIcon = new QLabel(this);
    m_layout->addWidget(m_dockWidgetIcon);

    m_layout->addStretch();
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(2);

    m_maximizeButton = TitleBarWidget::createButton(this, style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    m_floatButton = TitleBarWidget::createButton(this, style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    m_closeButton = TitleBarWidget::createButton(this, style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    m_layout->addWidget(m_maximizeButton);
    m_layout->addWidget(m_floatButton);
    m_layout->addWidget(m_closeButton);

    connect(m_floatButton, &QAbstractButton::clicked, this, [this] { onFloatClicked(); } );
    connect(m_closeButton, &QAbstractButton::clicked, this, [this] { onCloseClicked(); } );
    connect(m_maximizeButton, &QAbstractButton::clicked, this, [this] { onMaximizeClicked(); });

    updateCloseButton();
    updateFloatButton();
    updateMaximizeButton();

    connect(this, &TitleBarWidget::titleChanged, this, [this] {
        QWidget::update();
    });

    connect(this, &TitleBarWidget::iconChanged, this, [this] {
        if (icon().isNull()) {
            m_dockWidgetIcon->setPixmap(QPixmap());
        } else {
            const QPixmap pix = icon().pixmap(QSize(28,28));
            m_dockWidgetIcon->setPixmap(pix);
        }
        QWidget::update();
    });
}

QRect TitleBarWidget::iconRect() const
{
    if (icon().isNull()) {
        return QRect(0,0, 0,0);
    } else {
        return QRect(3, 3, 30, 30);
    }
}

int TitleBarWidget::buttonAreaWidth() const
{
    if (m_floatButton->isVisible())
        return QWidget::width() - m_floatButton->x();
    else
        return QWidget::width() - m_closeButton->x();
}

TitleBarWidget::~TitleBarWidget()
{
    // To avoid a crash
    for (auto button : { m_floatButton, m_maximizeButton, m_closeButton }) {
        button->setParent(nullptr);
        button->deleteLater();
    }
}

void TitleBarWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        onDoubleClicked();
}

QWidget *TitleBarWidget::closeButton() const
{
    return m_closeButton;
}

void TitleBarWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QStyleOptionDockWidget titleOpt;
    titleOpt.title = title();
    titleOpt.rect = iconRect().isEmpty() ? QWidget::rect().adjusted(2, 0, -buttonAreaWidth(), 0)
                                         : QWidget::rect().adjusted(iconRect().right(), 0, -buttonAreaWidth(), 0);

    style()->drawControl(QStyle::CE_DockWidgetTitle, &titleOpt, &p, this);
}

void TitleBarWidget::updateFloatButton()
{
    m_floatButton->setVisible(supportsFloatingButton());
}

void TitleBarWidget::updateCloseButton()
{
    const bool anyNonClosable = frame() ? frame()->anyNonClosable()
                                        : (floatingWindow() ? floatingWindow()->anyNonClosable()
                                                            : false);

    qCDebug(closebutton) << Q_FUNC_INFO << "enabled=" << !anyNonClosable;
    m_closeButton->setEnabled(!anyNonClosable);
}

void TitleBarWidget::updateMaximizeButton()
{
    if (auto fw = floatingWindow()) {
        m_maximizeButton->setIcon(style()->standardIcon(fw->isMaximized() ? QStyle::SP_TitleBarNormalButton
                                                                          : QStyle::SP_TitleBarMaxButton));

        m_maximizeButton->setVisible(supportsMaximizeButton());
    } else {
        m_maximizeButton->setVisible(false);
    }
}

bool TitleBarWidget::isCloseButtonVisible() const
{
    return m_closeButton->isVisible();
}

bool TitleBarWidget::isCloseButtonEnabled() const
{
    return m_closeButton->isEnabled();
}

bool TitleBarWidget::isFloatButtonVisible() const
{
    return m_floatButton->isVisible();
}

bool TitleBarWidget::isFloatButtonEnabled() const
{
    return m_floatButton->isEnabled();
}

QAbstractButton *TitleBarWidget::createButton(QWidget *parent, const QIcon &icon)
{
    auto button = new Button(parent);
    button->setIcon(icon);
    return button;
}
