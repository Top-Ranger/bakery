/*
 * Copyright (C) 2015,2016 Philipp Naumann
 * Copyright (C) 2015,2016 Marcus Soll
 * This file is part of Bakery.
 *
 * Bakery is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bakery is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Bakery.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shapewidget.h"
#include "ui_shapewidget.h"

#include "../lib/helpers.hpp"

#include <QPainter>

using namespace BakeryHelpers;

ShapeWidget::ShapeWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::ShapeWidget),
      _shape(NULL),
      _scale(20),
      _hoverCoordinate(QPointF(-1, -1)),
      _hoverEdgeIndex(-1),
      _dragPointIndex(-1),
      _shapeExteriorValid(false)
{
    _ui->setupUi(this);
    setResolution(0.1);
}

ShapeWidget::~ShapeWidget() { delete _ui; }

void ShapeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Background
    QPainter painter(this);
    painter.setPen(QColor(128, 128, 128));
    painter.fillRect(rect(), palette().background());

    // Polygon
    QPen polygonPen(_shapeExteriorValid ? QColor(128, 128, 128) : QColor(255, 32, 32));
    QPainterPath polygonPath;
    polygonPath.addPolygon(QPolygon(_displayPoints));
    polygonPath.closeSubpath();
    polygonPen.setWidth(2);
    painter.setPen(polygonPen);
    painter.setBrush(_shapeExteriorValid ? QColor(212, 212, 212) : QColor(QColor(255, 168, 168)));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPath(polygonPath);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Hover
    if (_hoverCoordinate.x() >= 0 && _hoverCoordinate.y() >= 0)
    {
        painter.fillRect(rectFor(_hoverCoordinate), QColor(192, 192, 255));
    }
    if (_hoverEdgeIndex != -1)
    {
        QPen highlightPen(QColor(128, 128, 255));
        highlightPen.setWidth(3);
        painter.setPen(highlightPen);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(_displayEdges[_hoverEdgeIndex]);
        painter.setRenderHint(QPainter::Antialiasing, false);
    }

    // Indicators
    int index = 1;
    painter.setPen(QColor(255, 255, 255));
    QRect hoverRect = rectFor(_hoverCoordinate);
    foreach (QRect rect, _displayRects)
    {
        painter.fillRect(rect, rect == hoverRect ? QColor(192, 192, 255) : QColor(128, 128, 255));
        painter.drawText(rect, Qt::AlignCenter, QString::number(index));
        ++index;
    }

    // Grid
    QRect bounds = visibleRegion().boundingRect();
    painter.setPen(QColor(128, 128, 128));
    for (int y = bounds.top() - bounds.top() % _scale; y < qMin(bounds.bottom(), height()); y += _scale)
    {
        painter.drawLine(0, y, width(), y);
    }
    for (int x = bounds.left() - bounds.left() % _scale; x < qMin(bounds.right(), width()); x += _scale)
    {
        painter.drawLine(x, 0, x, width());
    }

    // Frame
    painter.drawLine(bounds.topRight(), bounds.bottomRight());
    painter.drawLine(bounds.bottomLeft(), bounds.bottomRight());
}

void ShapeWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF coordinate = coordinateAt(event->pos());
    QPoint coordinatePrecise = qPointPrecise(coordinate);
    if (coordinate != _hoverCoordinate)
    {
        if (_dragPointIndex != -1)
        {
            _shape->replace(_dragPointIndex, coordinatePrecise);
            emit shapeUpdated();
            updateMetrics();
        }

        int edgeIndex = -1;
        QRect coordinateRect(rectFor(coordinate));
        QPainterPath coordinateRectPath;
        coordinateRectPath.addRect(coordinateRect);
        for (int i = 0; i < _displayEdges.size(); ++i)
        {
            QPainterPath edgePath;
            edgePath.moveTo(_displayEdges[i].p1());
            edgePath.lineTo(_displayEdges[i].p2());
            if (!_displayRects.contains(coordinateRect) && edgePath.intersects(coordinateRectPath))
            {
                edgeIndex = i;
                break;
            }
        }

        setHoverMetrics(edgeIndex, coordinate);
        repaint();
    }
}

void ShapeWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF coordinate = coordinateAt(event->pos());
    QPoint coordinatePrecise = qPointPrecise(coordinate);
    if (event->button() == Qt::LeftButton)
    {
        _dragPointIndex = _shape->indexOf(coordinatePrecise);
    }
}

void ShapeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF coordinate = coordinateAt(event->pos());
    QPoint coordinatePrecise = qPointPrecise(coordinate);
    switch (event->button())
    {
    case Qt::LeftButton:
        if (_dragPointIndex == -1)
        {
            if (_hoverEdgeIndex != -1)
            {
                _shape->insert(_hoverEdgeIndex + 1, coordinatePrecise);
            }
            else
            {
                _shape->append(coordinatePrecise);
            }
            emit shapeUpdated();
            updateMetrics();
        }
        break;
    case Qt::RightButton:
        _shape->removeAll(coordinatePrecise);
        updateMetrics();
        emit shapeUpdated();
        break;
    default:
        break;
    }
    setHoverMetrics(-1, QPointF(-1, -1));
    setDragMetrics(-1);
    repaint();
}

int ShapeWidget::dragPointIndex() const { return _dragPointIndex; }

QPointF ShapeWidget::hoverCoordinate() const { return _hoverCoordinate; }

void ShapeWidget::setShape(Shape *shape)
{
    _shape = shape;
    _shape->ensureClosed(false);
    updateMetrics();
}

int ShapeWidget::resolution() const { return _resolution; }

void ShapeWidget::setResolution(qreal resolution)
{
    _resolution = resolution;
    setHoverMetrics(-1, QPointF(-1, -1));
    updateMetrics();
    updateSize();
}

QSize ShapeWidget::sizeHint() const { return _size; }

void ShapeWidget::updateSize()
{
    _size = QSize(_scale / _resolution * 10, _scale / _resolution * 10);
    updateGeometry();
    repaint();
}

QPointF ShapeWidget::coordinateAt(QPoint point) const { return normalize(QPointF(point) * _resolution / _scale); }

QPointF ShapeWidget::normalize(QPointF coordinate) const
{
    return QPointF(coordinate.x() - fmod(coordinate.x(), _resolution), coordinate.y() - fmod(coordinate.y(), _resolution));
}

QRect ShapeWidget::rectFor(QPointF coordinate) const { return QRect((coordinate / _resolution * _scale).toPoint(), QSize(_scale, _scale)); }

void ShapeWidget::updateMetrics()
{
    if (_shape == NULL)
    {
        return;
    }

    _displayPoints.clear();
    _displayRects.clear();
    foreach (QPoint point, *_shape)
    {
        QRect rect = rectFor(qPointRounded(point));
        _displayPoints << rect.center();
        _displayRects << rect;
    }

    _displayEdges.clear();
    for (int i = 0; i < _displayPoints.size(); ++i)
    {
        _displayEdges << QLine(_displayPoints[i], _displayPoints[(i + 1) % _displayPoints.size()]);
    }

    Shape closedShape(*_shape);
    closedShape.ensureClosed(true);
    _shapeExteriorValid = closedShape.size() > 2 && closedShape.area() > 0;
}

void ShapeWidget::setHoverMetrics(int edgeIndex, QPointF coordinate)
{
    _hoverEdgeIndex = edgeIndex;
    _hoverCoordinate = coordinate;
    emit stateChanged();
}

void ShapeWidget::setDragMetrics(int pointIndex)
{
    _dragPointIndex = pointIndex;
    emit stateChanged();
}

void ShapeWidget::normalizeShape()
{
    _shape->normalize();
    emit shapeUpdated();
    updateMetrics();
    repaint();
}
