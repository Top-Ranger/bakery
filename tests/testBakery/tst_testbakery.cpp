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

#include <bakery.h>

#include <QString>
#include <QtTest>
#include <QtCore/qmath.h>
#include <QBuffer>
#include <QPainterPath>
#include <QTimer>

// Convenience
typedef QPoint P;
#define V(x) BakeryHelpers::qrealPrecise(x)

Q_DECLARE_METATYPE(PluginInput)
Q_DECLARE_METATYPE(PluginOutput)

void emptyMessageHandler(QtMsgType, const QMessageLogContext &, const QString &) {}

class TestBakery : public QObject
{
    Q_OBJECT

public:
    TestBakery();

private Q_SLOTS:
    void initTestCase();
    void loadDevice_data();
    void loadDevice();
    void saveDevice_data();
    void saveDevice();
    void saveDeviceInput_data();
    void saveDeviceInput();
    void constructorPluginLoading();
    void bakeSheetsfromInput_data();
    void bakeSheetsfromInput();
    void bakeSheetsfromFile_data();
    void bakeSheetsfromFile();
    void loadSVG_data();
    void loadSVG();
    void isOutputValidForInput_data();
    void isOutputValidForInput();
};

TestBakery::TestBakery() {}

void TestBakery::initTestCase()
{
    // Because the methods tested emit a lot of warnings on invalid files and we
    // want to test invalid files the warning and debug messages are suppressed
    // to make the test output more readable
    qInstallMessageHandler(emptyMessageHandler);
}

void TestBakery::loadDevice_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<PluginInput>("input");
    QTest::addColumn<bool>("ok");

    {
        PluginInput input;
        input.sheetWidth = V(3.5);
        input.sheetHeight = V(3);

        Shape square("Quadrat");
        square << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5)) << P(V(1.5), V(0));
        square.ensureClosed();

        Shape triangle("Dreieck");
        triangle << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5));
        triangle.ensureClosed();

        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(triangle);
        input.shapes.append(triangle);

        QTest::newRow("Valid file") << ":/testBakery/inputFiles/valid.txt" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(3.5);
        input.sheetHeight = V(3);

        Shape square("Quadrat");
        square << P(V(0), V(0)) << P(V(0), V(-1.5)) << P(V(-1.5), V(-1.5)) << P(V(-1.5), V(0));
        square.ensureClosed();

        Shape triangle("Dreieck");
        triangle << P(V(0), V(0)) << P(V(0), V(-1.5)) << P(V(-1.5), V(-1.5));
        triangle.ensureClosed();

        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(triangle);
        input.shapes.append(triangle);

        QTest::newRow("Valid file negative") << ":/testBakery/inputFiles/validNegative.txt" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(3.5);
        input.sheetHeight = V(3);

        Shape square("Quadrat 1");
        square << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5)) << P(V(1.5), V(0));
        square.ensureClosed();

        Shape triangle("Dreieck 2");
        triangle << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5));
        triangle.ensureClosed();

        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(triangle);
        input.shapes.append(triangle);

        QTest::newRow("Valid file with spaces in shape name") << ":/testBakery/inputFiles/validSpaces.txt" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(3.5);
        input.sheetHeight = V(3);

        Shape square("Quadrat");
        square << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5)) << P(V(1.5), V(0));
        square.ensureClosed();

        Shape triangle("Dreieck");
        triangle << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5));
        triangle.ensureClosed();

        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(triangle);
        input.shapes.append(triangle);

        QTest::newRow("Valid file") << ":/testBakery/inputFiles/validWithEmptyLine.txt" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(3.5);
        input.sheetHeight = V(3);

        for (qint32 i = 0; i < 50; ++i)
        {
            Shape square(QString("Quadrat%1").arg(i + 1));
            square << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5)) << P(V(1.5), V(0));
            square.ensureClosed();

            input.shapes << square;
        }

        QTest::newRow("Huge file") << ":/testBakery/inputFiles/hugeFile.txt" << input << true;
    }

    QTest::newRow("Non-existent file") << ":/testBakery/inputFiles/THISFILEREALLYEXISTS.txt" << PluginInput() << false;
    QTest::newRow("Missing number of shapes") << ":/testBakery/inputFiles/missingNumShapes.txt" << PluginInput() << false;
    QTest::newRow("Misssing point in shape") << ":/testBakery/inputFiles/missingPoint.txt" << PluginInput() << false;
    QTest::newRow("Missing shape") << ":/testBakery/inputFiles/missingShape.txt" << PluginInput() << false;
    QTest::newRow("Invalid symbol") << ":/testBakery/inputFiles/invalidSymbol.txt" << PluginInput() << false;
    QTest::newRow("Additional point") << ":/testBakery/inputFiles/additionalPoint.txt" << PluginInput() << false;
    QTest::newRow("Wrong number of shapes (too high)") << ":/testBakery/inputFiles/numberShapesTooHigh.txt" << PluginInput() << false;
    QTest::newRow("Completely broken") << ":/testBakery/inputFiles/completelyBroken.txt" << PluginInput() << false;
    QTest::newRow("Unique name used multiple times") << ":/testBakery/inputFiles/doubleName.txt" << PluginInput() << false;
}

void TestBakery::loadDevice()
{
    QFETCH(QString, path);
    QFETCH(PluginInput, input);
    QFETCH(bool, ok);

    bool testOk;

    QFile file(path);
    file.open(QFile::ReadOnly);
    PluginInput created = Bakery::loadFromDevice(&file, &testOk);

    QCOMPARE(ok, testOk);
    QCOMPARE(input, created);
}

void TestBakery::saveDevice_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<PluginOutput>("output");

    {
        Shape square("Quadrat");
        square << P(V(0), V(0)) << P(V(1), V(0)) << P(V(1), V(1)) << P(V(0), V(1));
        square.ensureClosed();

        PluginOutput output;
        Sheet s(V(2), V(2));
        s << square;
        output.sheets << s;

        QTest::newRow("1 shape, 1 sheet") << ":/testBakery/outputFiles/correctOneSheet.txt" << output;
    }

    {
        Shape square("Quadrat");
        square << P(V(0), V(0)) << P(V(1), V(0)) << P(V(1), V(1)) << P(V(0), V(1));
        square.ensureClosed();

        PluginOutput output;
        Sheet s(V(2), V(2));
        s << square;
        output.sheets << s << s;

        QTest::newRow("1 shape, 2 sheets") << ":/testBakery/outputFiles/correctTwoSheets.txt" << output;
    }

    {
        Shape square("Quadrat");
        square << P(V(0), V(0)) << P(V(1), V(0)) << P(V(1), V(1)) << P(V(0), V(1));
        square.ensureClosed();

        Shape square2("Quadrat");
        square2 << P(V(1), V(1)) << P(V(1), V(2)) << P(V(2), V(2)) << P(V(2), V(1));
        square2.ensureClosed();

        PluginOutput output;
        Sheet s(V(2), V(2));
        s << square << square2;
        output.sheets << s;

        QTest::newRow("2 shapes, 1 sheet") << ":/testBakery/outputFiles/correctTwoShapes.txt" << output;
    }
}

void TestBakery::saveDevice()
{
    QFETCH(QString, path);
    QFETCH(PluginOutput, output);

    QByteArray array;
    QBuffer buffer(&array);
    buffer.open(QIODevice::WriteOnly);

    Bakery::saveToDevice(output, &buffer);
    buffer.close();

    QBuffer bufferTest(&array);
    QFile file(path);
    QVERIFY(bufferTest.open(QIODevice::ReadOnly));
    QVERIFY(file.open(QIODevice::ReadOnly));

    QTextStream streamBuffer(&bufferTest);
    QTextStream streamFile(&file);

    while (!streamBuffer.atEnd() && !streamFile.atEnd())
    {
        QString bufferString;
        QString fileString;

        streamBuffer >> bufferString;
        streamFile >> fileString;

        QCOMPARE(bufferString, fileString);
    }

    QCOMPARE(streamBuffer.atEnd(), streamFile.atEnd());
}

void TestBakery::saveDeviceInput_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<PluginInput>("input");

    {
        PluginInput input;
        input.sheetWidth = V(1);
        input.sheetHeight = V(1);

        Shape shape("shape1");
        shape << P(V(0), V(0)) << P(V(0), V(0.5)) << P(V(0.5), V(0.5)) << P(V(0.5), V(0));
        shape.ensureClosed();

        input.shapes << shape << shape << shape;

        QTest::newRow("One shape type") << ":/testBakery/inputFiles/taskOneType.txt" << input;
    }

    {
        PluginInput input;
        input.sheetWidth = V(1);
        input.sheetHeight = V(1);

        Shape shape("shape1");
        shape << P(V(0), V(0)) << P(V(0), V(0.5)) << P(V(0.5), V(0.5)) << P(V(0.5), V(0));
        shape.ensureClosed();

        Shape shape2("shape2");
        shape2 << P(V(0), V(0)) << P(V(0), V(0.5)) << P(V(0.5), V(0.5)) << P(V(0.5), V(0));
        shape2.ensureClosed();

        input.shapes << shape << shape << shape << shape2;

        QTest::newRow("Two shape types") << ":/testBakery/inputFiles/taskTwoType.txt" << input;
    }

    {
        PluginInput input;
        input.sheetWidth = V(4.8);
        input.sheetHeight = V(3.6);

        QTest::newRow("Float rounding") << ":/testBakery/inputFiles/taskFloatRounding.txt" << input;
    }
}

void TestBakery::saveDeviceInput()
{
    QFETCH(QString, path);
    QFETCH(PluginInput, input);

    QByteArray array;
    QBuffer buffer(&array);
    buffer.open(QIODevice::WriteOnly);

    Bakery::saveToDevice(input, &buffer);
    buffer.close();

    QBuffer bufferTest(&array);
    QFile file(path);
    QVERIFY(bufferTest.open(QIODevice::ReadOnly));
    QVERIFY(file.open(QIODevice::ReadOnly));

    QTextStream streamBuffer(&bufferTest);
    QTextStream streamFile(&file);

    while (!streamBuffer.atEnd() && !streamFile.atEnd())
    {
        QString bufferString;
        QString fileString;

        streamBuffer >> bufferString;
        streamFile >> fileString;

        QCOMPARE(bufferString, fileString);
    }

    QCOMPARE(streamBuffer.atEnd(), streamFile.atEnd());
}

void TestBakery::constructorPluginLoading()
{
    Bakery bakery;

    QVERIFY(bakery.getAllPlugins().size() != 0);
}

void TestBakery::bakeSheetsfromInput_data()
{
    QTest::addColumn<PluginInput>("input");

    {
        PluginInput input;
        input.sheetWidth = V(3.5);
        input.sheetHeight = V(3);

        Shape square("square");
        square << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5)) << P(V(1.5), V(0));
        square.ensureClosed();

        Shape triangle("Dreieck");
        triangle << P(V(0), V(0)) << P(V(0), V(1.5)) << P(V(1.5), V(1.5));
        triangle.ensureClosed();

        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(square);
        input.shapes.append(triangle);
        input.shapes.append(triangle);

        QTest::newRow("Different shapes") << input;
    }

    {
        PluginInput input;
        input.sheetWidth = V(2.0);
        input.sheetHeight = V(2.0);

        Shape small("small");
        small << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));

        for (qint32 i = 0; i < 50; ++i)
        {
            input.shapes << small;
        }

        QTest::newRow("Many small shapes") << input;
    }
}

void TestBakery::bakeSheetsfromInput()
{
    Bakery bakery;
    bakery.setTimeLimit(10000);

    QFETCH(PluginInput, input);

    bool ok;
    PluginOutput output = bakery.computeBestOutput(input, &ok);
    QVERIFY2(ok, "Error while processing");
    QVERIFY2(output.sheets.size() != 0, "No sheets returned");
}

void TestBakery::bakeSheetsfromFile_data()
{
    QTest::addColumn<QString>("path");

    QTest::newRow("Valid") << ":/testBakery/inputFiles/valid.txt";
    QTest::newRow("Valid (negative)") << ":/testBakery/inputFiles/validNegative.txt";
}

void TestBakery::bakeSheetsfromFile()
{
    QFETCH(QString, path);

    Bakery bakery;
    bakery.setTimeLimit(10000);

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));

    bool ok;
    PluginOutput output = bakery.computeBestOutput(&file, &ok);
    QVERIFY2(ok, "Error while processing");
    QVERIFY2(output.sheets.size() != 0, "No sheets returned");
}

void TestBakery::loadSVG_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<PluginInput>("input");
    QTest::addColumn<bool>("ok");

    {
        PluginInput input;
        input.sheetWidth = V(1.0);
        input.sheetHeight = V(1.0);

        Shape shape1("shape1");
        shape1 << P(V(0), V(0)) << P(V(1), V(0)) << P(V(1), V(1)) << P(V(0), V(1));
        shape1.ensureClosed();

        Shape shape2("shape2");
        shape2 << P(V(-0.5), V(0)) << P(V(0), V(0)) << P(V(0), V(1)) << P(V(-0.5), V(1));
        shape2.ensureClosed();

        input.shapes << shape1 << shape2;

        QTest::newRow("<rect>") << ":/testBakery/inputFiles/rect.svg" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(1.0);
        input.sheetHeight = V(1.0);

        Shape shape1("shape1");
        shape1 << P(V(0), V(0)) << P(V(0), V(1)) << P(V(1), V(1)) << P(V(1), V(0));
        shape1.ensureClosed();

        Shape shape2("shape2");
        shape2 << P(V(-0.5), V(0)) << P(V(-0.5), V(1)) << P(V(0), V(1)) << P(V(0), V(0));
        shape2.ensureClosed();

        input.shapes << shape1 << shape2;

        QTest::newRow("<polygon>") << ":/testBakery/inputFiles/polygon.svg" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(1.0);
        input.sheetHeight = V(1.0);

        QPainterPath p;

        p.addEllipse(1, 1, 2, 2);
        QPolygonF polyf = p.toFillPolygon();
        Shape shape1;
        foreach (QPointF p, polyf)
        {
            shape1 << BakeryHelpers::qPointPrecise(p);
        }
        shape1.ensureClosed();

        p = QPainterPath();
        p.addEllipse(1, 1, 1, 1.5);
        polyf = p.toFillPolygon();
        Shape shape2;
        foreach (QPointF p, polyf)
        {
            shape2 << BakeryHelpers::qPointPrecise(p);
        }
        shape2.ensureClosed();

        input.shapes << shape1 << shape2;

        QTest::newRow("<circle>/<ellipse>") << ":/testBakery/inputFiles/ellipse.svg" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(1.0);
        input.sheetHeight = V(1.0);

        QPainterPath p;

        p.addRoundedRect(0, 0, 1, 1, 0.5, 0.5);
        QPolygonF polyf = p.toFillPolygon();
        Shape shape1;
        foreach (QPointF p, polyf)
        {
            shape1 << BakeryHelpers::qPointPrecise(p);
        }
        shape1.ensureClosed();

        p = QPainterPath();
        p.addRoundedRect(1.1, 1.2, 1.3, 1.4, 0.2, 0.3);
        polyf = p.toFillPolygon();
        Shape shape2;
        foreach (QPointF p, polyf)
        {
            shape2 << BakeryHelpers::qPointPrecise(p);
        }
        shape2.ensureClosed();

        input.shapes << shape1 << shape1 << shape1 << shape1 << shape2;

        QTest::newRow("<rect> with rounded corners") << ":/testBakery/inputFiles/rectRound.svg" << input << true;
    }

    {
        PluginInput input;
        input.sheetWidth = V(1.0);
        input.sheetHeight = V(1.0);

        Shape rect("rect");
        rect << P(V(0), V(0)) << P(V(1), V(0)) << P(V(1), V(1)) << P(V(0), V(1));
        rect.ensureClosed();

        for (qint32 i = 0; i < 100; ++i)
        {
            input.shapes << rect;
        }

        QTest::newRow("Huge file") << ":/testBakery/inputFiles/hugeFile.svg" << input << true;
    }

    QTest::newRow("Broken") << ":/testBakery/inputFiles/broken.svg" << PluginInput() << false;
    QTest::newRow("Missing polygon coordinate") << ":/testBakery/inputFiles/polygonMissingCoordinate.svg" << PluginInput() << false;
}

void TestBakery::loadSVG()
{
    QFETCH(QString, path);
    QFETCH(PluginInput, input);
    QFETCH(bool, ok);

    bool testOk;

    QFile file(path);
    file.open(QFile::ReadOnly);
    PluginInput created = Bakery::loadFromSVG(&file, &testOk);

    QCOMPARE(ok, testOk);
    QCOMPARE(input.sheetHeight, created.sheetHeight);
    QCOMPARE(input.sheetWidth, created.sheetWidth);
    QCOMPARE(input.shapes.size(), created.shapes.size());

    QSet<QString> nameSet;

    for (QList<Shape>::Iterator i = input.shapes.begin(), c = created.shapes.begin(); i != input.shapes.end(); ++i, ++c)
    {
        // The name is not important - just compare the points and ensure each name is unique
        QCOMPARE((QPolygon)*i, (QPolygon)*c);
        nameSet << c->name();
    }
    QVERIFY2(nameSet.size() == created.shapes.size(), "Not all shapes have unique name");
}

void TestBakery::isOutputValidForInput_data()
{
    QTest::addColumn<PluginInput>("input");
    QTest::addColumn<PluginOutput>("output");
    QTest::addColumn<bool>("equal");

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        QTest::newRow("No shape") << input << output << true;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        Sheet sheet(V(1), V(1));
        sheet << s;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Untransformed shapes") << input << output << true;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        s.moveTo(50, 50);
        Sheet sheet(V(1), V(1));
        sheet << s;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Moved shapes") << input << output << true;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        s.rotate(P(2, 2), 0.5);
        s.moveTo(50, 50);
        Sheet sheet(V(1), V(1));
        sheet << s;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Rotated and moved shapes") << input << output << true;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s1("shape1");
        s1 << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s1.ensureClosed();

        Shape s2("shape2");
        s2 << P(V(0), V(0)) << P(V(0), V(0.3)) << P(V(0.3), V(0.3));
        s2.ensureClosed();

        input.shapes << s1 << s1 << s1 << s2 << s2;

        Sheet sheet1(V(1.5), V(1));
        sheet1 << s1;

        Sheet sheet2(V(1.5), V(1));
        sheet1 << s2;

        output.sheets << sheet2 << sheet2 << sheet1 << sheet1 << sheet1;

        QTest::newRow("Different correct shapes") << input << output << false;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(3);
        input.sheetWidth = V(3);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        Sheet sheet(V(3), V(3));
        sheet << s;
        s.moveTo(V(2), V(0));
        sheet << s;
        s.moveTo(V(0), V(2));
        sheet << s;

        output.sheets << sheet;

        QTest::newRow("Correct on one sheet") << input << output << true;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s;

        Sheet sheet(V(1), V(1));
        sheet << s;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Too few shapes") << input << output << false;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s << s;

        Sheet sheet(V(1), V(1));
        sheet << s;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Too many shapes") << input << output << false;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        s = Shape("shapeNEW");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1)) << P(V(0.1), V(0));
        s.ensureClosed();

        Sheet sheet(V(1), V(1));
        sheet << s;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Different shapes") << input << output << false;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(1);
        input.sheetWidth = V(1);

        Shape shape("shape");
        shape << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        shape.ensureClosed();

        input.shapes << shape << shape << shape;

        Sheet sheet(V(1.5), V(1));
        sheet << shape;

        output.sheets << sheet << sheet << sheet;

        QTest::newRow("Wrong sheet size") << input << output << false;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(3);
        input.sheetWidth = V(3);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        Sheet sheet(V(3), V(3));
        sheet << s;
        sheet << s;
        sheet << s;

        output.sheets << sheet;

        QTest::newRow("Invalid sheet - overlapping") << input << output << false;
    }

    {
        PluginInput input;
        PluginOutput output;

        input.sheetHeight = V(3);
        input.sheetWidth = V(3);

        Shape s("shape");
        s << P(V(0), V(0)) << P(V(0), V(0.1)) << P(V(0.1), V(0.1));
        s.ensureClosed();

        input.shapes << s << s << s;

        Sheet sheet(V(3), V(3));
        sheet << s;
        sheet << s;
        s.moveTo(V(5), V(5));
        sheet << s;

        output.sheets << sheet;

        QTest::newRow("Invalid sheet - out of bounds") << input << output << false;
    }
}

void TestBakery::isOutputValidForInput()
{
    QFETCH(PluginInput, input);
    QFETCH(PluginOutput, output);
    QFETCH(bool, equal);

    QCOMPARE(Bakery::isOutputValidForInput(input, output), equal);
}

QTEST_MAIN(TestBakery)

#include "tst_testbakery.moc"
