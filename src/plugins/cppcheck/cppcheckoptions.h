// Copyright (C) 2018 Sergey Morozov
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/dialogs/ioptionspage.h>
#include <utils/filepath.h>

#include <QCoreApplication>
#include <QPointer>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QCheckBox;
QT_END_NAMESPACE

namespace Utils { class PathChooser; }

namespace Cppcheck {
namespace Internal {

class CppcheckTool;
class CppcheckTrigger;
class OptionsWidget;

class CppcheckOptions final
{
public:
    Utils::FilePath binary;

    bool warning = true;
    bool style = true;
    bool performance = true;
    bool portability = true;
    bool information = true;
    bool unusedFunction = false;
    bool missingInclude = false;
    bool inconclusive = false;
    bool forceDefines = false;

    QString customArguments;
    QString ignoredPatterns;
    bool showOutput = false;
    bool addIncludePaths = false;
    bool guessArguments = true;
};

class OptionsWidget final : public QWidget
{
    Q_DECLARE_TR_FUNCTIONS(CppcheckOptionsPage)
public:
    explicit OptionsWidget(QWidget *parent = nullptr);
    void load(const CppcheckOptions &options);
    void save(CppcheckOptions &options) const;

private:
    Utils::PathChooser *m_binary = nullptr;
    QLineEdit *m_customArguments = nullptr;
    QLineEdit *m_ignorePatterns = nullptr;
    QCheckBox *m_warning = nullptr;
    QCheckBox *m_style = nullptr;
    QCheckBox *m_performance = nullptr;
    QCheckBox *m_portability = nullptr;
    QCheckBox *m_information = nullptr;
    QCheckBox *m_unusedFunction = nullptr;
    QCheckBox *m_missingInclude = nullptr;
    QCheckBox *m_inconclusive = nullptr;
    QCheckBox *m_forceDefines = nullptr;
    QCheckBox *m_showOutput = nullptr;
    QCheckBox *m_addIncludePaths = nullptr;
    QCheckBox *m_guessArguments = nullptr;
};

class CppcheckOptionsPage final : public Core::IOptionsPage
{
    Q_OBJECT
public:
    explicit CppcheckOptionsPage(CppcheckTool &tool, CppcheckTrigger &trigger);

    QWidget *widget() final;
    void apply() final;
    void finish() final;

private:
    void save(const CppcheckOptions &options) const;
    void load(CppcheckOptions &options) const;

    CppcheckTool &m_tool;
    CppcheckTrigger &m_trigger;
    QPointer<OptionsWidget> m_widget;
};

} // namespace Internal
} // namespace Cppcheck
