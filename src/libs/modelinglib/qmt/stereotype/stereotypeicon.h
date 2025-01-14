// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "iconshape.h"

#include <QString>
#include <QSet>
#include <QColor>

namespace qmt {

class QMT_EXPORT StereotypeIcon
{
public:
    enum Element {
        ElementAny,
        ElementPackage,
        ElementComponent,
        ElementClass,
        ElementDiagram,
        ElementItem
    };

    enum Display {
        DisplayNone,
        DisplayLabel,
        DisplayDecoration,
        DisplayIcon,
        DisplaySmart
    };

    enum SizeLock {
        LockNone,
        LockWidth,
        LockHeight,
        LockSize,
        LockRatio
    };

    enum TextAlignment {
        TextalignBelow,
        TextalignCenter,
        TextalignNone,
        TextalignTop
    };

    QString id() const { return m_id; }
    void setId(const QString &id);
    QString title() const;
    void setTitle(const QString &title);
    QSet<Element> elements() const { return m_elements; }
    void setElements(const QSet<Element> &elements);
    QSet<QString> stereotypes() const { return m_stereotypes; }
    void setStereotypes(const QSet<QString> &stereotypes);
    bool hasName() const { return m_hasName; }
    void setHasName(bool hasName);
    QString name() const { return m_name; }
    void setName(const QString &name);
    qreal width() const { return m_width; }
    void setWidth(qreal width);
    qreal height() const { return m_height; }
    void setHeight(qreal height);
    bool hasMinWidth() const { return m_minWidth > 0; }
    qreal minWidth() const { return m_minWidth; }
    void setMinWidth(qreal minWidth);
    bool hasMinHeight() const { return m_minHeight > 0; }
    qreal minHeight() const { return m_minHeight; }
    void setMinHeight(qreal minHeight);
    SizeLock sizeLock() const { return m_sizeLock; }
    void setSizeLock(SizeLock sizeLock);
    Display display() const { return m_display; }
    void setDisplay(Display display);
    TextAlignment textAlignment() const { return m_textAlignment; }
    void setTextAlignment(TextAlignment textAlignment);
    QColor baseColor() const { return m_baseColor; }
    void setBaseColor(const QColor &baseColor);
    IconShape iconShape() const { return m_iconShape; }
    void setIconShape(const IconShape &iconShape);
    IconShape outlineShape() const { return m_outlineShape; }
    void setOutlineShape(const IconShape &outlineShape);

private:
    QString m_id;
    QString m_title;
    QSet<Element> m_elements;
    QSet<QString> m_stereotypes;
    bool m_hasName = false;
    QString m_name;
    qreal m_width = 100.0;
    qreal m_height = 100.0;
    qreal m_minWidth = -1;
    qreal m_minHeight = -1;
    SizeLock m_sizeLock = LockNone;
    Display m_display = DisplaySmart;
    TextAlignment m_textAlignment = TextalignBelow;
    QColor m_baseColor;
    IconShape m_iconShape;
    IconShape m_outlineShape;
};

} // namespace qmt
