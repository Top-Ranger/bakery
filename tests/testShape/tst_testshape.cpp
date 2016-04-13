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

#include <helpers.hpp>

#include <shape.h>

#include <QString>
#include <QtTest>
#include <QTextStream>
#include <random>

// Convenience
#define V(x) BakeryHelpers::qrealPrecise(x)
#define VL(x) BakeryHelpers::qrealPreciseLong(x)
#define P(x, y) QPoint(V(x), V(y))

Q_DECLARE_METATYPE(Shape)

void emptyMessageHandler(QtMsgType, const QMessageLogContext &, const QString &) {}

class TestShape : public QObject
{
    Q_OBJECT

public:
    TestShape();

private Q_SLOTS:
    void ensureClosed_data();
    void ensureClosed();
    void translate_data();
    void translate();
    void rotate_data();
    void rotate();
    void equals_data();
    void equals();
    void isCongruent_data();
    void isCongruent();
    void area_data();
    void area();
    void centroid_data();
    void centroid();
    void inverted_data();
    void inverted();
    void moveTo_data();
    void moveTo();
    void intersects_data();
    void intersects();
    void isSimple_data();
    void isSimple();
    void serialize_data();
    void serialize();
    void deserializeNegative_data();
    void deserializeNegative();
    void deserialize_data();
    void deserialize();
    void convexHull_data();
    void convexHull();
};

TestShape::TestShape() {}

void TestShape::ensureClosed_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<qint32>("expectedLength");

    {
        Shape shape;
        shape << P(0, 0) << P(0, 1) << P(1, 1);
        QTest::newRow("Open") << shape << 4;
    }

    {
        Shape shape;
        shape << P(0, 0) << P(0, 1) << P(1, 1) << P(0, 0);
        QTest::newRow("Closed") << shape << 4;
    }
}

void TestShape::ensureClosed()
{
    QFETCH(Shape, shape);
    QFETCH(qint32, expectedLength);

    shape.ensureClosed();
    QVERIFY(shape.isClosed());
    QVERIFY(shape.size() == expectedLength);
}

void TestShape::translate_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<QPoint>("delta");
    QTest::addColumn<Shape>("translated");

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1);
        shape.ensureClosed();
        Shape translated;
        translated << P(1, 1) << P(2, 1) << P(2, 2);
        translated.ensureClosed();
        QTest::newRow("Positive delta") << shape << P(1, 1) << translated;
    }

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1);
        shape.ensureClosed();
        Shape translated;
        translated << P(-1, -1) << P(0, -1) << P(0, 0);
        translated.ensureClosed();
        QTest::newRow("Negative delta") << shape << P(-1, -1) << translated;
    }

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1);
        shape.ensureClosed();
        Shape translated;
        translated << P(-1.5, 1.25) << P(-0.5, 1.25) << P(-0.5, 2.25);
        translated.ensureClosed();
        QTest::newRow("Mixed real delta") << shape << P(-1.5, 1.25) << translated;
    }
}

void TestShape::translate()
{
    QFETCH(Shape, shape);
    QFETCH(QPoint, delta);
    QFETCH(Shape, translated);

    shape.translate(delta);
    QCOMPARE(shape, translated);
}

void TestShape::rotate_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<QPoint>("center");
    QTest::addColumn<qreal>("angle");
    QTest::addColumn<Shape>("rotated");

    {
        Shape shape;
        shape << P(0, 2) << P(1, 0) << P(2, 2) << P(0, 2);
        shape.ensureClosed();
        Shape rotated;
        rotated << P(0, 2) << P(2, 3) << P(0, 4);
        rotated.ensureClosed();
        QTest::newRow("Triangle 90") << shape << P(0, 2) << M_PI_2 << rotated;
    }
}

void TestShape::rotate()
{
    QFETCH(Shape, shape);
    QFETCH(QPoint, center);
    QFETCH(qreal, angle);
    QFETCH(Shape, rotated);

    shape.rotate(center, angle);
    QVERIFY(shape == rotated);
}

void TestShape::equals_data()
{
    QTest::addColumn<Shape>("firstShape");
    QTest::addColumn<Shape>("secondShape");
    QTest::addColumn<bool>("equal");

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1);
        QTest::newRow("Equal shapes") << shape << shape << true;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        Shape secondShape;
        secondShape << P(1, 2) << P(3, 4) << P(5, 6);
        QTest::newRow("Inequal shapes (same length)") << firstShape << secondShape << false;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 0);
        Shape secondShape;
        secondShape << P(0, 0) << P(1, 0) << P(1, 1);
        QTest::newRow("Inequal shapes (same if closed)") << firstShape << secondShape << false;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1) << P(2, 0);
        Shape secondShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        QTest::newRow("Inequal shapes (different length)") << firstShape << secondShape << false;
    }
}

void TestShape::equals()
{
    QFETCH(Shape, firstShape);
    QFETCH(Shape, secondShape);
    QFETCH(bool, equal);

    QCOMPARE(firstShape == secondShape, equal);
}

void TestShape::isCongruent_data()
{
    QTest::addColumn<Shape>("firstShape");
    QTest::addColumn<Shape>("secondShape");
    QTest::addColumn<bool>("congruent");

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1);
        shape.ensureClosed();
        QTest::newRow("Equal shapes") << shape << shape << true;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        firstShape.ensureClosed();
        Shape secondShape(firstShape);
        secondShape.translate(QPoint(1, -1));
        QTest::newRow("Translated") << firstShape << secondShape << true;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        firstShape.ensureClosed();
        Shape secondShape(firstShape);
        secondShape.rotate(QPoint(1, -1), M_PI);
        QTest::newRow("Rotated") << firstShape << secondShape << true;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        firstShape.ensureClosed();
        Shape secondShape(firstShape);
        secondShape.translate(QPoint(1, -1));
        secondShape.rotate(QPoint(1, -1), M_PI);
        QTest::newRow("Translated and rotated") << firstShape << secondShape << true;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        firstShape.ensureClosed();
        Shape secondShape(firstShape);

        std::random_device rnd;

        for (qint32 i = 0; i < 100; i++)
        {
            if (rnd() % 2)
            {
                secondShape.translate(QPoint(1, -1));
            }
            else
            {
                secondShape.rotate(QPoint(1, -1), M_PI);
            }
        }
        QTest::newRow("Randomly translated and rotated") << firstShape << secondShape << true;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        firstShape.ensureClosed();
        Shape secondShape;
        secondShape << P(1, 2) << P(2, 3) << P(3, 4);
        secondShape.ensureClosed();
        QTest::newRow("Not congruent (without transformations)") << firstShape << secondShape << false;
    }

    {
        Shape firstShape;
        firstShape << P(0, 0) << P(1, 0) << P(1, 1);
        firstShape.ensureClosed();
        Shape secondShape;
        secondShape << P(1, 2) << P(2, 3) << P(3, 4);
        secondShape.ensureClosed();
        secondShape.translate(QPoint(1, -1));
        secondShape.rotate(QPoint(1, -1), M_PI);
        QTest::newRow("Not congruent (with transformations)") << firstShape << secondShape << false;
    }
}

void TestShape::isCongruent()
{
    QFETCH(Shape, firstShape);
    QFETCH(Shape, secondShape);
    QFETCH(bool, congruent);

    QCOMPARE(firstShape.isCongruent(secondShape), congruent);
}

void TestShape::area_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<qint64>("area");

    {
        Shape shape;
        shape << P(1, 0) << P(2, 2) << P(0, 2);
        shape.ensureClosed();
        QTest::newRow("Triangle") << shape << VL(2);
    }

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 1);
        shape.ensureClosed();
        QTest::newRow("Square") << shape << VL(1);
    }

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 1);
        shape.ensureClosed();
        shape.rotate(P(2, 2), M_PI_2);
        QTest::newRow("Square (rotated)") << shape << VL(1);
    }
}

void TestShape::area()
{
    QFETCH(Shape, shape);
    QFETCH(qint64, area);

    QCOMPARE(shape.area(), area);
}

void TestShape::centroid_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<QPoint>("centroid");

    {
        Shape shape;
        shape << P(0, 0) << P(0, 6) << P(6, 6);
        shape.ensureClosed();
        QTest::newRow("Triangle") << shape << P(2, 4);
    }

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 1);
        shape.ensureClosed();
        QTest::newRow("Square") << shape << P(0.5, 0.5);
    }

    {
        Shape shape;
        shape << QPoint(0, 0) << QPoint(0, 150000) << QPoint(150000, 150000) << QPoint(150000, 0);
        shape.ensureClosed();
        QTest::newRow("Square (precise, counter-clockwise)") << shape << QPoint(75000, 75000);
    }

    {
        Shape shape;
        shape << QPoint(0, 0) << QPoint(150000, 0) << QPoint(150000, 150000) << QPoint(0, 150000);
        shape.ensureClosed();
        QTest::newRow("Square (precise, clockwise)") << shape << QPoint(75000, 75000);
    }

    {
        Shape shape;
        shape << QPoint(110000, 0) << QPoint(70000, 10000) << QPoint(40000, 20000) << QPoint(20000, 40000) << QPoint(20000, 70000)
              << QPoint(40000, 100000) << QPoint(70000, 110000) << QPoint(110000, 120000) << QPoint(150000, 110000)
              << QPoint(180000, 100000) << QPoint(200000, 70000) << QPoint(200000, 40000) << QPoint(180000, 20000) << QPoint(150000, 10000);
        shape.ensureClosed();
        QTest::newRow("Many edges (precise)") << shape << QPoint(110000, 59715);
    }

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 1);
        shape.ensureClosed();
        for (qint32 i = 0; i < 64; ++i)
        {
            shape.rotate(shape.centroid(), M_PI_4);
        }
        QTest::newRow("Square (rotated around centroid)") << shape << P(0.5, 0.5);
    }
}

void TestShape::centroid()
{
    QFETCH(Shape, shape);
    QFETCH(QPoint, centroid);

    QCOMPARE(shape.centroid(), centroid);
}

void TestShape::inverted_data()
{
    QTest::addColumn<Shape>("inverted");
    QTest::addColumn<Shape>("original");

    Shape original;
    original << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 1) << P(0, 0);

    {
        Shape rotated(original);
        rotated.rotate(rotated.centroid(), 0.1);
        QTest::newRow("Rotated") << rotated.inverted() << original;
    }

    {
        Shape rotated(original);
        QPoint centroid = rotated.centroid();
        qint32 steps = 32;
        for (qint32 i = 0; i < steps; i++)
        {
            rotated.rotate(centroid, 2 * M_PI / steps);
        }
        QTest::newRow("Rotated many times") << rotated.inverted() << original;
    }

    {
        Shape moved(original);
        moved.moveTo(2, 2);
        QTest::newRow("Moved") << moved.inverted() << original;
    }

    {
        Shape moved(original);
        for (qint32 i = 0; i < 100; i++)
        {
            moved.moveTo(i, i);
        }
        QTest::newRow("Moved many times") << moved.inverted() << original;
    }
}

void TestShape::inverted()
{
    QFETCH(Shape, inverted);
    QFETCH(Shape, original);

    QCOMPARE(inverted, original);
}

void TestShape::moveTo_data()
{
    QTest::addColumn<Shape>("source");
    QTest::addColumn<Shape>("target");
    QTest::addColumn<qint32>("x");
    QTest::addColumn<qint32>("y");

    {
        Shape source("source");
        source << P(0, 0) << P(0, 1) << P(1, 1) << P(1, 0);
        source.ensureClosed();

        Shape target("target");
        target << P(1, 1) << P(1, 2) << P(2, 2) << P(2, 1);
        target.ensureClosed();

        QTest::newRow("Move shape") << source << target << V(1) << V(1);
    }

    {
        Shape source("source");
        source << P(0, 0) << P(0, 1) << P(1, 1) << P(1, 0);
        source.ensureClosed();
        source.moveTo(1, 1);

        Shape target("target");
        target << P(1, 1) << P(1, 2) << P(2, 2) << P(2, 1);
        target.ensureClosed();

        QTest::newRow("Move already moved shape") << source << target << V(1) << V(1);
    }

    {
        Shape source("source");
        source << P(0, 2) << P(1, 0) << P(2, 2) << P(0, 2);
        source.ensureClosed();
        source.rotate(P(0, 2), M_PI_2);

        Shape target("target");
        target << P(1, 1) << P(3, 2) << P(1, 3);
        target.ensureClosed();

        QTest::newRow("Move rotated shape") << source << target << V(1) << V(1);
    }
}

void TestShape::moveTo()
{
    QFETCH(Shape, source);
    QFETCH(Shape, target);
    QFETCH(qint32, x);
    QFETCH(qint32, y);

    source.moveTo(x, y);
    QCOMPARE(source.length(), target.length());

    for (qint32 i = 0; i < target.length(); ++i)
    {
        QCOMPARE(source[i], target[i]);
    }
}

void TestShape::intersects_data()
{
    QTest::addColumn<Shape>("shape1");
    QTest::addColumn<Shape>("shape2");
    QTest::addColumn<bool>("isIntersecting");

    QTest::newRow("Not overlapping, not connected") << (Shape() << P(0, 0) << P(0, 4.5) << P(4.5, 4.5) << P(4.5, 0) << P(0, 0))
                                                    << (Shape() << P(5.5, 5.5) << P(5.5, 10) << P(10, 10) << P(5.5, 10) << P(5.5, 5.5))
                                                    << false;

    QTest::newRow("Not overlapping, connected") << (Shape() << P(0, 0) << P(0, 5.5) << P(5.5, 5.5) << P(0, 5.5) << P(0, 0))
                                                << (Shape() << P(5, 5) << P(5, 10) << P(10, 10) << P(5, 10) << P(5, 5)) << false;

    QTest::newRow("Slightly overlapping") << (Shape() << P(0, 0) << P(0, 5) << P(5.01, 5.01) << P(5, 0) << P(0, 0))
                                          << (Shape() << P(5, 5) << P(5, 1) << P(10, 10) << P(10, 5) << P(5, 5)) << true;

    QTest::newRow("Completely overlapping") << (Shape() << P(0, 0) << P(0, 1) << P(1, 1) << P(0, 0))
                                            << (Shape() << P(0, 0) << P(0, 1) << P(1, 1) << P(0, 0)) << true;
}

void TestShape::intersects()
{
    QFETCH(Shape, shape1);
    QFETCH(Shape, shape2);
    QFETCH(bool, isIntersecting);

    QCOMPARE(shape1.intersects(shape2), isIntersecting);
}

void TestShape::isSimple_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<bool>("simple");

    QTest::newRow("Triangle") << (Shape() << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 0)) << true;

    QTest::newRow("Hull") << (Shape() << P(0, 1) << P(0, 2) << P(2, 4) << P(3, 5) << P(5, 7) << P(6, 7) << P(7, 6) << P(6, 5) << P(2, 1)
                                      << P(1, 0) << P(0, 1)) << true;
}

void TestShape::isSimple()
{
    QFETCH(Shape, shape);
    QFETCH(bool, simple);

    QCOMPARE(shape.isSimple(), simple);
}

void TestShape::serialize_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<QString>("serialized");

    {
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        QTest::newRow("Open 0.1 square") << s << "shape_begin text_begin testshape text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end ";
    }

    {
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        QTest::newRow("Closed 0.1 square") << s << "shape_begin text_begin testshape text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end ";
    }

    {
        Shape s("testshape spaces");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        QTest::newRow("Closed 0.1 square name with spaces")
            << s << "shape_begin text_begin testshape spaces text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end ";
    }
}

void TestShape::serialize()
{
    QFETCH(Shape, shape);
    QFETCH(QString, serialized);

    QString testString;
    QTextStream stream(&testString, QIODevice::WriteOnly);
    stream << shape;
    QCOMPARE(testString, serialized);
}

void TestShape::deserialize_data()
{
    QTest::addColumn<QString>("serialized");
    QTest::addColumn<Shape>("shape");

    {
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        QTest::newRow("Open 0.1 square") << "shape_begin text_begin testshape text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end" << s;
        QTest::newRow("Closed 0.1 square") << "shape_begin text_begin testshape text_end 5 0 0 0 10000 10000 10000 10000 0 0 0 shape_end"
                                           << s;
    }

    {
        Shape s("testshape spaces");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        QTest::newRow("Open 0.1 square name with spaces")
            << "shape_begin text_begin testshape spaces text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end" << s;
        QTest::newRow("Closed 0.1 square name with spaces")
            << "shape_begin text_begin testshape spaces text_end 5 0 0 0 10000 10000 10000 10000 0 0 0 shape_end" << s;
    }
}

void TestShape::deserialize()
{
    QFETCH(QString, serialized);
    QFETCH(Shape, shape);

    QTextStream stream(&serialized, QIODevice::ReadOnly);
    Shape newShape;
    stream >> newShape;
    QCOMPARE(newShape, shape);
    QCOMPARE(newShape.name(), shape.name());
    QVERIFY2(stream.status() == QTextStream::Ok, "QTextStream status is not QTextStream::Ok");
}

void TestShape::convexHull_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<Shape>("convexHull");

    QTest::newRow("Empty hull, 0 points") << Shape() << Shape();
    QTest::newRow("Empty hull, 2 points") << (Shape() << P(0, 0) << P(0, 1)) << Shape();

    {
        Shape shape;
        shape << P(0, 0) << P(1, 0) << P(1, 1) << P(0, 1) << P(0, 0) << P(2, 2) << P(3, 2) << P(3, 3) << P(2, 3) << P(2, 2);

        Shape convexHull;
        convexHull << P(0, 0) << P(1, 0) << P(3, 2) << P(3, 3) << P(2, 3) << P(0, 1) << P(0, 0);
        QTest::newRow("Two squares") << shape << convexHull;
    }
}

void TestShape::convexHull()
{
    QFETCH(Shape, shape);
    QFETCH(Shape, convexHull);

    QCOMPARE(shape.convexHull(), convexHull);
}

void TestShape::deserializeNegative_data()
{
    QTest::addColumn<QString>("serialized");

    QTest::newRow("Missing initializer") << "text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end";
    QTest::newRow("Misspelled initializer") << "shapebegin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end";
    QTest::newRow("Missing finalizer") << "shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0";
    QTest::newRow("Misspelled finalizer") << "shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shapeend";
    QTest::newRow("Wrong number of values") << "shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 shape_end";
    QTest::newRow("Wrong format of number of values") << "shape_begin text_begin testshape text_end 4.2 0 0 0 0.1 0.1 0.1 0.1 0 shape_end";
    QTest::newRow("Correct absolute number of values") << "shape_begin text_begin testshape text_end -4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end";
    QTest::newRow("Single invalid value") << "shape_begin text_begin testshape text_end 4 0 0 float 0.1 0.1 0.1 0.1 0 shape_end";
    QTest::newRow("Multiple invalid values")
        << "shape_begin text_begin testshape text_end 4 integer 0 float 0.1 0.1 0.1 fourty-two 0 shape_end";
    QTest::newRow("No text_end") << "shape_begin text_begin testshape 4 integer 0 float 0.1 0.1 0.1 fourty-two 0 shape_end";
}

void TestShape::deserializeNegative()
{
    qInstallMessageHandler(emptyMessageHandler);

    QFETCH(QString, serialized);

    QTextStream stream(&serialized, QIODevice::WriteOnly);
    Shape shape;
    stream >> shape;
    QVERIFY2(stream.status() != QTextStream::Ok, "QTextStream status is QTextStream::Ok");

    qInstallMessageHandler(0);
}

QTEST_APPLESS_MAIN(TestShape)

#include "tst_testshape.moc"
