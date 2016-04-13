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
#include <sheet.h>

#include <QString>
#include <QtTest>

// Convenience
#define V(x) BakeryHelpers::qrealPrecise(x)
#define VL(x) BakeryHelpers::qrealPreciseLong(x)
#define P(x, y) QPoint(V(x), V(y))
#define S(x, y) Sheet sheet(V(x), V(y))

Q_DECLARE_METATYPE(Shape)
Q_DECLARE_METATYPE(Sheet)

void emptyMessageHandler(QtMsgType, const QMessageLogContext &, const QString &) {}

class TestSheet : public QObject
{
    Q_OBJECT

public:
    TestSheet();

private Q_SLOTS:
    void isValid_data();
    void isValid();
    void mayPlace_data();
    void mayPlace();
    void shapesArea_data();
    void shapesArea();
    void utilitization_data();
    void utilitization();
    void shapesBoundingRect_data();
    void shapesBoundingRect();
    void area_data();
    void area();
    void availableSpace_data();
    void availableSpace();
    void serialize_data();
    void serialize();
    void deserializeNegative_data();
    void deserializeNegative();
    void deserialize_data();
    void deserialize();
};

TestSheet::TestSheet() {}

void TestSheet::isValid_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<bool>("valid");

    {
        S(1, 1);
        sheet << (Shape() << P(0, 0) << P(0, 1) << P(1, 1) << P(0, 1) << P(0, 0));
        QTest::newRow("Completely filled") << sheet << true;
    }

    {
        S(1, 1);
        sheet << (Shape() << P(0, 0) << P(0, 1.001) << P(1, 1) << P(0, 1) << P(0, 0));
        QTest::newRow("Slightly over-filled") << sheet << false;
    }

    {
        S(10, 10);
        sheet << (Shape() << P(0, 0) << P(0, 4.5) << P(4.5, 4.5) << P(4.5, 0) << P(0, 0));
        sheet << (Shape() << P(5.5, 5.5) << P(5.5, 10) << P(10, 10) << P(5.5, 10) << P(5.5, 5.5));
        QTest::newRow("Two polygons, not overlapping, not connected") << sheet << true;
    }

    {
        S(10, 10);
        sheet << (Shape() << P(0, 0) << P(0, 5.5) << P(5.5, 5.5) << P(0, 5.5) << P(0, 0));
        sheet << (Shape() << P(5, 5) << P(5, 10) << P(10, 10) << P(5, 10) << P(5, 5));
        QTest::newRow("Two polygons, not overlapping, connected") << sheet << true;
    }

    {
        S(2, 2);
        sheet << (Shape() << P(0, 0) << P(0, 1.5) << P(1.5, 1.5) << P(1.5, 0) << P(0, 0));
        sheet << (Shape() << P(0.5, 0.5) << P(0.5, 2) << P(2, 2) << P(2, 0.5) << P(0.5, 0.5));
        QTest::newRow("Two polygons, clearly overlapping") << sheet << false;
    }

    {
        S(10, 10);
        sheet << (Shape() << P(0, 0) << P(0, 5) << P(5.01, 5.01) << P(5, 0) << P(0, 0));
        sheet << (Shape() << P(5, 5) << P(5, 1) << P(10, 10) << P(10, 5) << P(5, 5));
        QTest::newRow("Two polygons, slightly overlapping") << sheet << false;
    }

    {
        S(1, 1);
        sheet << (Shape() << P(0, 0) << P(0, 1) << P(1, 1) << P(0, 0));
        sheet << (Shape() << P(0, 0) << P(0, 1) << P(1, 1) << P(0, 0));
        QTest::newRow("Two polygons, completely overlapping") << sheet << false;
    }
}

void TestSheet::isValid()
{
    QFETCH(Sheet, sheet);
    QFETCH(bool, valid);

    QCOMPARE(sheet.isValid(), valid);
}

void TestSheet::mayPlace_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<bool>("mayPlace");

    {
        S(1, 1);
        Shape shape;
        shape << P(-1, 0) << P(0, 1) << P(1, 1) << P(1, 0) << P(0, 0);
        QTest::newRow("Clearly exceeds bounds") << sheet << shape << false;
    }

    {
        S(1, 1);
        Shape shape;
        shape << P(-0.001, 0) << P(0, 1) << P(1, 1) << P(1, 0) << P(0, 0);
        QTest::newRow("Slightly exceeds bounds") << sheet << shape << false;
    }

    {
        S(1000, 1000);
        Shape shape;
        shape << P(-2, -2) << P(-2, -3) << P(-3, -3) << P(-3, -2) << P(-2, -2);
        QTest::newRow("Out of bounds") << sheet << shape << false;
    }

    {
        S(1, 1);
        Shape shape;
        shape << P(0, 0) << P(0, 0.5) << P(0.5, 0.5) << P(0.5, 0) << P(0, 0);
        sheet << shape;
        QTest::newRow("Completely overlapping") << sheet << shape << false;
    }

    {
        S(10, 10);
        Shape shape;
        sheet << (Shape() << P(4.99, 4.99) << P(4.99, 5) << P(5, 5) << P(5, 4.99) << P(0, 4.99));
        shape << P(0, 0) << P(0, 5) << P(5, 5) << P(5, 0) << P(0, 0);
        QTest::newRow("Slightly overlapping") << sheet << shape << false;
    }
}

void TestSheet::mayPlace()
{
    QFETCH(Sheet, sheet);
    QFETCH(Shape, shape);
    QFETCH(bool, mayPlace);

    QCOMPARE(sheet.mayPlace(shape), mayPlace);
}

void TestSheet::shapesArea_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<qint64>("shapesArea");

    {
        QTest::newRow("Empty") << Sheet(1, 1) << VL(0);
    }

    {
        S(1000, 1000);

        Shape shape("shape");
        shape << P(0, 0) << P(0, 10) << P(10, 10) << P(10, 0);
        shape.ensureClosed();
        sheet << shape;
        QTest::newRow("Single shape") << sheet << VL(100);
    }

    {
        S(3, 3);

        Shape shape("shape1");
        shape << P(0, 0) << P(0, 1) << P(1, 1) << P(1, 0);
        shape.ensureClosed();
        sheet << shape;

        shape = Shape("shape2");
        shape << P(1, 1) << P(1, 2) << P(2, 2) << P(2, 1);
        shape.ensureClosed();
        sheet << shape;

        QTest::newRow("Two shapes") << sheet << VL(2);
    }
}

void TestSheet::shapesArea()
{
    QFETCH(Sheet, sheet);
    QFETCH(qint64, shapesArea);

    QCOMPARE(sheet.shapesArea(), shapesArea);
}

void TestSheet::utilitization_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<qreal>("utilitization");

    {
        S(10, 10);
        QTest::newRow("Empty") << sheet << 0.0;
    }

    {
        S(10, 10);

        Shape shape("shape");
        shape << P(0, 0) << P(0, 5) << P(5, 5) << P(5, 0);
        shape.ensureClosed();
        sheet << shape;

        QTest::newRow("Single shape (25%)") << sheet << 0.25;
    }

    {
        S(10, 10);

        Shape shape("shape");
        shape << P(0, 0) << P(0, 5) << P(5, 5) << P(5, 0);
        shape.ensureClosed();
        sheet << shape;

        shape.moveTo(5, 5);
        sheet << shape;

        QTest::newRow("Two shape (50%)") << sheet << 0.5;
    }

    {
        S(10, 10);

        Shape shape("shape");
        shape << P(0, 0) << P(0, 5) << P(5, 5) << P(5, 0);
        shape.ensureClosed();
        sheet << shape;

        shape.moveTo(5, 5);
        sheet << shape;

        shape.moveTo(0, 5);
        sheet << shape;

        shape.moveTo(5, 0);
        sheet << shape;
        QTest::newRow("Four shapes (100%)") << sheet << 1.0;
    }
}

void TestSheet::utilitization()
{
    QFETCH(Sheet, sheet);
    QFETCH(qreal, utilitization);

    QCOMPARE(sheet.utilitization(), utilitization);
}

void TestSheet::shapesBoundingRect_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<QRect>("boundingRect");

    {
        S(1, 1);
        QTest::newRow("Empty sheet") << sheet << QRect();
    }

    {
        S(1, 1);
        Shape shape("Shape");
        sheet << shape;
        QTest::newRow("Sheet with empty shape") << sheet << QRect();
    }

    {
        S(100, 100);
        Shape shape("shape");
        shape << P(2, 3) << P(10, 0) << P(11, 15) << P(5, 7);
        shape.ensureClosed();
        sheet << shape;
        QTest::newRow("Single shape") << sheet << QRect(P(2, 0), P(11, 15));
    }

    {
        S(100, 100);
        Shape shape("1");
        shape << P(2, 3) << P(10, 0) << P(11, 15) << P(5, 7);
        shape.ensureClosed();
        Shape shape2("2");
        shape2 << P(90, 85) << P(91, 86) << P(75, 74);
        shape2.ensureClosed();
        sheet << shape << shape2;
        QTest::newRow("Two shapes, large distance") << sheet << QRect(P(2, 0), P(91, 86));
    }

    {
        S(100, 100);
        Shape shape("1");
        shape << P(2, 2) << P(2, 3) << P(3, 3) << P(3, 2);
        shape.ensureClosed();
        Shape shape2("2");
        shape2 << P(2, 3) << P(3, 3) << P(3, 4) << P(2, 4);
        shape2.ensureClosed();
        sheet << shape << shape2;
        QTest::newRow("Two shapes, connected") << sheet << QRect(P(2, 2), P(3, 4));
    }

    {
        S(100, 100);
        Shape shape("1");
        shape << P(2, 2) << P(2, 10) << P(10, 10) << P(10, 2);
        shape.ensureClosed();
        Shape shape2("2");
        shape2 << P(5, 5) << P(5, 15) << P(15, 15);
        shape2.ensureClosed();
        sheet << shape << shape2;
        QTest::newRow("Two shapes, overlapping") << sheet << QRect(P(2, 2), P(15, 15));
    }
}

void TestSheet::shapesBoundingRect()
{
    QFETCH(Sheet, sheet);
    QFETCH(QRect, boundingRect);
    QCOMPARE(sheet.shapesBoundingRect(), boundingRect);
}

void TestSheet::area_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<qint64>("area");

    {
        Sheet s(V(100), V(100));
        QTest::newRow("100x100") << s << VL(10000);
    }

    {
        Sheet s(V(50), V(100));
        QTest::newRow("50x100") << s << VL(5000);
    }

    {
        Sheet s(V(100), V(50));
        QTest::newRow("100x50") << s << VL(5000);
    }
}

void TestSheet::area()
{
    QFETCH(Sheet, sheet);
    QFETCH(qint64, area);
    QCOMPARE(sheet.area(), area);
}

void TestSheet::availableSpace_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<qint64>("area");

    {
        S(100, 100);
        QTest::newRow("100x100, no shapes") << sheet << VL(10000);
    }

    {
        S(100, 100);
        Shape shape("50x50");
        shape << P(0, 0) << P(0, 50) << P(50, 50) << P(50, 0);
        shape.ensureClosed();
        sheet << shape;
        QTest::newRow("100x100, 1 shape") << sheet << VL(7500);
    }

    {
        S(100, 100);
        Shape shape("50x50");
        shape << P(0, 0) << P(0, 50) << P(50, 50) << P(50, 0);
        shape.ensureClosed();
        sheet << shape << shape;
        QTest::newRow("100x100, 2 shapes") << sheet << VL(5000);
    }

    {
        S(100, 100);
        Shape shape("50x50");
        shape << P(0, 0) << P(0, 50) << P(50, 50) << P(50, 0);
        shape.ensureClosed();
        sheet << shape << shape << shape << shape;
        QTest::newRow("100x100, no space available") << sheet << VL(0);
    }
}

void TestSheet::availableSpace()
{
    QFETCH(Sheet, sheet);
    QFETCH(qint64, area);

    QCOMPARE(sheet.availableSpace(), area);
}

void TestSheet::serialize_data()
{
    QTest::addColumn<Sheet>("sheet");
    QTest::addColumn<QString>("serialized");

    {
        Sheet sheet(V(5.5), V(2.5));
        QTest::newRow("Empty sheet") << sheet << "sheet_begin 550000 250000 0 sheet_end ";
    }

    {
        Sheet sheet(V(5.5), V(2.5));
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        sheet << s;
        QTest::newRow("Sheet with 1 square") << sheet << "sheet_begin 550000 250000 1 shape_begin text_begin testshape text_end 4 0 0 0 "
                                                         "10000 10000 10000 10000 0 shape_end sheet_end ";
    }

    {
        Sheet sheet(V(5.5), V(2.5));
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        sheet << s;
        s.moveTo(V(1), V(1));
        sheet << s;
        QTest::newRow("Sheet with 2 squares") << sheet << "sheet_begin 550000 250000 2 shape_begin text_begin testshape text_end 4 0 0 0 "
                                                          "10000 10000 10000 10000 0 shape_end shape_begin text_begin testshape text_end 4 "
                                                          "100000 100000 100000 110000 110000 110000 110000 100000 shape_end sheet_end ";
    }
}

void TestSheet::serialize()
{
    QFETCH(Sheet, sheet);
    QFETCH(QString, serialized);

    QString testString;
    QTextStream stream(&testString, QIODevice::WriteOnly);
    stream << sheet;
    QCOMPARE(testString, serialized);
}

void TestSheet::deserialize_data()
{
    QTest::addColumn<QString>("serialized");
    QTest::addColumn<Sheet>("sheet");

    {
        S(5.5, 2.5);
        QTest::newRow("Empty sheet") << "sheet_begin 550000 250000 0 sheet_end " << sheet;
    }

    {
        S(5.5, 2.5);
        Shape shape("testshape");
        shape << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        shape.ensureClosed();
        sheet << shape;
        QTest::newRow("Sheet with 1 square") << "sheet_begin 550000 250000 1 shape_begin text_begin testshape text_end 4 0 0 0 10000 10000 "
                                                "10000 10000 0 shape_end sheet_end " << sheet;
    }

    {
        S(5.5, 2.5);
        Shape shape("testshape");
        shape << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        shape.ensureClosed();
        sheet << shape;
        shape.moveTo(V(1), V(1));
        sheet << shape;
        QTest::newRow("Sheet with 2 squares")
            << "sheet_begin 550000 250000 2 shape_begin text_begin testshape text_end 4 0 0 0 10000 10000 10000 10000 0 "
               "shape_end shape_begin text_begin testshape text_end 4 100000 100000 100000 110000 110000 110000 110000 100000 shape_end "
               "sheet_end " << sheet;
    }
}

void TestSheet::deserialize()
{
    QFETCH(QString, serialized);
    QFETCH(Sheet, sheet);

    QTextStream stream(&serialized, QIODevice::ReadOnly);
    Sheet newSheet;
    stream >> newSheet;
    QCOMPARE(newSheet.width(), sheet.width());
    QCOMPARE(newSheet.height(), sheet.height());
    QCOMPARE(newSheet.size(), sheet.size());

    for (qint32 i = 0; i < sheet.size(); ++i)
    {
        QCOMPARE(newSheet.shapes()[i].name(), sheet.shapes()[i].name());
        QCOMPARE(newSheet.shapes()[i], sheet.shapes()[i]);
    }

    QVERIFY2(stream.status() == QTextStream::Ok, "QTextStream status is not QTextStream::Ok");
}

void TestSheet::deserializeNegative_data()
{
    QTest::addColumn<QString>("serialized");

    QTest::newRow("Missing initializer")
        << "5.5 2.5 1 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end sheet_end ";
    QTest::newRow("Misspelled initializer")
        << "shööt_begin 5.5 2.5 1 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end sheet_end ";
    QTest::newRow("Missing finalizer")
        << "sheet_begin 5.5 2.5 1 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end";
    QTest::newRow("Misspelled finalizer")
        << "sheet_begin 5.5 2.5 1 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end shööt_end ";
    QTest::newRow("Wrong number of values")
        << "sheet_begin 5.5 2.5 2 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end sheet_end ";
    QTest::newRow("Correct absolute number of values")
        << "sheet_begin 5.5 2.5 -1 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end sheet_end ";
    QTest::newRow("Single invalid value")
        << "sheet_begin 5.5 float 1 shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end sheet_end ";
    QTest::newRow("Multiple invalid values")
        << "sheet_begin float float int shape_begin text_begin testshape text_end 4 0 0 0 0.1 0.1 0.1 0.1 0 shape_end sheet_end ";
}

void TestSheet::deserializeNegative()
{
    qInstallMessageHandler(emptyMessageHandler);

    QFETCH(QString, serialized);

    QTextStream stream(&serialized, QIODevice::WriteOnly);
    Sheet sheet;
    stream >> sheet;
    QVERIFY2(stream.status() != QTextStream::Ok, "QTextStream status is QTextStream::Ok");

    qInstallMessageHandler(0);
}

QTEST_APPLESS_MAIN(TestSheet)

#include "tst_testsheet.moc"
