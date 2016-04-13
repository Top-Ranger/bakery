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

#include "sheet.h"

#include "helpers.hpp"
#include "math.h"

Sheet::Sheet() : _width(1), _height(1), _bounds(QRect(QPoint(0, 0), QPoint(1, 1))) {}

Sheet::Sheet(qint32 width, qint32 height) : _width(width), _height(height), _bounds(QRect(QPoint(0, 0), QPoint(width, height))) {}

void Sheet::append(const Shape &shape) { _shapes << shape; }

qint64 Sheet::shapesArea() const
{
    qreal a = 0;
    for (QList<Shape>::ConstIterator i_shape = _shapes.begin(); i_shape < _shapes.end(); ++i_shape)
    {
        a += i_shape->area();
    }
    return a;
}

qint64 Sheet::availableSpace() const { return area() - shapesArea(); }

qreal Sheet::utilitization() const { return (qreal)shapesArea() / area(); }

bool Sheet::isValid() const
{
    QList<Shape>::ConstIterator i_this = _shapes.end();
    while (i_this != _shapes.begin())
    {
        --i_this;
        if (!_bounds.contains(i_this->boundingRect()))
        {
            return false;
        }
        QList<Shape>::ConstIterator i_other = i_this;
        while (i_other != _shapes.begin())
        {
            --i_other;
            if (i_this->intersects(*i_other))
            {
                return false;
            }
        }
    }
    return true;
}

bool Sheet::mayPlace(Shape &shape) const
{
    Sheet copy(*this);
    copy << shape;
    return copy.isValid();
}

QList<Shape> &Sheet::shapes() { return _shapes; }

Sheet::Iterator Sheet::begin() { return _shapes.begin(); }

Sheet::Iterator Sheet::end() { return _shapes.end(); }

Sheet::ConstIterator Sheet::constBegin() const { return _shapes.constBegin(); }

Sheet::ConstIterator Sheet::constEnd() const { return _shapes.constEnd(); }

Sheet &Sheet::operator<<(const Shape &shape)
{
    _shapes << shape;
    return *this;
}

qint32 Sheet::width() const { return _width; }

qint32 Sheet::height() const { return _height; }

QRect Sheet::boundingRect() const { return _bounds; }

QRect Sheet::shapesBoundingRect() const
{
    QRect bounds;
    for (QList<Shape>::ConstIterator i_shape = _shapes.begin(); i_shape != _shapes.end(); ++i_shape)
    {
        bounds = bounds.united(i_shape->boundingRect());
    }
    return bounds;
}

Shape Sheet::shapesHull() const
{
    QVector<QPoint> allPoints;
    for (ConstIterator i_shape = constBegin(); i_shape != constEnd(); ++i_shape)
    {
        allPoints << *i_shape;
    }
    return Shape(allPoints).convexHull();
}

qint64 Sheet::area() const
{
    // BAKERY_PRECISION is considered twice when computing sheet area
    return (qint64)_width * _height / BAKERY_PRECISION;
}

qreal Sheet::density() const { return (qreal)shapesArea() / (qreal)shapesHull().area(); }

qint32 Sheet::size() const { return _shapes.size(); }

bool Sheet::isEmpty() const { return _shapes.isEmpty(); }

QTextStream &operator<<(QTextStream &stream, const Sheet &sheet)
{
    if (stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    stream << "sheet_begin"
           << " ";
    stream << sheet.width() << " ";
    stream << sheet.height() << " ";
    stream << sheet.size() << " ";
    for (Sheet::ConstIterator i_shapes = sheet.constBegin(); i_shapes != sheet.constEnd(); ++i_shapes)
    {
        stream << *i_shapes;
    }
    stream << "sheet_end"
           << " ";
    return stream;
}

QTextStream &operator>>(QTextStream &stream, Sheet &sheet)
{
    if (stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    QString input;
    bool ok;
    qint32 numShapes;
    qreal width;
    qreal height;

    stream >> input;
    if (input != "sheet_begin")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-Sheet into a Sheet (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Width
    stream >> input;
    width = input.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read width");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Height
    stream >> input;
    height = input.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read height");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Create sheet
    sheet = Sheet(width, height);

    // Number of shapes
    stream >> input;
    numShapes = input.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read number of shapes");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Read shapes
    for (qint32 i = 0; i < numShapes; ++i)
    {
        Shape shape;
        stream >> shape;
        if (stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL("Can not read shape" << (i + 1));
            return stream;
        }
        sheet << shape;
    }

    stream >> input;
    if (input != "sheet_end")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-Sheet into a Sheet (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    return stream;
}

bool operator==(const Sheet &left, const Sheet &right)
{
    if (left.size() != right.size())
    {
        return false;
    }

    for (Sheet::ConstIterator i_left = left.constBegin(), i_right = right.constBegin(); i_left != left.constEnd(); ++i_left, ++i_right)
    {
        if (*i_left != *i_right)
        {
            return false;
        }
    }

    return true;
}
