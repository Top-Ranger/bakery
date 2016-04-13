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

#ifndef BAKERY_HELPERS_H
#define BAKERY_HELPERS_H

#include "global.h"

#include "qmath.h"
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <QLine>
#include <random>

namespace BakeryHelpers
{
/*!
 * \brief Returns an integer representation of a real number. See BAKERY_PRECISION.
 * \param r Real number.
 * \return Integer representation.
 */
inline qint32 qrealPrecise(qreal r) { return (qint32)(r * BAKERY_PRECISION); }

/*!
 * \brief Returns a long integer representation of a real number. See BAKERY_PRECISION.
 * \param r Real number.
 * \return Long integer representation.
 */
inline qint64 qrealPreciseLong(qreal r) { return (qint64)(r * BAKERY_PRECISION); }

/*!
 * \brief Returns a real representation of an integer. See BAKERY_PRECISION.
 * \param i Integer.
 * \return Real representation.
 */
inline qreal qrealRounded(qint32 i) { return ((qreal)i / BAKERY_PRECISION); }

/*!
 * \brief Returns a real representation of a long integer. See BAKERY_PRECISION.
 * \param i Long integer.
 * \return Real representation.
 */
inline qreal qrealRoundedLong(qint64 i) { return ((qreal)i / BAKERY_PRECISION); }

/*!
 * \brief Same as QPoint(qrealPrecise(p.x()), qrealPrecise(p.y())).
 * \param p Point.
 * \return Integer representation of point.
 */
inline QPoint qPointPrecise(const QPointF p) { return QPoint(qrealPrecise(p.x()), qrealPrecise(p.y())); }

/*!
 * \brief Same as QPointF(qrealRounded(p.x()), qrealRounded(p.y())).
 * \param p Point.
 * \return Real representation of point.
 */
inline QPointF qPointRounded(const QPoint p) { return QPointF(qrealRounded(p.x()), qrealRounded(p.y())); }

/*!
 * \brief Computes \f$GCD(a, b)\f$.
 * \param a Positive integer.
 * \param b Positive integer.
 * \return Greatest common divisor of a and b.
 * Iterative implementation of Stein's algorithm. See https://en.wikipedia.org/wiki/Binary_GCD_algorithm.
 */
inline qint32 binaryGCD(qint32 a, qint32 b)
{
    qint32 s;
    if (a == 0)
    {
        return b;
    }
    if (b == 0)
    {
        return a;
    }
    for (s = 0; ((a | b) & 1) == 0; ++s)
    {
        a >>= 1;
        b >>= 1;
    }
    while ((a & 1) == 0)
    {
        a >>= 1;
    }
    do
    {
        while ((b & 1) == 0)
        {
            b >>= 1;
        }
        if (a > b)
        {
            // XOR swap
            a = a ^ b;
            b = a ^ b;
            a = a ^ b;
        }
        b = b - a;
    } while (b != 0);
    return a << s;
}

/*!
 * \brief Computes \f$GCD(xs_0, xs_1, ..., xs_n)\f$.
 * \param xs List of positive integers. No range check is performed.
 * \return Greatest common divisor of a and b of \f$xs_0, xs_1, ..., xs_n\f$.
 */
inline qint32 binaryGCD(const QList<qint32> &xs)
{
    if (xs.size() == 0)
    {
        BAKERY_WARNING("Empty list");
        return 0;
    }
    qint32 gcd = xs[0];
    for (qint32 i = 1; i < xs.size(); ++i)
    {
        // GCD is associative
        gcd = binaryGCD(gcd, xs[i]);
    }
    return gcd;
}

/*!
 * \brief Computes the angle between two lines.
 * \param line1 First line.
 * \param line2 Second line.
 * \return
 */
inline qreal linesAngle(const QLine &line1, const QLine &line2)
{
    qreal len1 = qSqrt(line1.dx() * line1.dx() + line1.dy() * line1.dy());
    qreal len2 = qSqrt(line2.dx() * line2.dx() + line2.dy() * line2.dy());
    qreal cos = (line1.dx() * line2.dx() + line1.dy() * line2.dy()) / (len1 * len2);
    return cos >= qreal(-1.0) && cos <= qreal(1.0) ? qAcos(cos) : 0;
}

/*!
 * \brief Reads text from a QTextStream. See format specification.
 * \param stream Stream.
 * \param ok true if successful.
 * \return Text.
 */
inline QString readText(QTextStream &stream, bool *ok = NULL)
{
    if (ok != NULL)
    {
        *ok = false;
    }

    QString result;
    QString input;

    if (stream.status() != QTextStream::Ok)
    {
        return result;
    }

    stream >> input;
    if (input != "text_begin")
    {
        return result;
    }

    forever
    {
        if (stream.atEnd())
        {
            return result;
        }
        stream >> input;
        if (input == "text_end")
        {
            break;
        }
        else
        {
            result.append(" ");
            result.append(input);
        }
    }

    if (ok != NULL)
    {
        *ok = true;
    }

    result = result.trimmed();
    if (result.isEmpty())
    {
        *ok = false;
    }
    return result;
}

/*!
 * \brief Writes text to a QTextStream. See format specification.
 * \param stream Stream.
 * \param text Text.
 * \return true if successful.
 */
inline bool writeText(QTextStream &stream, QString text)
{
    if (text.contains("text_begin") || text.contains("text_end"))
    {
        return false;
    }
    stream << "text_begin"
           << " ";
    stream << text << " ";
    stream << "text_end"
           << " ";
    return true;
}

/*!
 * \brief Computes the orientation of vectors pq and pr.
 * \param p Vector p.
 * \param q Vector q.
 * \param r Vector r.
 * \return 0: collinear, 1: clockwise, 2: counter-clockwise.
 */
inline int vectorOrientation(QPoint p, QPoint q, QPoint r)
{
    qint64 v = (qint64)(q.y() - p.y()) * (qint64)(r.x() - q.x()) - (qint64)(q.x() - p.x()) * (qint64)(r.y() - q.y());
    if (v == 0)
    {
        return 0;
    }
    return (v > 0) ? 1 : 2;
}
}

#endif // BAKERY_HELPERS_H
