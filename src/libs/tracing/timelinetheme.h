// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "tracing_global.h"

#include <utils/theme/theme.h>

#include <QObject>
#include <QtQml/qqml.h>

namespace Timeline {

class TRACING_EXPORT TimelineTheme : public Utils::Theme
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Theme)
    QML_SINGLETON

public:
    explicit TimelineTheme(QObject *parent = nullptr);

    static void setupTheme(QQmlEngine* engine);
};

} // namespace Timeline
