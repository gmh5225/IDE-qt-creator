// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "itemitem.h"

#include "qmt/diagram_controller/diagramcontroller.h"
#include "qmt/diagram/ditem.h"
#include "qmt/diagram_scene/diagramsceneconstants.h"
#include "qmt/diagram_scene/diagramscenemodel.h"
#include "qmt/diagram_scene/parts/contextlabelitem.h"
#include "qmt/diagram_scene/parts/customiconitem.h"
#include "qmt/diagram_scene/parts/editabletextitem.h"
#include "qmt/diagram_scene/parts/stereotypesitem.h"
#include "qmt/infrastructure/geometryutilities.h"
#include "qmt/infrastructure/qmtassert.h"
#include "qmt/stereotype/stereotypecontroller.h"
#include "qmt/stereotype/stereotypeicon.h"
#include "qmt/style/style.h"
#include "qmt/tasks/diagramscenecontroller.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QBrush>
#include <QPen>
#include <QFont>

#include <algorithm>

namespace qmt {

static const qreal BODY_VERT_BORDER = 4.0;
static const qreal BODY_HORIZ_BORDER = 4.0;

ItemItem::ItemItem(DItem *item, DiagramSceneModel *diagramSceneModel, QGraphicsItem *parent)
    : ObjectItem("item", item, diagramSceneModel, parent)
{
}

ItemItem::~ItemItem()
{
}

void ItemItem::update()
{
    prepareGeometryChange();
    updateStereotypeIconDisplay();

    auto diagramItem = dynamic_cast<DItem *>(object());
    Q_UNUSED(diagramItem) // avoid warning about unsed variable
    QMT_ASSERT(diagramItem, return);

    const Style *style = adaptedStyle(shapeIconId());

    if (!shapeIconId().isEmpty()) {
        if (!m_customIcon)
            m_customIcon = new CustomIconItem(diagramSceneModel(), this);
        m_customIcon->setStereotypeIconId(shapeIconId());
        m_customIcon->setBaseSize(stereotypeIconMinimumSize(m_customIcon->stereotypeIcon(), CUSTOM_ICON_MINIMUM_AUTO_WIDTH, CUSTOM_ICON_MINIMUM_AUTO_HEIGHT));
        m_customIcon->setBrush(style->fillBrush());
        m_customIcon->setPen(style->outerLinePen());
        m_customIcon->setZValue(SHAPE_ZVALUE);
    } else if (m_customIcon) {
        m_customIcon->scene()->removeItem(m_customIcon);
        delete m_customIcon;
        m_customIcon = nullptr;
    }

    // shape
    if (!m_customIcon) {
        if (!m_shape)
            m_shape = new QGraphicsRectItem(this);
        m_shape->setBrush(style->fillBrush());
        m_shape->setPen(style->outerLinePen());
        m_shape->setZValue(SHAPE_ZVALUE);
    } else {
        if (m_shape) {
            m_shape->scene()->removeItem(m_shape);
            delete m_shape;
            m_shape = nullptr;
        }
    }

    // stereotypes
    updateStereotypes(stereotypeIconId(), stereotypeIconDisplay(), adaptedStyle(stereotypeIconId()));

    // component name
    updateNameItem(style);

    // context
    if (!suppressTextDisplay() && showContext()) {
        if (!m_contextLabel)
            m_contextLabel = new ContextLabelItem(this);
        m_contextLabel->setFont(style->smallFont());
        m_contextLabel->setBrush(style->textBrush());
        m_contextLabel->setContext(object()->context());
    } else if (m_contextLabel) {
        m_contextLabel->scene()->removeItem(m_contextLabel);
        delete m_contextLabel;
        m_contextLabel = nullptr;
    }

    updateSelectionMarker(m_customIcon);
    updateRelationStarter();
    updateAlignmentButtons();
    updateGeometry();
}

bool ItemItem::intersectShapeWithLine(const QLineF &line, QPointF *intersectionPoint, QLineF *intersectionLine) const
{
    if (m_customIcon) {
        QList<QPolygonF> polygons = m_customIcon->outline();
        for (int i = 0; i < polygons.size(); ++i)
            polygons[i].translate(object()->pos() + object()->rect().topLeft());
        if (shapeIcon().textAlignment() == qmt::StereotypeIcon::TextalignBelow) {
            if (nameItem()) {
                QPolygonF polygon(nameItem()->boundingRect());
                polygon.translate(object()->pos() + nameItem()->pos());
                polygons.append(polygon);
            }
            if (m_contextLabel) {
                QPolygonF polygon(m_contextLabel->boundingRect());
                polygon.translate(object()->pos() + m_contextLabel->pos());
                polygons.append(polygon);
            }
        }
        return GeometryUtilities::intersect(polygons, line, nullptr, intersectionPoint, intersectionLine);
    }
    QRectF rect = object()->rect();
    rect.translate(object()->pos());
    QPolygonF polygon;
    polygon << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft() << rect.topLeft();
    return GeometryUtilities::intersect(polygon, line, intersectionPoint, intersectionLine);
}

QSizeF ItemItem::minimumSize() const
{
    return calcMinimumGeometry();
}

QList<ILatchable::Latch> ItemItem::horizontalLatches(ILatchable::Action action, bool grabbedItem) const
{
    return ObjectItem::horizontalLatches(action, grabbedItem);
}

QList<ILatchable::Latch> ItemItem::verticalLatches(ILatchable::Action action, bool grabbedItem) const
{
    return ObjectItem::verticalLatches(action, grabbedItem);
}

QSizeF ItemItem::calcMinimumGeometry() const
{
    double width = 0.0;
    double height = 0.0;

    if (m_customIcon) {
        QSizeF sz = stereotypeIconMinimumSize(m_customIcon->stereotypeIcon(),
                                              CUSTOM_ICON_MINIMUM_AUTO_WIDTH, CUSTOM_ICON_MINIMUM_AUTO_HEIGHT);
        if (shapeIcon().textAlignment() != qmt::StereotypeIcon::TextalignTop
                  && shapeIcon().textAlignment() != qmt::StereotypeIcon::TextalignCenter)
            return sz;
        width = sz.width();
    }

    height += BODY_VERT_BORDER;
    if (CustomIconItem *stereotypeIconItem = this->stereotypeIconItem()) {
        width = std::max(width, stereotypeIconItem->boundingRect().width());
        height += stereotypeIconItem->boundingRect().height();
    }
    if (StereotypesItem *stereotypesItem = this->stereotypesItem()) {
        width = std::max(width, stereotypesItem->boundingRect().width());
        height += stereotypesItem->boundingRect().height();
    }
    if (nameItem()) {
        width = std::max(width, nameItem()->boundingRect().width());
        height += nameItem()->boundingRect().height();
    }
    if (m_contextLabel)
        height += m_contextLabel->height();
    height += BODY_VERT_BORDER;

    width = BODY_HORIZ_BORDER + width + BODY_HORIZ_BORDER;

    return GeometryUtilities::ensureMinimumRasterSize(QSizeF(width, height), 2 * RASTER_WIDTH, 2 * RASTER_HEIGHT);
}

void ItemItem::updateGeometry()
{
    prepareGeometryChange();

    // calc width and height
    double width = 0.0;
    double height = 0.0;

    QSizeF geometry = calcMinimumGeometry();
    width = geometry.width();
    height = geometry.height();

    if (object()->isAutoSized()) {
        // nothing
    } else {
        QRectF rect = object()->rect();
        if (rect.width() > width)
            width = rect.width();
        if (rect.height() > height)
            height = rect.height();
    }

    // update sizes and positions
    double left = -width / 2.0;
    double right = width / 2.0;
    double top = -height / 2.0;
    double y = top;

    setPos(object()->pos());

    QRectF rect(left, top, width, height);

    // the object is updated without calling DiagramController intentionally.
    // attribute rect is not a real attribute stored on DObject but
    // a backup for the graphics item used for manual resized and persistency.
    object()->setRect(rect);

    if (m_customIcon) {
        m_customIcon->setPos(left, top);
        m_customIcon->setActualSize(QSizeF(width, height));
    }

    if (m_shape)
        m_shape->setRect(rect);

    if (m_customIcon) {
        switch (shapeIcon().textAlignment()) {
        case qmt::StereotypeIcon::TextalignBelow:
            y += height + BODY_VERT_BORDER;
            break;
        case qmt::StereotypeIcon::TextalignCenter:
        {
            double h = 0.0;
            if (CustomIconItem *stereotypeIconItem = this->stereotypeIconItem())
                h += stereotypeIconItem->boundingRect().height();
            if (StereotypesItem *stereotypesItem = this->stereotypesItem())
                h += stereotypesItem->boundingRect().height();
            if (nameItem())
                h += nameItem()->boundingRect().height();
            if (m_contextLabel)
                h += m_contextLabel->height();
            y = top + (height - h) / 2.0;
            break;
        }
        case qmt::StereotypeIcon::TextalignNone:
            // nothing to do
            break;
        case qmt::StereotypeIcon::TextalignTop:
            y += BODY_VERT_BORDER;
            break;
        }
    } else {
        y += BODY_VERT_BORDER;
    }
    if (CustomIconItem *stereotypeIconItem = this->stereotypeIconItem()) {
        stereotypeIconItem->setPos(right - stereotypeIconItem->boundingRect().width() - BODY_HORIZ_BORDER, y);
        y += stereotypeIconItem->boundingRect().height();
    }
    if (StereotypesItem *stereotypesItem = this->stereotypesItem()) {
        stereotypesItem->setPos(-stereotypesItem->boundingRect().width() / 2.0, y);
        y += stereotypesItem->boundingRect().height();
    }
    if (nameItem()) {
        nameItem()->setPos(-nameItem()->boundingRect().width() / 2.0, y);
        y += nameItem()->boundingRect().height();
    }
    if (m_contextLabel) {
        if (m_customIcon) {
            m_contextLabel->resetMaxWidth();
        } else {
            double maxContextWidth = width - 2 * BODY_HORIZ_BORDER;
            m_contextLabel->setMaxWidth(maxContextWidth);
        }
        m_contextLabel->setPos(-m_contextLabel->boundingRect().width() / 2.0, y);
    }

    updateSelectionMarkerGeometry(rect);
    updateRelationStarterGeometry(rect);
    updateAlignmentButtonsGeometry(rect);
    updateDepth();
}

} // namespace qmt
