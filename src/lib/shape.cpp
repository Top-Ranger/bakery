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

#include "shape.h"

#include "helpers.hpp"

#include <QTransform>

Shape::Shape(QString name) : _name(name), _simple(true), _updateMetrics(true) {}

Shape::Shape(const Shape &other) : QPolygon(other)
{
    _name = other._name;
    _simple = other._simple;
    _edges = other._edges;
    _signedArea = other._signedArea;
    _centroid = other._centroid;
    _transform = other._transform;
    _innerDistances = other._innerDistances;
    _updateMetrics = other._updateMetrics;
}

Shape::Shape(const QPolygon &polygon) : QPolygon(polygon), _simple(true), _updateMetrics(true)
{
    _name = defaultName();
    updateMetrics();
}

void Shape::append(const QPoint &p)
{
    QPolygon::append(p);
    updateMetrics();
}

void Shape::normalize() { moveTo(0, 0); }

Shape Shape::normalized() const
{
    Shape normalized(*this);
    normalized.normalize();
    return normalized;
}

void Shape::moveTo(qint32 x, qint32 y)
{
    QPoint p = boundingRect().topLeft();
    transform(QTransform::fromTranslate(x - p.x(), y - p.y()));
}

void Shape::moveTo(const QPoint &p) { moveTo(p.x(), p.y()); }

void Shape::updateMetrics()
{
    if (!_updateMetrics)
    {
        return;
    }

    // Edges, signed area, inner distances and centroid
    qint64 cx = 0, cy = 0;
    _signedArea = 0;
    _edges.clear();
    _innerDistances.clear();
    _innerDistances.reserve(size());

    // Required because QLine does not implement intersect()
    QList<QLineF> edgesF;
    edgesF.reserve(size() - 1);

    for (Iterator i_point = begin(); i_point < end() - 1; ++i_point)
    {
        _edges << QLine(*i_point, *(i_point + 1));
        edgesF << QLineF(BakeryHelpers::qPointRounded(*i_point), BakeryHelpers::qPointRounded(*(i_point + 1)));
        qint64 x1 = i_point->x(), y1 = i_point->y(), x2 = (i_point + 1)->x(), y2 = (i_point + 1)->y();
        qint64 z = x1 * y2 - x2 * y1;
        _signedArea += z;
        cx += (x1 + x2) * z;
        cy += (y1 + y2) * z;
        qint32 innerX = qAbs((i_point + 1)->x() - i_point->x());
        if (innerX > 0)
        {
            _innerDistances << innerX;
        }
        qint32 innerY = qAbs((i_point + 1)->y() - i_point->y());
        if (innerY > 0)
        {
            _innerDistances << innerY;
        }
    }
    if (!isClosed() && size() > 1)
    {
        edgesF << QLineF(BakeryHelpers::qPointRounded(last()), BakeryHelpers::qPointRounded(first()));
    }
    _signedArea /= 2;
    if (_signedArea != 0)
    {
        cx /= _signedArea * 6;
        cy /= _signedArea * 6;
    }
    _centroid = QPoint(cx, cy);

    // BAKERY_PRECISION is considered twice when computing z
    _signedArea /= BAKERY_PRECISION;

    // Simplicity test
    _simple = true;
    for (QList<QLineF>::Iterator i_edgeF = edgesF.begin(); _simple && i_edgeF != edgesF.end(); ++i_edgeF)
    {
        for (QList<QLineF>::Iterator i_edgeFInner = i_edgeF; _simple && i_edgeFInner != edgesF.end(); ++i_edgeFInner)
        {
            QPointF p;
            if (i_edgeF->intersect(*i_edgeFInner, &p) != QLineF::BoundedIntersection)
            {
                continue;
            }
            if ((p == i_edgeF->p1() && p == i_edgeFInner->p2()) || (p == i_edgeF->p2() && p == i_edgeFInner->p1()))
            {
                continue;
            }
            _simple = false;
        }
    }
}

void Shape::transform(QTransform transform)
{
    QPolygon transformed = transform.map(*this);
    swap(transformed);
    _transform *= transform;
    updateMetrics();
}

Shape Shape::transformed(QTransform transform)
{
    Shape transformed(*this);
    transformed.transform(transform);
    return transformed;
}

void Shape::translate(const QPoint &p)
{
    QTransform translation = QTransform::fromTranslate(p.x(), p.y());
    transform(translation);
}

void Shape::scale(qreal sx, qreal sy)
{
    QTransform scale;
    scale.scale(sx, sy);
    transform(scale);
}

Shape Shape::scaled(qreal sx, qreal sy) const
{
    Shape scaled(*this);
    scaled.scale(sx, sy);
    return scaled;
}

void Shape::invert() { transform(_transform.inverted()); }

Shape Shape::inverted() const
{
    Shape inverted(*this);
    inverted.invert();
    return inverted;
}

bool Shape::isSimple() const { return _simple; }

void Shape::rotate(QPoint center, qreal angle)
{
    QTransform rotation;
    rotation.translate(center.x(), center.y());
    rotation.rotateRadians(angle);
    rotation.translate(-center.x(), -center.y());
    transform(rotation);
}

Shape Shape::rotated(QPoint center, qreal angle) const
{
    Shape rotated(*this);
    rotated.rotate(center, angle);
    return rotated;
}

void Shape::ensureClosed(bool closed)
{
    if (closed)
    {
        if (!isClosed() && size() > 0)
        {
            append(first());
        }
    }
    else
    {
        if (isClosed() && size() > 1)
        {
            removeLast();
        }
    }
    updateMetrics();
}

bool Shape::isCongruent(Shape &other)
{
    Shape otherInverted = other.inverted();
    return inverted() == otherInverted;
}

bool Shape::isClosed() const { return size() == 0 || (size() > 2 && first() == last()); }

bool Shape::intersects(const Shape &other) const
{
    // Polygons do not intersect if their bounding rectangles do not intersect.
    // This is a cost-effective pre-test
    if (boundingRect().intersected(other.boundingRect()).size().isEmpty())
    {
        return false;
    }

    return intersected(other).size() > 0;
}

QSet<qint32> Shape::innerDistances() const { return _innerDistances; }

QList<QLine> Shape::edges() const { return _edges; }

Shape Shape::convexHull() const
{
    // Unique points
    QList<QPoint> ps;
    for (QVector<QPoint>::ConstIterator i_point = constBegin(); i_point != constEnd(); ++i_point)
    {
        if (!ps.contains(*i_point))
        {
            ps << *i_point;
        }
    }

    int n = ps.size();
    if (Q_UNLIKELY(n < 3))
    {
        return Shape();
    }

    // Top-left-most point
    int l = 0;
    for (int i = 1; i < n; ++i)
    {
        if (ps[i].x() < ps[l].x() || (ps[i].x() == ps[l].x() && ps[i].y() < ps[l].y()))
        {
            l = i;
        }
    }

    int p = l, q;
    Shape hull;
    hull.setUpdateMetrics(false);
    do
    {
        q = (p + 1) % n;
        for (int i = 0; i < n; ++i)
        {
            if (BakeryHelpers::vectorOrientation(ps[p], ps[i], ps[q]) == 2)
            {
                q = i;
            }
        }
        hull << ps[p];
        p = q;
        if (hull.size() > n)
        {
            BAKERY_WARNING("Computation of convex hull failed");
            return Shape();
        }
    } while (p != l);
    hull.ensureClosed();
    hull.setUpdateMetrics(true);

    return hull;
}

qint64 Shape::area() const
{
    if (Q_UNLIKELY(!_simple))
    {
        BAKERY_DEBUG("Shape is not simple");
    }
    if (Q_UNLIKELY(!isClosed()))
    {
        BAKERY_WARNING("Shape is not closed");
    }
    return qAbs(_signedArea);
}

qint64 Shape::signedArea() const
{
    if (Q_UNLIKELY(!_simple))
    {
        BAKERY_DEBUG("Shape is not simple");
    }
    if (!Q_UNLIKELY(isClosed()))
    {
        BAKERY_WARNING("Shape is not closed");
    }
    return _signedArea;
}

QPoint Shape::centroid() const
{
    if (Q_UNLIKELY(!_simple))
    {
        BAKERY_DEBUG("Shape is not simple");
    }
    if (!Q_UNLIKELY(isClosed()))
    {
        BAKERY_WARNING("Shape is not closed");
    }
    return _centroid;
}

QPoint Shape::position() const { return boundingRect().topLeft(); }

QString Shape::name() const { return _name; }

void Shape::replace(qint32 index, const QPoint &p)
{
    QPolygon::replace(index, p);
    updateMetrics();
}

void Shape::setName(const QString &name) { _name = name; }

void Shape::setUpdateMetrics(bool updateMetrics_)
{
    _updateMetrics = updateMetrics_;
    if (_updateMetrics)
    {
        updateMetrics();
    }
}

Shape &Shape::operator<<(const QPoint point)
{
    append(point);
    return *this;
}

QString Shape::defaultName() { return "<default>"; }

Shape::Unique Shape::reduceToUnique(const QList<Shape> &shapes)
{
    Unique unique;
    for (QList<Shape>::ConstIterator i_shape = shapes.begin(); i_shape != shapes.end(); ++i_shape)
    {
        if (!unique.names.contains(i_shape->name()))
        {
            unique.shapes << *i_shape;
            unique.names << i_shape->name();
            unique.amounts[i_shape->name()] = 1;
        }
        else
        {
            ++unique.amounts[i_shape->name()];
        }
    }
    return unique;
}

QTextStream &operator<<(QTextStream &stream, const Shape &shape)
{
    if (stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL("QTextStream status is not ok");
        return stream;
    }
    stream << "shape_begin"
           << " ";
    BakeryHelpers::writeText(stream, shape.name());
    qint32 size;
    if (shape.isClosed())
    {
        // Shape is closed - ignore last point
        size = shape.size() - 1;
    }
    else
    {
        // Shape is not closed - we need to transfer all points
        size = shape.size();
    }
    stream << size << " ";
    for (qint32 i = 0; i < size; ++i)
    {
        stream << shape[i].x() << " " << shape[i].y() << " ";
    }
    stream << "shape_end"
           << " ";
    return stream;
}

QTextStream &operator>>(QTextStream &stream, Shape &shape)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    QString input;
    QString name;
    bool ok;
    qint32 numPoints;

    // Initializer
    stream >> input;
    if (input != "shape_begin")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-shape into a shape (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Shape name
    name = BakeryHelpers::readText(stream, &ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read name");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }
    shape = Shape(name);

    // Number of shapes
    stream >> input;
    numPoints = input.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read number of shapes");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Read points
    shape.setUpdateMetrics(false);
    for (qint32 i = 0; i < numPoints; ++i)
    {
        qreal x;
        qreal y;

        // x
        stream >> input;
        x = input.toDouble(&ok);
        if (Q_UNLIKELY(!ok))
        {
            BAKERY_CRITICAL("Can not read x in iteration" << (i + 1));
            stream.setStatus(QTextStream::ReadCorruptData);
            return stream;
        }

        // y
        stream >> input;
        y = input.toDouble(&ok);
        if (Q_UNLIKELY(!ok))
        {
            BAKERY_CRITICAL("Can not read y in iteration" << (i + 1));
            stream.setStatus(QTextStream::ReadCorruptData);
            return stream;
        }

        shape << QPoint(x, y);
    }
    shape.ensureClosed();
    shape.setUpdateMetrics(true);

    // Finalizer
    stream >> input;
    if (input != "shape_end")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-shape into a shape (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }
    return stream;
}

bool operator==(const Shape &left, const Shape &right)
{
    if (left.name() != right.name())
    {
        return false;
    }

    return (QPolygon)left == (QPolygon)right;
}
