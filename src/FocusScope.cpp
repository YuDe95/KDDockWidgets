/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief FocusScope
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "FocusScope.h"
#include "TitleBar_p.h"

#include <QObject>
#include <QApplication>
#include <QPointer>

using namespace KDDockWidgets;

// Our Private inherits from QObject since FocusScope can't (Since Frame is already QObject)
class FocusScope::Private : public QObject
{
public:
    Private(FocusScope *qq, QWidgetAdapter *thisWidget)
        : q(qq)
        , m_thisWidget(thisWidget)
    {
        connect(qApp, &QGuiApplication::focusObjectChanged,
                this, &Private::onFocusObjectChanged);

        onFocusObjectChanged(qApp->focusObject());
        m_inCtor = false;
    }

    void setIsFocused(bool);
    void onFocusObjectChanged(QObject *);
    bool isInFocusScope(WidgetType *) const;

    FocusScope *const q;
    QWidgetAdapter *const m_thisWidget;
    bool m_isFocused = false;
    bool m_inCtor = true;
    QPointer<WidgetType> m_lastFocusedInScope;
};


FocusScope::FocusScope(QWidgetAdapter *thisWidget)
    : d(new Private(this, thisWidget))
{
}

FocusScope::~FocusScope()
{
    delete d;
}

bool FocusScope::isFocused() const
{
    return d->m_isFocused;
}

WidgetType *FocusScope::focusedWidget() const
{
    return d->m_lastFocusedInScope;
}

void FocusScope::focus(Qt::FocusReason reason)
{
    if (d->m_lastFocusedInScope) {
        d->m_lastFocusedInScope->setFocus(reason);
    } else {
        if (auto frame = qobject_cast<Frame*>(d->m_thisWidget)) {
            if (DockWidgetBase *dw = frame->currentDockWidget()) {
                if (auto guest = dw->widget()) {
                    if (guest->focusPolicy() != Qt::NoFocus)
                        guest->setFocus(reason);
                }
            }
        } else {
            // Not a use case right now
            d->m_thisWidget->setFocus(reason);
        }
    }
}

void FocusScope::Private::setIsFocused(bool is)
{
    if (is != m_isFocused) {
        m_isFocused = is;

        if (!m_inCtor) // Hack so we don't call pure-virtual
            Q_EMIT q->isFocusedChanged();
    }
}

void FocusScope::Private::onFocusObjectChanged(QObject *obj)
{
    auto widget = qobject_cast<WidgetType*>(obj);
    if (!widget)
        return;

    const bool is = isInFocusScope(widget);
    if (is && m_lastFocusedInScope != widget && !qobject_cast<TitleBar*>(obj)) {
        m_lastFocusedInScope = widget;
        Q_EMIT q->focusedWidgetChanged();
    }

    setIsFocused(is);
}

bool FocusScope::Private::isInFocusScope(WidgetType *widget) const
{
    WidgetType *p = widget;
    while (p) {
        if (p == m_thisWidget)
            return true;

        p = KDDockWidgets::Private::parentWidget(p);
    }

    return false;
}
