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

#ifndef SHAPE_H
#define SHAPE_H

#include "global.h"

#include <QPolygon>
#include <QTransform>
#include <QString>
#include <QTextStream>

/*!
 * \brief Used to store and manipulate shapes. A shape represents a closed polygonal chain whose vertices are stored.
 */
class BAKERYSHARED_EXPORT Shape : public QPolygon
{
public:
    /*!
     * \brief Used for storing the output of Shape::reduceToUnique().
     * \sa Shape::Unique reduceToUnique(const QList<Shape> &shapes)
     */
    struct Unique
    {
        /*!
         * \brief Unique Shape names.
         */
        QList<QString> names;

        /*!
         * \brief Unique Shapes (by name).
         */
        QList<Shape> shapes;

        /*!
         * \brief Amount of Shapes (by name).
         */
        QMap<QString, qint32> amounts;
    };

    /*!
     * \brief Copy constructor.
     * \param other Shape to be copied.
     */
    Shape(const Shape &other);

    /*!
     * \brief Convenience constructor.
     * \param polygon Polygon whose points to use.
     */
    Shape(const QPolygon &polygon);

    /*!
     * \brief Constructor.
     * \param name Shape name. Defaults to Shape::defaultName().
     * \sa Shape::defaultName()
     */
    Shape(QString name = defaultName());

    /*!
     * \brief Appends a point to the shape.
     * \param p Point.
     */
    void append(const QPoint &p);

    /*!
     * \brief Normalizes the Shape's coordinates.
     *
     * Normalization is done by translating the Shape so that its bounding box's
     * top left coordinate is (0, 0).
     */
    void normalize();

    /*!
     * \brief Returns a normalized version of the Shape, i.e. a copy on which normalize() was called().
     * \return Normalized shape
     */
    Shape normalized() const;

    /*!
     * \brief Moves the Shape so that the top left coordinate of its bounding box is (x, y).
     * \param x x coordinate.
     * \param y y coordinate.
     */
    void moveTo(qint32 x, qint32 y);

    /*!
     * \brief Same as moveTo(p.x(), p.y()).
     * \param p Point.
     * \sa Shape::moveTo(qint32 x, qint32 y)
     */
    void moveTo(const QPoint &p);

    /*!
     * \brief Moves the Shape so that the top left coordinate is moved by (dx, dy).
     * \param dx x delta.
     * \param dy y delta.
     */
    void translate(qint32 dx, qint32 dy);

    /*!
     * \brief Same as translate(p.x(), p.y()).
     * \param p Delta.
     */
    void translate(const QPoint &p);

    /*!
     * \brief Scales the Shape by (sx, sy).
     * \param sx Horizontal scale.
     * \param sy Vertical scale.
     */
    void scale(qreal sx, qreal sy);

    /*!
     * \brief Returnes as scaled version of the Shape, i.e. a copy on which scale(sx, sy) was called().
     * \param sx Horizontal scale.
     * \param sy Vertical scale.
     * \return Scaled Shape.
     */
    Shape scaled(qreal sx, qreal sy) const;

    /*!
     * \brief Rotates the Shape by angle (in radians) around center.
     * \param center Rotation center.
     * \param angle Rotation angle (in radians).
     */
    void rotate(QPoint center, qreal angle);

    /*!
     * \brief Returns a rotated version of the Shape, i.e. a copy on which rotate(center, angle) was called().
     * \param center Rotation center.
     * \param angle Rotation angle (in radians).
     * \return Rotated Shape.
     */
    Shape rotated(QPoint center, qreal angle) const;

    /*!
     * \brief Checks if this Shape intersects with another Shape.
     * \param other Shape to check against.
     * \return Whether the Shapes intersect.
     */
    bool intersects(const Shape &other) const;

    /*!
     * \brief Returns the set of inner distances computed by Shape::updateMetrics().
     * \return Set of inner distances.
     * \sa Shape::updateMetrics()
     */
    QSet<qint32> innerDistances() const;

    /*!
     * \brief Returns the list of edges computed by Shape::updateMetrics().
     * \return List of edges.
     * \sa Shape::updateMetrics()
     */
    QList<QLine> edges() const;

    /*!
     * \brief Returns the Shape's convex hull.
     *
     * Implements the gift wrapping algorithm (Jarvis march). See https://en.wikipedia.org/wiki/Gift_wrapping_algorithm.
     * \return Convex hull.
     */
    Shape convexHull() const;

    /*!
     * \brief Ensures that the Shape is (not) closed by re-adding the first or removing the last point.
     * \param closed Whether the Shape should be closed.
     */
    void ensureClosed(bool closed = true);

    /*!
     * \brief Checks if this Shape's inverted version equals the other Shape's inverted version. They are then considered to be congruent.
     * \param other Shape to check against.
     * \return Whether the Shapes are congruent.
     * \sa Shape::invert()
     */
    bool isCongruent(Shape &other);

    /*!
     * \brief Checks if the Shape is closed, i.e. if the first equals the last point.
     * \return true if closed.
     */
    bool isClosed() const;

    /*!
     * \brief Applies the inverse transformation matrix.
     */
    void invert();

    /*!
     * \brief Returns an inverted version of the Shape, i.e. a copy on which invert() was called().
     * \return Inverted version.
     */
    Shape inverted() const;

    /*!
     * \brief Getter for member _simple.
     * \return Whether the Shape is simple.
     * \sa Shape::updateMetrics()
     */
    bool isSimple() const;

    /*!
     * \brief Same as qAbs(_unsignedArea). It may be incorrect if the polygon is not simple.
     * \return Shape's unsigned area.
     * \sa Shape::updateMetrics()
     * \sa Shape::isSimple()
     */
    qint64 area() const;

    /*!
     * \brief Getter for member _signedArea. It may be incorrect if the polygon is not simple.
     * \return Shape's signed area.
     * \sa Shape::updateMetrics()
     * \sa Shape::isSimple()
     */
    qint64 signedArea() const;

    /*!
     * \brief Getter for member _centroid. It may be incorrect if the polygon is not simple.
     * \return Shape's centroid.
     * \sa Shape::updateMetrics()
     * \sa Shape::isSimple()
     */
    QPoint centroid() const;

    /*!
     * \brief Returns this Shape's bounding box's top left coordinate.
     * \return Position of this Shape.
     */
    QPoint position() const;

    /*!
     * \brief Getter for member _name.
     * \return Name of this Shape.
     */
    QString name() const;

    /*!
     * \brief Replaces the point at offset i with p.
     * \param i Offset.
     * \param p New point.
     */
    void replace(qint32 i, const QPoint &p);

    /*!
     * \brief Setter for member _name.
     * \param name Name of this Shape.
     */
    void setName(const QString &name);

    /*!
     * \brief Setter for member _updateMetrics. If _updateMetrics is true Shape::updateMetrics() will be called every time the Shape is
     * modified.
     *
     * It is encouraged to set this to false when adding many points to the Shape without using the metrics in between.
     * \param updateMetrics_ If true Shape::updateMetrics() is immediately called.
     * \sa Shape::updateMetrics()
     */
    void setUpdateMetrics(bool updateMetrics_);

    /*!
     * \brief Appends a point to this Shape.
     * \param point Point.
     * \return Reference to Shape.
     */
    Shape &operator<<(const QPoint point);

    /*!
     * \brief Default Shape name.
     * \return Default Shape name.
     */
    static QString defaultName();

    /*!
     * \brief Used to populate the struct Shape::Unique.
     * \param shapes Shapes to be considered.
     * \return Unique shapes, their names and amounts.
     */
    static Unique reduceToUnique(const QList<Shape> &shapes);

    /*!
     * \brief Compares Shapes' sizes, i.e. their number of points.
     * \param s1 First Shape.
     * \param s2 Second Shape.
     * \return Result of comparison.
     */
    static inline bool lessThanBySize(const Shape &s1, const Shape &s2) { return s1.size() < s2.size(); }

    /*!
     * \brief Compares Shapes' signed area.
     * \param s1 First Shape.
     * \param s2 Second Shape.
     * \return Result of comparison.
     */
    static inline bool lessThanBySignedArea(const Shape &s1, const Shape &s2) { return s1.signedArea() < s2.signedArea(); }

    /*!
     * \brief Compares Shapes' signed area (descending order).
     * \param s1 First Shape.
     * \param s2 Second Shape.
     * \return Result of comparison.
     */
    static inline bool lessThanBySignedAreaDesc(const Shape &s1, const Shape &s2) { return s1.signedArea() > s2.signedArea(); }

    /*!
     * \brief Compares Shapes' unsigned area.
     * \param s1 First Shape.
     * \param s2 Second Shape.
     * \return Result of comparison.
     */
    static inline bool lessThanByArea(const Shape &s1, const Shape &s2) { return s1.area() < s2.area(); }

    /*!
     * \brief Compares Shapes' unsigned area (descending order).
     * \param s1 First Shape.
     * \param s2 Second Shape.
     * \return Result of comparison.
     */
    static inline bool lessThanByAreaDesc(const Shape &s1, const Shape &s2) { return s1.area() > s2.area(); }

private:
    /*!
     * \brief Shape name.
     */
    QString _name;

    /*!
     * \brief true if the Shape is simple, i.e. not complex.
     * \sa updateMetrics()
     */
    bool _simple;

    /*!
     * \brief Shape edges.
     * \sa updateMetrics()
     */
    QList<QLine> _edges;

    /*!
     * \brief Shape's signed area.
     * \sa updateMetrics()
     */
    qint64 _signedArea;

    /*!
     * \brief Shape's centroid.
     * \sa updateMetrics()
     */
    QPoint _centroid;

    /*!
     * \brief Transformation matrix. Is updated when calling transform(QTransform). Its inverse is applied to invert all Shape
     * transformations.
     * \sa transform(QTransform);
     */
    QTransform _transform;

    /*!
     * \brief Shape's inner distances, i.e. all distances between all orthogonally projected Shape points.
     * \sa updateMetrics()
     */
    QSet<qint32> _innerDistances;

    /*!
     * \brief If false updateMetrics() immediately returns. It is encourated to set this to false if adding many points without using
     * dependent values for effiency.
     */
    bool _updateMetrics;

    /*!
     * \brief Computes the shape's metrics.
     *
     * Computes edges, signed area and centroid and performs a naive simplicity
     * test. Called whenever the shape is changed unless _updateMetrics is false.
     * The following formulas are used:\n
     * \f$A = \frac{1}{2}\sum_{i = 0}^{n - 1}(x_i y_{i + 1} - x_{i + 1} y_i)\f$\n
     * \f$C_x = \frac{1}{6A}\sum_{i = 0}^{n - 1}(x_i x_{i + 1})(x_i y_{i + 1} - x_{i
     * + 1} y_i)\f$\n
     * \f$C_y = \frac{1}{6A}\sum_{i = 0}^{n - 1}(y_i y_{i + 1})(x_i y_{i + 1} - x_{i
     * + 1} y_i)\f$\n
     * where \f$(x_i, y_i)\f$ are the polygon's vertices, \f$A\f$ is the signed area
     * and \f$(C_x, C_y)\f$ is the polygon's centroid.
     * \sa Shape::setUpdateMetrics()
     */
    void updateMetrics();

    /*!
     * \brief Applies the given transform to the Shape and updates the transformation matrix.
     * \param transform Transform to be applied.
     */
    void transform(QTransform transform);

    /*!
     * \brief Returns a transformed version of the Shape, i.e. a copy on which transform(transform) was called.
     * \param transform Transform to be applied.
     * \return Transformed version.
     */
    Shape transformed(QTransform transform);

    /*!
     * \brief Hidden.
     */
    void swap(QPolygon &other) { QPolygon::swap(other); }

    /*!
     * \brief Hidden.
     */
    void translate();
};

/*!
 * \relates Shape
 * \brief Streaming operator. See format specification.
 * \param stream Stream.
 * \param shape Shape.
 * \return
 */
BAKERYSHARED_EXPORT QTextStream &operator<<(QTextStream &stream, const Shape &shape);

/*!
 * \relates Shape
 * \brief Streaming operator. See format specification.
 * \param stream Stream.
 * \param shape Shape.
 * \return Stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator>>(QTextStream &stream, Shape &shape);

/*!
 * \relates Shape
 * \brief Two Shapes are considered to be equal when all of the following conditions are met:
 * - Their names are equal.
 * - Their points are pairwise equal.
 * \param left First Shape.
 * \param right Second Shape.
 * \return Whether the Shapes are equal.
 */
BAKERYSHARED_EXPORT bool operator==(const Shape &left, const Shape &right);

/*!
 * \relates Shape
 * \brief Same as !(left == right).
 * \param left First Shape.
 * \param right Second Shape.
 * \return Whether the Shapes are not equal.
 */
inline bool operator!=(const Shape &left, const Shape &right) { return !(left == right); }

#endif // SHAPE_H
