// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "edit3dactions.h"
#include "edit3dview.h"
#include "edit3dwidget.h"

#include <viewmanager.h>
#include <nodeinstanceview.h>
#include <qmldesignerplugin.h>
#include <nodemetainfo.h>

#include <utils/algorithm.h>

#include <QDebug>

namespace QmlDesigner {

Edit3DActionTemplate::Edit3DActionTemplate(const QString &description,
                                           SelectionContextOperation action,
                                           AbstractView *view,
                                           View3DActionType type)
    : DefaultAction(description)
    , m_action(action)
    , m_view(view)
    , m_type(type)
{}

void Edit3DActionTemplate::actionTriggered(bool b)
{
    if (m_type != View3DActionType::Empty && m_type != View3DActionType::SelectBackgroundColor
        && m_type != View3DActionType::SelectGridColor) {
        m_view->emitView3DAction(m_type, b);
    }

    if (m_action)
        m_action(m_selectionContext);
}

Edit3DAction::Edit3DAction(const QByteArray &menuId,
                           View3DActionType type,
                           const QString &description,
                           const QKeySequence &key,
                           bool checkable,
                           bool checked,
                           const QIcon &iconOff,
                           const QIcon &iconOn,
                           AbstractView *view,
                           SelectionContextOperation selectionAction,
                           const QString &toolTip)
    : AbstractAction(new Edit3DActionTemplate(description, selectionAction, view, type))
    , m_menuId(menuId)
{
    action()->setShortcut(key);
    action()->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action()->setCheckable(checkable);
    action()->setChecked(checked);

    // Description will be used as tooltip by default if no explicit tooltip is provided
    if (!toolTip.isEmpty())
        action()->setToolTip(toolTip);

    if (checkable) {
        QIcon onOffIcon;
        const auto onAvail = iconOn.availableSizes(); // Assume both icons have same sizes available
        for (const auto &size : onAvail) {
            onOffIcon.addPixmap(iconOn.pixmap(size), QIcon::Normal, QIcon::On);
            onOffIcon.addPixmap(iconOff.pixmap(size), QIcon::Normal, QIcon::Off);
        }
        action()->setIcon(onOffIcon);
    } else {
        action()->setIcon(iconOff);
    }
}

QByteArray Edit3DAction::category() const
{
    return QByteArray();
}

bool Edit3DAction::isVisible([[maybe_unused]] const SelectionContext &selectionContext) const
{
    return true;
}

bool Edit3DAction::isEnabled(const SelectionContext &selectionContext) const
{
    return isVisible(selectionContext);
}

Edit3DCameraAction::Edit3DCameraAction(const QByteArray &menuId,
                                       View3DActionType type,
                                       const QString &description,
                                       const QKeySequence &key,
                                       bool checkable,
                                       bool checked,
                                       const QIcon &iconOff,
                                       const QIcon &iconOn,
                                       AbstractView *view,
                                       SelectionContextOperation selectionAction)
    : Edit3DAction(menuId, type, description, key, checkable, checked, iconOff, iconOn, view, selectionAction)
{
}

bool Edit3DCameraAction::isEnabled(const SelectionContext &selectionContext) const
{
    return Utils::anyOf(selectionContext.selectedModelNodes(),
                        [](const ModelNode &node) { return node.metaInfo().isQtQuick3DCamera(); });
}

}

