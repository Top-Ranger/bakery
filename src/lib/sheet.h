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

#ifndef SHEET_H
#define SHEET_H

#include "global.h"

#include "shape.h"

/*!
 * \brief Used to store and manipulate sheets. A sheet is a two-dimensional rectangular container for storing shapes.
 */
class BAKERYSHARED_EXPORT Sheet
{
public:
    /*!
     * \brief Convenience iterator.
     */
    typedef QList<Shape>::Iterator Iterator;

    /*!
     * \brief Convenience iterator.
     */
    typedef QList<Shape>::iterator iterator;

    /*!
     * \brief Convenience iterator.
     */
    typedef QList<Shape>::ConstIterator ConstIterator;

    /*!
     * \brief Convenience iterator.
     */
    typedef QList<Shape>::const_iterator const_iterator;

    /*!
     * \brief Constructor. Sets _width = 1, _height = 1, _bounds = QRect(0, 0, 1, 1).
     */
    Sheet();

    /*!
     * \brief Constructor. Sets _bounds = QRect(0, 0, width, height).
     * \param width Sheet width.
     * \param height Sheet height.
     */
    Sheet(qint32 width, qint32 height);

    /*!
     * \brief Appends a Shape to the sheet.
     * \param shape Shape.
     */
    void append(const Shape &shape);

    /*!
     * \brief Computes the sum of all Shapes' unsigned areas.
     * \return Sum of Shape areas.
     */
    qint64 shapesArea() const;

    /*!
     * \brief Same as area() - shapesArea().
     * \return Available space.
     */
    qint64 availableSpace() const;

    /*!
     * \brief Same as (qreal)shapesArea() / area().
     * \return Utiltization.
     */
    qreal utilitization() const;

    /*!
     * \brief Pairwise Shapes edges intersection test.
     * \return true if no Shapes intersect.
     */
    bool isValid() const;

    /*!
     * \brief Creates a copy of this Sheet, appends the Shape and checks validity.
     * \param shape Shape.
     * \return true if the Shape may be placed at its position
     * \sa Shape.position()
     */
    bool mayPlace(Shape &shape) const;

    /*!
     * \brief Returns a reference the the Shapes list.
     * \return Reference to Shapes list.
     */
    QList<Shape> &shapes();

    /*!
     * \brief Same as _shapes.begin()
     * \return Iterator.
     */
    Iterator begin();

    /*!
     * \brief Same as _shapes.end().
     * \return Iterator.
     */
    Iterator end();

    /*!
     * \brief _shapes.constBegin()
     * \return Constant iterator.
     */
    ConstIterator constBegin() const;

    /*!
     * \brief _shapes.constEnd()
     * \return Constant iterator.
     */
    ConstIterator constEnd() const;

    /*!
     * \brief Used to append a Shape to the Sheet.
     * \param shape Shape.
     * \return Reference to this Sheet.
     */
    Sheet &operator<<(const Shape &shape);

    /*!
     * \brief Getter for member _width.
     * \return Sheet width.
     */
    qint32 width() const;

    /*!
     * \brief Getter for member _height.
     * \return Sheet height.
     */
    qint32 height() const;

    /*!
     * \brief Number of shapes on this Sheet.
     * \return Number of shapes on this Sheet.
     */
    qint32 size() const;

    /*!
     * \brief Checks if the Sheet is empty, i.e. when no Shapes have been appended.
     * \return true if the Sheet is empty.
     */
    bool isEmpty() const;

    /*!
     * \brief Getter for member _bounds.
     * \return Sheet bounds.
     */
    QRect boundingRect() const;

    /*!
     * \brief Minimum rectangle that contains all Shapes.
     * \return Minimum rectangle that contains all Shapes.
     */
    QRect shapesBoundingRect() const;

    /*!
     * \brief Convex hull of all Shapes.
     * \return Convex hull of all Shapes.
     * \sa Shape::convexHull()
     */
    Shape shapesHull() const;

    /*!
     * \brief Sheet area.
     * \return Sheet area.
     */
    qint64 area() const;

    /*!
     * \brief Quotient of total shapes area and area of all Shapes' convex hull.
     * \return Sheet density.
     */
    qreal density() const;

private:
    /*!
     * \brief Sheet width.
     */
    qint32 _width;

    /*!
     * \brief Sheet height.
     */
    qint32 _height;

    /*!
     * \brief Sheet bounds, i.e. QRect(QPoint(0, 0), QPoint(_width, _height)).
     */
    QRect _bounds;

    /*!
     * \brief List of contained Shapes. Their positions are stored within Shape.
     * \sa Shape::position()
     */
    QList<Shape> _shapes;
};

/*!
 * \relates Sheet
 * \brief Operator to serialize Sheets. See format specification.
 * \param stream Text stream.
 * \param sheet Sheet.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator<<(QTextStream &stream, const Sheet &sheet);

/*!
 * \relates Sheet
 * \brief Operator to unserialize Sheets. See format specification.
 * \param stream Text stream.
 * \param sheet Sheet.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator>>(QTextStream &stream, Sheet &sheet);

/*!
 * \relates Sheet
 * \brief Operator to compare Sheets. They are considered to be equal if and only if their Shapes are pairwise equal.
 * \param left First Sheet.
 * \param right Second Sheet.
 * \return true if the Sheets are equal.
 */
BAKERYSHARED_EXPORT bool operator==(const Sheet &left, const Sheet &right);

/*!
 * \relates Sheet
 * \brief Inequality operator for Sheets.
 * \param left First Sheet.
 * \param right Second Sheet.
 * \return true if the Sheets are not equal.
 */
inline bool operator!=(const Sheet &left, const Sheet &right) { return !(left == right); }

#endif // SHEET_H
