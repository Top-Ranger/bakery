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

#ifndef SHAPEWIDGET_H
#define SHAPEWIDGET_H

#include "../lib/shape.h"

#include <QWidget>
#include <QMouseEvent>

namespace Ui
{
/*!
 * \brief Core component of Shape editor.
 */
class ShapeWidget;
}

/*!
 * \brief Core component of Shape editor.
 */
class ShapeWidget : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     * \param parent QWidget parent.
     */
    explicit ShapeWidget(QWidget *parent = 0);
    ~ShapeWidget();

    /*!
     * \brief Getter for member _resolution.
     * \return Resolution.
     */
    int resolution() const;

    /*!
     * \brief Setter for member _resolution.
     * \param resolution Resolution.
     */
    void setResolution(qreal resolution);

    /*!
     * \brief Returns _size.
     * \return Preferred widget dimensions.
     */
    QSize sizeHint() const;

    /*!
     * \brief Updates _size according to resolution.
     */
    void updateSize();

    /*!
     * \brief Setter for member _shape. Ensures that the Shape is closed.
     * \param shape Shape.
     */
    void setShape(Shape *shape);

    /*!
     * \brief Getter for member _hoverCoordinate.
     * \return Currently hovered coordinate.
     */
    QPointF hoverCoordinate() const;

    /*!
     * \brief Getter for member _dragPointIndex.
     * \return Index of point that is currently dragged, -1 if none.
     */
    int dragPointIndex() const;

    /*!
     * \brief Calls Shape::normalize().
     * \sa Shape::normalize()
     */
    void normalizeShape();

protected:
    /*!
     * \brief Overrides QWidget::paintEvent(QPaintEvent). Paints grid and Shape.
     * \param event Paint event.
     */
    void paintEvent(QPaintEvent *event);

    /*!
     * \brief Overrides QWidget::mouseMoveEvent(QMouseEvent). Handles mouse movements.
     * \param event Mouse event.
     */
    void mouseMoveEvent(QMouseEvent *event);

    /*!
     * \brief Overrides QWidget::mousePressEvent(QMouseEvent). Handles mouse button presses.
     * \param event Mouse event.
     */
    void mousePressEvent(QMouseEvent *event);

    /*!
     * \brief Overrides QWidget::mouseReleaseEvent(QMouseEvent). Handles mouse button releases.
     * \param event Mouse event.
     */
    void mouseReleaseEvent(QMouseEvent *event);

private:
    /*!
     * \brief Pointer to UI container class instance.
     */
    Ui::ShapeWidget *_ui;

    /*!
     * \brief Shape to be edited.
     */
    Shape *_shape;

    /*!
     * \brief Display scale.
     */
    int _scale;

    /*!
     * \brief Display resolution.
     */
    qreal _resolution;

    /*!
     * \brief Widget size.
     */
    QSize _size;

    /*!
     * \brief Currently hovered coordinate, -1 if not hovering.
     */
    QPointF _hoverCoordinate;

    /*!
     * \brief Index of currently hovered edge, -1 if not hovering.
     */
    int _hoverEdgeIndex;

    /*!
     * \brief Index of currently dragged point, -1 if not dragging.
     */
    int _dragPointIndex;

    /*!
     * \brief true if Shape points adhere to limitations.
     */
    bool _shapeExteriorValid;

    /*!
     * \brief Pixel-based display points.
     */
    QVector<QPoint> _displayPoints;

    /*!
     * \brief Pixel-based display rectangles.
     */
    QVector<QRect> _displayRects;

    /*!
     * \brief Pixel-based display edges.
     */
    QVector<QLine> _displayEdges;

    /*!
     * \brief Computes the Shape coordinate (e.g. (1.2, 2.3)) at the given point.
     * \param point Pixel coordinate.
     * \return
     */
    QPointF coordinateAt(QPoint point) const;

    /*!
     * \brief Aligns a Shape coordinate with the grid.
     * \param coordinate Shape coordinate.
     * \return
     */
    QPointF normalize(QPointF coordinate) const;

    /*!
     * \brief Compute the pixel coordinate rectangle for a given Shape coordinate.
     * \param coordinate Shape coordinate.
     * \return
     */
    QRect rectFor(QPointF coordinate) const;

    /*!
     * \brief Updates display metrics.
     */
    void updateMetrics();

    /*!
     * \brief Sets hover metrics.
     * \param edgeIndex Index of currently hovered edge.
     * \param coordinate Shape coordinate of currently hovered point.
     */
    void setHoverMetrics(int edgeIndex, QPointF coordinate);

    /*!
     * \brief Sets drag metrics.
     * \param pointIndex Index of currently dragged point.
     */
    void setDragMetrics(int pointIndex);

signals:
    /*!
     * \brief Is emitted when the Shape is updated, i.e. when a point is added or dragged.
     */
    void shapeUpdated();

    /*!
     * \brief Is emitted when the widget's state changes.
     */
    void stateChanged();
};

#endif // SHAPEWIDGET_H
