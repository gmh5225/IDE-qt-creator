// Copyright (C) 2019 Klarälvdalens Datakonsult AB, a KDAB Group company,
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "ctfvisualizerconstants.h"

#include <utils/itemviews.h>

namespace CtfVisualizer {
namespace Internal {

class CtfStatisticsModel;

class CtfStatisticsView : public Utils::TreeView
{
    Q_OBJECT
public:
    explicit CtfStatisticsView(CtfStatisticsModel *model, QWidget *parent = nullptr);
    ~CtfStatisticsView() override = default;

    void selectByTitle(const QString &title);

signals:
    void eventTypeSelected(const QString &title);
};

}  // Internal
}  // CtfVisualizer
