// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include <utils/layoutbuilder.h>

#include <QApplication>
#include <QLineEdit>

using namespace Utils::Layouting;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto lineEdit = new QLineEdit("0");

    auto minusClick = [lineEdit] {
        lineEdit->setText(QString::number(lineEdit->text().toInt() + 1));
    };

    auto plusClick = [lineEdit] {
        lineEdit->setText(QString::number(lineEdit->text().toInt() + 1));
    };

    Row {
        PushButton { text("-"), onClicked(minusClick) },
        lineEdit,
        PushButton { text("+"), onClicked(plusClick) }
    }.emerge()->show();

    return app.exec();
}
