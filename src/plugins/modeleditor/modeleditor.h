// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/editormanager/ieditor.h>

#include <QAbstractButton>

#include <functional>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QToolButton;
QT_END_NAMESPACE

namespace qmt {
class MElement;
class MPackage;
class MDiagram;
class DElement;
class DContainer;
}

namespace ModelEditor {
namespace Internal {

class UiController;
class ActionHandler;

enum class SelectedArea {
    Nothing,
    Diagram,
    TreeView
};

class ModelEditor :
        public Core::IEditor
{
    Q_OBJECT
    class ModelEditorPrivate;

public:
    ModelEditor(UiController *uiController, ActionHandler *actionHandler);
    ~ModelEditor();

    Core::IDocument *document() const override;
    QWidget *toolBar() override;
    QByteArray saveState() const override;
    void restoreState(const QByteArray &state) override;

    qmt::MDiagram *currentDiagram() const;
    void showDiagram(qmt::MDiagram *diagram);
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void removeSelectedElements();
    void deleteSelectedElements();
    void selectAll();
    void openParentDiagram();
    void editProperties();
    void editSelectedItem();
    void exportDiagram();
    void exportSelectedElements();
    void zoomIn();
    void zoomOut();
    void zoomInAtPos(const QPoint &pos);
    void zoomOutAtPos(const QPoint &pos);
    void resetZoom();

    qmt::MPackage *guessSelectedPackage() const;

private:
    void init();
    void initDocument();

    void updateSelectedArea(SelectedArea selectedArea);
    void showProperties(const QList<qmt::MElement *> &modelElements);
    void showProperties(qmt::MDiagram *diagram, const QList<qmt::DElement *> &diagramElements);
    void clearProperties();
    void expandModelTreeToDepth(int depth);
    QToolButton *createToolbarCommandButton(const Utils::Id &id,
                                            const std::function<void()> &slot,
                                            QWidget *parent);
    bool updateButtonIconByTheme(QAbstractButton *button, const QString &name);
    void showZoomIndicator();
    void zoomAtPos(const QPoint &pos, double scale);

    void onAddPackage();
    void onAddComponent();
    void onAddClass();
    void onAddCanvasDiagram();
    void onCurrentEditorChanged(Core::IEditor *editor);
    void onCanUndoChanged(bool canUndo);
    void onCanRedoChanged(bool canRedo);
    void onTreeModelReset();
    void onTreeViewSelectionChanged(const QItemSelection &selected,
                                    const QItemSelection &deselected);
    void onTreeViewActivated();
    void onTreeViewDoubleClicked(const QModelIndex &index);
    void onCurrentDiagramChanged(const qmt::MDiagram *diagram);
    void onDiagramActivated(const qmt::MDiagram *diagram);
    void onDiagramClipboardChanged(bool isEmpty);
    void onNewElementCreated(qmt::DElement *element, qmt::MDiagram *diagram);
    void onDiagramSelectionChanged(const qmt::MDiagram *diagram);
    void onDiagramModified(const qmt::MDiagram *diagram);
    void onRightSplitterMoved(int pos, int index);
    void onRightSplitterChanged(const QByteArray &state);
    void onRightHorizSplitterMoved(int pos, int index);
    void onRightHorizSplitterChanged(const QByteArray &state);
    void onToolbarSelectionChanged();

    void initToolbars();
    void openDiagram(qmt::MDiagram *diagram, bool addToHistory);
    void closeCurrentDiagram(bool addToHistory);
    void closeDiagram(const qmt::MDiagram *diagram);
    void closeAllDiagrams();

    void onContentSet();

    void setDiagramClipboard(const qmt::DContainer &dcontainer);

    void addDiagramToSelector(const qmt::MDiagram *diagram);
    void updateDiagramSelector();
    void onDiagramSelectorSelected(int index);
    QString buildDiagramLabel(const qmt::MDiagram *diagram);
    void storeToolbarIdInDiagram(qmt::MDiagram *diagram);

    void addToNavigationHistory(const qmt::MDiagram *diagram);
    QByteArray saveState(const qmt::MDiagram *diagram) const;

    void onEditSelectedElement();
    bool isSyncBrowserWithDiagram() const;
    bool isSyncDiagramWithBrowser() const;
    void synchronizeDiagramWithBrowser();
    void synchronizeBrowserWithDiagram(const qmt::MDiagram *diagram);

    void exportToImage(bool selectedElements);

private:
    ModelEditorPrivate *d;
};

} // namespace Internal
} // namespace ModelEditor
