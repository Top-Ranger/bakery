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
#include <plugins.h>

#include <QString>
#include <QtTest>
#include <QTimer>

// Convenience
#define V(x) BakeryHelpers::qrealPrecise(x)
#define P(x, y) QPoint(V(x), V(y))

Q_DECLARE_METATYPE(PluginInput)
Q_DECLARE_METATYPE(PluginOutput)
Q_DECLARE_METATYPE(PluginMetadata)

void emptyMessageHandler(QtMsgType, const QMessageLogContext &, const QString &) {}

class TestPlugins : public QObject
{
    Q_OBJECT

public:
    TestPlugins();

private Q_SLOTS:
    void initTestCase();
    void testPlugin_data();
    void testPlugin();
    void pluginInputSerialization_data();
    void pluginInputSerialization();
    void pluginOutputSerialization_data();
    void pluginOutputSerialization();
    void pluginMetadataSerialization_data();
    void pluginMetadataSerialization();
    void pluginInputDeserialization_data();
    void pluginInputDeserialization();
    void pluginOutputDeserialization_data();
    void pluginOutputDeserialization();
    void pluginMetadataDeserialization_data();
    void pluginMetadataDeserialization();
};

TestPlugins::TestPlugins() {}

void TestPlugins::initTestCase()
{
    // Because the methods tested emit a lot of warnings on invalid files and we
    // want to test invalid files the warning and debug messages are suppressed
    // to make the test output more readable
    qInstallMessageHandler(emptyMessageHandler);
}
void TestPlugins::testPlugin_data()
{
    QTest::addColumn<QString>("pluginName");
    QTest::addColumn<QString>("fileName");
    Bakery bakery;
    QList<QString> files;

    files << ":/testPlugins/inputFiles/valid.txt";
    files << ":/testPlugins/inputFiles/validNegative.txt";
    files << ":/testPlugins/inputFiles/hasToTurn.txt";
    files << ":/testPlugins/inputFiles/manyHuge.txt";
    files << ":/testPlugins/inputFiles/manyMixed.txt";
    files << ":/testPlugins/inputFiles/manySmall.txt";

    foreach (QString file, files)
    {
        foreach (QString s, bakery.getAllPlugins())
        {
            QTest::newRow(QString("%1: '%2'").arg(s).arg(file).toLatin1()) << s << file;
        }
    }
}

void TestPlugins::testPlugin()
{
    QFETCH(QString, pluginName);
    QFETCH(QString, fileName);

    Bakery bakery;
    bakery.setAllPluginsEnabled(false);
    bakery.setPluginEnabled(pluginName);
    bakery.setTimeLimit(120 * 1000);

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    bool ok;
    PluginInput input = bakery.loadFromDevice(&file, &ok);
    QVERIFY2(ok, "Error while loading file");

    PluginOutput output = bakery.computeBestOutput(input, &ok);
    QVERIFY2(ok, "Error while processing");
    QVERIFY2(output.sheets.size() != 0, "No sheets returned");
    QVERIFY2(Bakery::isOutputValidForInput(input, output), "Input does not match output");
}

void TestPlugins::pluginInputSerialization_data()
{
    QTest::addColumn<PluginInput>("input");
    QTest::addColumn<QString>("serialized");

    {
        PluginInput input;
        input.sheetWidth = V(1.5);
        input.sheetHeight = V(2.5);
        QTest::newRow("Empty input") << input << "plugininput_begin 100000 150000 250000 0 shapelist_begin shapelist_end plugininput_end ";
    }

    {
        PluginInput input;
        input.sheetWidth = V(1.5);
        input.sheetHeight = V(2.5);

        Shape shape("shape");
        shape << P(0, 0) << P(0, 1) << P(1, 1) << P(1, 0);
        shape.ensureClosed();

        input.shapes << shape;

        QTest::newRow("1 shape") << input << "plugininput_begin 100000 150000 250000 1 shapelist_begin shape_begin text_begin shape "
                                             "text_end 4 0 0 0 100000 100000 100000 100000 0 shape_end shapelist_end plugininput_end ";
    }

    {
        PluginInput input;
        input.sheetWidth = V(1.5);
        input.sheetHeight = V(2.5);

        Shape shape("shape");
        shape << P(0, 0) << P(0, 1) << P(1, 1) << P(1, 0);
        shape.ensureClosed();

        input.shapes << shape << shape;

        QTest::newRow("2 shapes") << input << "plugininput_begin 100000 150000 250000 2 shapelist_begin shape_begin text_begin shape "
                                              "text_end 4 0 0 0 100000 100000 100000 100000 0 shape_end shape_begin text_begin shape "
                                              "text_end 4 0 0 0 100000 100000 100000 100000 0 shape_end shapelist_end plugininput_end ";
    }
}

void TestPlugins::pluginInputSerialization()
{
    QFETCH(PluginInput, input);
    QFETCH(QString, serialized);

    QString testString;
    QTextStream stream(&testString, QIODevice::WriteOnly);
    stream << input;
    QCOMPARE(testString, serialized);
}

void TestPlugins::pluginOutputSerialization_data()
{
    QTest::addColumn<PluginOutput>("output");
    QTest::addColumn<QString>("serialized");

    {
        PluginOutput output;
        QTest::newRow("Empty output") << output << "pluginoutput_begin 0 sheetlist_begin sheetlist_end pluginoutput_end ";
    }

    {
        PluginOutput output;

        Sheet sheet(V(5.5), V(2.5));
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        sheet << s;

        output.sheets << sheet;

        QTest::newRow("1 sheet") << output << "pluginoutput_begin 1 sheetlist_begin sheet_begin 550000 250000 1 shape_begin text_begin "
                                              "testshape text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end sheet_end sheetlist_end "
                                              "pluginoutput_end ";
    }

    {
        PluginOutput output;

        Sheet sheet(V(5.5), V(2.5));
        Shape s("testshape");
        s << P(0, 0) << P(0, 0.1) << P(0.1, 0.1) << P(0.1, 0);
        s.ensureClosed();
        sheet << s;

        output.sheets << sheet << sheet;

        QTest::newRow("2 sheets") << output
                                  << "pluginoutput_begin 2 sheetlist_begin sheet_begin 550000 250000 1 shape_begin text_begin "
                                     "testshape text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end sheet_end sheet_begin 550000 250000 1 "
                                     "shape_begin text_begin testshape text_end 4 0 0 0 10000 10000 10000 10000 0 shape_end sheet_end "
                                     "sheetlist_end pluginoutput_end ";
    }
}

void TestPlugins::pluginOutputSerialization()
{
    QFETCH(PluginOutput, output);
    QFETCH(QString, serialized);

    QString testString;
    QTextStream stream(&testString, QIODevice::WriteOnly);
    stream << output;
    QCOMPARE(testString, serialized);
}

void TestPlugins::pluginMetadataSerialization_data()
{
    QTest::addColumn<PluginMetadata>("meta");
    QTest::addColumn<QString>("serialized");

    {
        PluginMetadata meta;
        meta.uniqueName = "myName";
        meta.type = "myType";
        meta.author = "myAuthor";
        meta.license = "myLicense";

        QTest::newRow("Metadata without spaces") << meta << "pluginmetadata_begin text_begin myName text_end text_begin myType text_end "
                                                            "text_begin myAuthor text_end text_begin myLicense text_end "
                                                            "pluginmetadata_end ";
    }

    {
        PluginMetadata meta;
        meta.uniqueName = "my name";
        meta.type = "my type";
        meta.author = "my author";
        meta.license = "my license";

        QTest::newRow("Metadata with spaces") << meta << "pluginmetadata_begin text_begin my name text_end text_begin my type text_end "
                                                         "text_begin my author text_end text_begin my license text_end pluginmetadata_end ";
    }
}

void TestPlugins::pluginMetadataSerialization()
{
    QFETCH(PluginMetadata, meta);
    QFETCH(QString, serialized);

    QString testString;
    QTextStream stream(&testString, QIODevice::WriteOnly);
    stream << meta;
    QCOMPARE(testString, serialized);
}

void TestPlugins::pluginInputDeserialization_data()
{
    // Reuse data
    pluginInputSerialization_data();
}

void TestPlugins::pluginInputDeserialization()
{
    QFETCH(PluginInput, input);
    QFETCH(QString, serialized);

    QTextStream stream(&serialized, QIODevice::ReadOnly);
    PluginInput newInput;
    stream >> newInput;
    QCOMPARE(newInput, input);
}

void TestPlugins::pluginOutputDeserialization_data()
{
    // Reuse data
    pluginOutputSerialization_data();
}

void TestPlugins::pluginOutputDeserialization()
{
    QFETCH(PluginOutput, output);
    QFETCH(QString, serialized);

    QTextStream stream(&serialized, QIODevice::ReadOnly);
    PluginOutput newOutput;
    stream >> newOutput;
    QCOMPARE(newOutput, output);
}

void TestPlugins::pluginMetadataDeserialization_data()
{
    // Reuse data
    pluginMetadataSerialization_data();
}

void TestPlugins::pluginMetadataDeserialization()
{
    QFETCH(PluginMetadata, meta);
    QFETCH(QString, serialized);

    QTextStream stream(&serialized, QIODevice::ReadOnly);
    PluginMetadata newMeta;
    stream >> newMeta;
    QCOMPARE(newMeta, meta);
}

QTEST_MAIN(TestPlugins)

#include "tst_testplugins.moc"
