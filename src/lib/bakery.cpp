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

#include "helpers.hpp"
#include "bakery.h"

#include "qmath.h"
#include <QLibrary>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QBuffer>
#include <QPainterPath>
#include <QColor>
#include <QSet>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <random>
#include <QTime>

/*!
 * \brief Convenience method to set the value of a bool pointer to value if not NULL.
 * \param b bool pointer.
 * \param value Target value.
 */
static void setBool(bool *b, bool value)
{
    if (b != NULL)
    {
        *b = value;
    }
}

// Static methods

PluginInput Bakery::loadFromDevice(QIODevice *device, bool *ok)
{
    if (device == NULL)
    {
        BAKERY_WARNING("Device is NULL");
        setBool(ok, false);
        return PluginInput();
    }
    if (!device->isOpen())
    {
        BAKERY_WARNING("Device not open");
        setBool(ok, false);
        return PluginInput();
    }

    QSet<QString> namesSet;
    QString s;
    QTextStream stream(device);
    bool conversionOk;

    PluginInput input;
    qint32 numTypeShapes;

    // Load sheet info
    stream >> s;
    double w = s.toDouble(&conversionOk);
    if (!conversionOk || stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL("Could not convert width");
        setBool(ok, false);
        return PluginInput();
    }
    input.sheetWidth = BakeryHelpers::qrealPrecise(w);

    stream >> s;
    double h = s.toDouble(&conversionOk);
    if (!conversionOk || stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL("Could not convert height");
        setBool(ok, false);
        return PluginInput();
    }
    input.sheetHeight = BakeryHelpers::qrealPrecise(h);

    // Load number of shapes
    stream >> s;
    numTypeShapes = s.toInt(&conversionOk);
    if (!conversionOk || stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL("Could not convert number of shape types");
        setBool(ok, false);
        return PluginInput();
    }

    if (numTypeShapes < 0)
    {
        BAKERY_CRITICAL("Negative number of type of shapes");
        setBool(ok, false);
        return PluginInput();
    }

    QList<Shape> shapes;

    // Load shapes
    for (qint32 i_shapes = 0; i_shapes < numTypeShapes; ++i_shapes)
    {
        if (stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL("Could not load shape");
            setBool(ok, false);
            return PluginInput();
        }

        QString shapeName;
        qint32 numShapes;
        qint32 numPoints;
        qreal x;
        qreal y;

        do
        {
            if (stream.atEnd())
            {
                BAKERY_CRITICAL("Could not find name");
                setBool(ok, false);
                return PluginInput();
            }
            shapeName = stream.readLine();
        } while (shapeName == "");

        if (namesSet.contains(shapeName))
        {
            BAKERY_CRITICAL("Unique name" << shapeName << "found twice");
            setBool(ok, false);
            return PluginInput();
        }

        namesSet << shapeName;

        Shape shape(shapeName);
        shape.setUpdateMetrics(false);

        stream >> s;
        numShapes = s.toInt(&conversionOk);
        if (!conversionOk || stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL("Could not convert number of shapes");
            setBool(ok, false);
            return PluginInput();
        }
        if (numShapes < 0)
        {
            BAKERY_CRITICAL("Negative number of shapes");
            setBool(ok, false);
            return PluginInput();
        }

        stream >> s;
        numPoints = s.toInt(&conversionOk);
        if (!conversionOk || stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL("Could not convert number of points");
            setBool(ok, false);
            return PluginInput();
        }
        if (numPoints < 0)
        {
            BAKERY_CRITICAL("Negative number of points");
            setBool(ok, false);
            return PluginInput();
        }

        for (qint32 i_points = 0; i_points < numPoints; ++i_points)
        {
            stream >> s;
            x = s.toDouble(&conversionOk);
            if (!conversionOk || stream.status() != QTextStream::Ok)
            {
                BAKERY_CRITICAL("Could not convert x");
                setBool(ok, false);
                return PluginInput();
            }

            stream >> s;
            y = s.toDouble(&conversionOk);
            if (!conversionOk || stream.status() != QTextStream::Ok)
            {
                BAKERY_CRITICAL("Could not convert y");
                setBool(ok, false);
                return PluginInput();
            }

            shape << BakeryHelpers::qPointPrecise(QPointF(x, y));
        }

        shape.ensureClosed();
        shape.setUpdateMetrics(true);

        for (qint32 i_numShapes = 0; i_numShapes < numShapes; ++i_numShapes)
        {
            shapes.append(shape);
        }
    }

    input.shapes = shapes;
    setBool(ok, true);
    return input;
}

PluginInput Bakery::loadFromSVG(QIODevice *device, bool *ok)
{
    if (device == NULL)
    {
        BAKERY_WARNING("Device is NULL");
        setBool(ok, false);
        return PluginInput();
    }
    if (!device->isOpen())
    {
        BAKERY_WARNING("Device not open");
        setBool(ok, false);
        return PluginInput();
    }

    setBool(ok, true);

    QXmlStreamReader reader(device);
    PluginInput input;
    qint32 counter = 0;

    input.sheetWidth = BakeryHelpers::qrealPrecise(1.0);
    input.sheetHeight = BakeryHelpers::qrealPrecise(1.0);

    while (!reader.atEnd())
    {
        switch (reader.readNext())
        {
        case QXmlStreamReader::StartDocument:
            // Nothing to do here
            break;

        case QXmlStreamReader::StartElement:
        {
            QXmlStreamAttributes attributes = reader.attributes();
            if (reader.name() == "svg")
            {
                if (attributes.hasAttribute("width"))
                {
                    input.sheetWidth = BakeryHelpers::qrealPrecise(attributes.value("width").toDouble());
                }
                if (attributes.hasAttribute("height"))
                {
                    input.sheetHeight = BakeryHelpers::qrealPrecise(attributes.value("height").toDouble());
                }
                continue;
            }
            else if (reader.name() == "line" || reader.name() == "polyline")
            {
                BAKERY_WARNING("Not supporting elements of type" << reader.name());
                continue;
            }
            else if (reader.name() == "rect")
            {
                qreal x = attributes.hasAttribute("x") ? attributes.value("x").toDouble() : 0.0;
                qreal y = attributes.hasAttribute("y") ? attributes.value("y").toDouble() : 0.0;
                qreal widthRect = attributes.hasAttribute("width") ? attributes.value("width").toDouble() : 0.0;
                qreal heightRect = attributes.hasAttribute("height") ? attributes.value("height").toDouble() : 0.0;
                qreal rx = 0.0;
                qreal ry = 0.0;

                if (widthRect == 0.0 || heightRect == 0.0)
                {
                    continue;
                }

                if (widthRect < 0.0 || heightRect < 0.0)
                {
                    BAKERY_CRITICAL("Width / height is not allowed to be negative");
                    setBool(ok, false);
                }

                if (attributes.hasAttribute("rx") || attributes.hasAttribute("ry"))
                {
                    rx = attributes.hasAttribute("rx") ? attributes.value("rx").toDouble() : 0.0;
                    ry = attributes.hasAttribute("ry") ? attributes.value("ry").toDouble() : 0.0;

                    if (rx == 0)
                    {
                        rx = ry;
                    }
                    else if (ry == 0)
                    {
                        ry = rx;
                    }

                    rx = qMin(rx, widthRect / 2.0);
                    ry = qMin(ry, heightRect / 2.0);
                }

                Shape shape(Shape(QString("rect-%1").arg(counter++)));
                QPainterPath path;
                path.addRoundedRect(x, y, heightRect, widthRect, rx, ry);
                foreach (QPointF point, path.toFillPolygon())
                {
                    shape << BakeryHelpers::qPointPrecise(point);
                }
                shape.ensureClosed();
                input.shapes << shape;
            }
            else if (reader.name() == "circle" || reader.name() == "ellipse")
            {
                qreal cx = attributes.hasAttribute("cx") ? attributes.value("cx").toDouble() : 0.0;
                qreal cy = attributes.hasAttribute("cx") ? attributes.value("cx").toDouble() : 0.0;
                qreal rWidth;
                qreal rHeight;

                if (reader.name() == "circle")
                {
                    if (!attributes.hasAttribute("r") || attributes.value("r").toDouble() <= 0)
                    {
                        BAKERY_CRITICAL("Radius must be a positive number");
                        setBool(ok, false);
                        continue;
                    }

                    rWidth = rHeight = attributes.value("r").toDouble();
                }
                else // ellipse
                {
                    if (!attributes.hasAttribute("rx") || attributes.value("rx").toDouble() <= 0 || !attributes.hasAttribute("ry")
                        || attributes.value("ry").toDouble() <= 0)
                    {
                        BAKERY_CRITICAL("Radius (rx/ry) must be a positive number");
                        setBool(ok, false);
                        continue;
                    }
                    rWidth = attributes.value("rx").toDouble();
                    rHeight = attributes.value("ry").toDouble();
                }
                Shape shape(Shape(QString("elippse-%1").arg(counter++)));
                QPainterPath p;
                p.addEllipse(cx, cy, rWidth, rHeight);
                foreach (QPointF point, p.toFillPolygon())
                {
                    shape << BakeryHelpers::qPointPrecise(point);
                }
                shape.ensureClosed();
                input.shapes << shape;
            }
            else if (reader.name() == "polygon")
            {
                if (attributes.hasAttribute("points"))
                {
                    QByteArray points_buffer = attributes.value("points").toLatin1();
                    QBuffer buffer(&points_buffer);
                    buffer.open(QIODevice::ReadOnly);
                    QTextStream stream(&buffer);
                    Shape shape(QString("polygon-%1").arg(counter++));
                    bool parseSuccessful = true;
                    while (parseSuccessful && !stream.atEnd())
                    {
                        qreal x;
                        qreal y;
                        QString s;

                        stream >> s;
                        QStringList l = s.split(",", QString::SkipEmptyParts);
                        if (l.size() == 0)
                        {
                            BAKERY_CRITICAL("Error while parsing points");
                            setBool(ok, false);
                            parseSuccessful = false;
                            break;
                        }

                        x = l[0].toDouble(&parseSuccessful);
                        if (!parseSuccessful)
                        {
                            BAKERY_CRITICAL("Error while parsing points");
                            setBool(ok, false);
                            parseSuccessful = false;
                            break;
                        }

                        if (l.size() >= 2)
                        {
                            y = l[1].toDouble(&parseSuccessful);
                            if (!parseSuccessful)
                            {
                                BAKERY_CRITICAL("Error while parsing points");
                                setBool(ok, false);
                                parseSuccessful = false;
                                break;
                            }
                        }
                        else
                        {
                            stream >> s;
                            if (stream.atEnd())
                            {
                                BAKERY_CRITICAL("Error while parsing points: Missing coordinate");
                                setBool(ok, false);
                                parseSuccessful = false;
                                break;
                            }
                            l = s.split(",", QString::SkipEmptyParts);
                            if (l.size() == 0)
                            {
                                BAKERY_CRITICAL("Error while parsing points");
                                setBool(ok, false);
                                parseSuccessful = false;
                                break;
                            }

                            y = l[0].toDouble(&parseSuccessful);
                            if (!parseSuccessful)
                            {
                                BAKERY_CRITICAL("Error while parsing points");
                                setBool(ok, false);
                                parseSuccessful = false;
                                break;
                            }
                        }

                        shape << BakeryHelpers::qPointPrecise(QPointF(x, y));
                    }
                    buffer.close();
                    shape.ensureClosed();
                    if (parseSuccessful)
                    {
                        input.shapes << shape;
                    }
                }
                else
                {
                    BAKERY_CRITICAL("Polygon is missing points attribute");
                    setBool(ok, false);
                }
            }
            else
            {
                BAKERY_DEBUG("Unknown element" << reader.name());
            }
        }
        break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::EndElement:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::DTD:
            // Nothing to do here
            break;

        case QXmlStreamReader::Invalid:
            BAKERY_WARNING("Invalid SVG file:" << reader.errorString());
            setBool(ok, false);
            return PluginInput();
            break;

        default:
            BAKERY_DEBUG("Unknown token:" << reader.tokenString());
        }
    }

    if (input.shapes.size() == 0)
    {
        BAKERY_WARNING("No shapes found");
        setBool(ok, false);
        return PluginInput();
    }

    return input;
}

PluginInput Bakery::randomInput(RandomPluginInputParameters parameters)
{
    /* Random initiation */
    std::mt19937 rnd;
    std::uniform_int_distribution<qint32> distribution(0, BakeryHelpers::qrealPrecise(1));
    std::uniform_int_distribution<qint32> shape_distribution(parameters.minShapes, parameters.maxShapes);
    std::uniform_int_distribution<qint32> points_distribution(parameters.minPoints, parameters.maxPoints);
    std::uniform_int_distribution<qint32> amount_distribution(parameters.minAmount, parameters.maxAmount);
    std::uniform_real_distribution<qreal> scale_distribution(parameters.minScale, parameters.maxScale);

    if (parameters.seed == -1)
    {
        std::random_device random_device;
        if (random_device.entropy() == 0.0)
        {
            BAKERY_DEBUG("random_device entropy is zero (possibly deterministic) - using fallback");
            QVector<int> seed_vector;
            seed_vector.reserve(4);
            seed_vector << random_device(); /* In the case that random_device actually returns true random - some implementations always
                                               return zero entropy */
            seed_vector << QTime::currentTime().msecsSinceStartOfDay();
            seed_vector << (int)QDateTime::currentMSecsSinceEpoch();
            seed_vector << (int)(32 >> QDateTime::currentMSecsSinceEpoch());
            std::seed_seq seed(seed_vector.begin(), seed_vector.end());
            rnd.seed(seed);
        }
        else
        {
            rnd.seed(random_device());
        }
    }
    else
    {
        rnd.seed(parameters.seed);
    }

    /* Other declarations */
    PluginInput input;
    qreal sheetWidth = std::uniform_real_distribution<qreal>(parameters.minSheetWidth, parameters.maxSheetWidth)(rnd);
    qreal sheetHeight = std::uniform_real_distribution<qreal>(parameters.minSheetHeight, parameters.maxSheetHeight)(rnd);
    input.sheetWidth = BakeryHelpers::qrealPrecise(sheetWidth);
    input.sheetHeight = BakeryHelpers::qrealPrecise(sheetHeight);

    qint32 sheets = std::uniform_int_distribution<qint32>(parameters.minSheets, parameters.maxSheets)(rnd);
    qint64 sheetArea = (qint64)input.sheetWidth * (qint64)input.sheetHeight / BAKERY_PRECISION;
    qint64 minArea = (qint64)(sheets - 1) * sheetArea;
    qint64 area = 0;

    /* random distributions based on height or width */
    std::uniform_real_distribution<qreal> scale_x_distribution(1, sheetWidth);
    std::uniform_real_distribution<qreal> scale_y_distribution(1, sheetHeight);

    do
    {
        input.shapes.clear();
        area = 0;
        qint32 shapes = shape_distribution(rnd);
        while (shapes > 0)
        {
            Shape shape;
            shape << QPoint(0, 0);
            qint32 points = points_distribution(rnd);
            while (shape.size() < points)
            {
                qint32 x = distribution(rnd);
                qint32 y = distribution(rnd);
                shape << QPoint(x, y);
                if (!shape.isSimple())
                {
                    shape.removeLast();
                }
            }
            shape.ensureClosed();

            bool valid = true;
            qint32 size = shape.edges().size();
            for (qint32 i = 0; i < size; ++i)
            {
                QLineF e1(shape.edges().at(i));
                QLineF e2(shape.edges().at((i + 1) % size));
                qreal angle = qAbs(e1.angleTo(e2) - 180);
                if (angle < parameters.minAngle || angle > 360 - parameters.minAngle)
                {
                    valid = false;
                    break;
                }
            }
            if (!valid)
            {
                continue;
            }

            shape.setName(QString("Shape %1").arg(shapes + 1));
            qreal scaleX = scale_x_distribution(rnd) / scale_distribution(rnd);
            qreal scaleY = scale_y_distribution(rnd) / scale_distribution(rnd);
            shape.scale(scaleX, scaleY);
            qint32 amount = amount_distribution(rnd);
            for (qint32 i = 0; i < amount; ++i)
            {
                input.shapes << shape;
                area += shape.area();
            }
            --shapes;
        }
    } while (area < minArea);

    return input;
}

bool Bakery::saveToDevice(PluginInput input, QIODevice *device)
{
    if (device == NULL)
    {
        BAKERY_WARNING("Device is NULL");
        return false;
    }
    if (!device->isOpen())
    {
        BAKERY_WARNING("Device not open");
        return false;
    }
    QTextStream stream(device);

    // Sheet meta
    Shape::Unique unique = Shape::reduceToUnique(input.shapes);
    stream << BakeryHelpers::qrealRounded(input.sheetWidth) << "\n";
    stream << BakeryHelpers::qrealRounded(input.sheetHeight) << "\n";
    stream << unique.shapes.size() << "\n";

    // Shapes
    foreach (Shape shape, unique.shapes)
    {
        // Name, number of copies and number of coordinates
        shape.ensureClosed(false);
        stream << shape.name() << "\n" << unique.amounts[shape.name()] << "\n" << shape.size() << "\n";
        // Coordinates
        foreach (QPoint point, shape)
        {
            stream << BakeryHelpers::qrealRounded(point.x()) << " " << BakeryHelpers::qrealRounded(point.y()) << "\n";
        }
    }

    return true;
}

bool Bakery::saveToDevice(PluginOutput output, QIODevice *device)
{
    if (device == NULL)
    {
        BAKERY_WARNING("Device is NULL");
        return false;
    }
    if (!device->isOpen())
    {
        BAKERY_WARNING("Device not open");
        return false;
    }

    QTextStream stream(device);

    // Utilitization and number of sheets
    qreal utilitization = 0.0;
    qint32 numSheets = 0;

    foreach (Sheet sheet, output.sheets)
    {
        utilitization += sheet.utilitization();
        ++numSheets;
    }

    if (numSheets == 0)
    {
        BAKERY_CRITICAL("No sheets in output");
        return false;
    }

    utilitization /= numSheets;
    utilitization *= 100;

    stream << (qint32)utilitization << "\n" << numSheets << "\n";

    // Write sheets
    for (qint32 i = 0; i < output.sheets.size(); ++i)
    {
        // Sheet number
        stream << (i + 1) << "\n";

        // Shapes
        foreach (Shape s, output.sheets[i].shapes())
        {
            // Name
            stream << s.name() << "\n";

            // Points
            // The last point is not saved because it is equal to the first one if the shape is closed (should be)
            for (qint32 i = 0; i < s.size(); ++i)
            {
                if (i != (s.size() - 1) || !s.isClosed())
                {
                    stream << BakeryHelpers::qrealRounded(s[i].x()) << " " << BakeryHelpers::qrealRounded(s[i].y()) << "\n";
                }
            }
        }
    }
    return true;
}

bool Bakery::saveAsSVG(PluginOutput output, QDir outputDir, QString filePrefix)
{
    if (output.sheets.size() == 0)
    {
        BAKERY_CRITICAL("No sheets");
        return false;
    }

    if (!outputDir.exists())
    {
        // Create directory
        BAKERY_WARNING("Creating directory" << outputDir.absolutePath());
        if (!outputDir.mkpath(outputDir.absolutePath()))
        {
            BAKERY_CRITICAL("Could not create directory");
            return false;
        }
    }

    for (qint32 i_sheets = 0; i_sheets < output.sheets.size(); ++i_sheets)
    {
        QFile file(outputDir.absoluteFilePath(QString("%2-%3.svg").arg(filePrefix).arg(i_sheets + 1)));
        if (!file.open(QIODevice::WriteOnly))
        {
            BAKERY_CRITICAL("Could not open file");
            return false;
        }

        qreal scale = 100.0;
        qreal width = BakeryHelpers::qrealRounded(output.sheets[0].width());
        qreal height = BakeryHelpers::qrealRounded(output.sheets[0].height());

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeDTD(
            "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">");
        writer.writeStartElement("svg");
        writer.writeAttribute("xmlns", "http://www.w3.org/2000/svg");
        writer.writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
        writer.writeAttribute("width", QString("%1").arg(width * scale));
        writer.writeAttribute("height", QString("%1").arg(height * scale));
        writer.writeAttribute("viewBox", QString("0 0 %1 %2").arg(width).arg(height));

        // Write shapes
        QMap<QString, QColor> colors;
        foreach (Shape shape, output.sheets[i_sheets].shapes())
        {
            if (!colors.contains(shape.name()))
            {
                colors[shape.name()] = QColor(qrand() % 255, qrand() % 255, qrand() % 255);
            }
            QString polygonPoints;
            foreach (QPointF p, shape)
            {
                polygonPoints.append(QString("%1,%2 ").arg(BakeryHelpers::qrealRounded(p.x())).arg(BakeryHelpers::qrealRounded(p.y())));
            }
            writer.writeStartElement("polygon");
            writer.writeAttribute("points", polygonPoints);
            QString style = QString("fill:%1;stroke:#000;stroke-width:0.01").arg(colors[shape.name()].name());
            writer.writeAttribute("style", style);
            writer.writeEndElement(); // polygon
        }

        writer.writeEndElement(); // svg
        writer.writeEndDocument();
    }
    return true;
}

bool Bakery::isOutputValidForInput(const PluginInput &input, const PluginOutput &output)
{
    QList<Shape> inputList = input.shapes;
    foreach (Sheet s, output.sheets)
    {
        // Test valid sheet
        if (!s.isValid())
        {
            return false;
        }

        // Test sheet size
        if (s.width() != input.sheetWidth || s.height() != input.sheetHeight)
        {
            return false;
        }

        foreach (Shape shape, s.shapes())
        {
            // Test shapes
            bool found = false;
            for (QList<Shape>::Iterator i = inputList.begin(); !found && i != inputList.end(); ++i)
            {
                if (shape.name() == i->name())
                {
                    found = true;
                    inputList.erase(i);
                }
            }
            if (!found)
            {
                return false;
            }
        }
    }
    return inputList.isEmpty();
}

PluginOutput Bakery::findBestOutput(QHash<QString, PluginOutput> &outputs)
{
    if (outputs.isEmpty())
    {
        BAKERY_WARNING("Empty hash");
        return PluginOutput();
    }

    QList<PluginOutput> candidates(outputs.values());
    if (candidates.size() > 1)
    {
        std::sort(candidates.begin(), candidates.end());
    }
    return candidates.last();
}

bool Bakery::saveToDirectory(PluginOutput &output, QString outputDirectoryPath, QString resultsFileName, bool svgOutput)
{
    // Create output directory
    if (!QDir(outputDirectoryPath).exists())
    {
        if (!QDir().mkpath(outputDirectoryPath))
        {
            BAKERY_CRITICAL(QString("Failed to create output directory \"%1\"").arg(outputDirectoryPath));
            return false;
        }
    }

    // Save text file
    QString resultsFilePath = QDir(outputDirectoryPath).absoluteFilePath(resultsFileName);
    QFile resultsFile(resultsFilePath);
    if (!resultsFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        BAKERY_CRITICAL(QString("Failed to save text file to \"%1\"").arg(resultsFilePath));
        return false;
    }
    saveToDevice(output, &resultsFile);
    resultsFile.close();

    // Save SVGs
    if (svgOutput)
    {
        if (!saveAsSVG(output, outputDirectoryPath))
        {
            BAKERY_CRITICAL(QString("Failed to save SVG files to directory \"%1\"").arg(outputDirectoryPath));
            return false;
        }
    }

    return true;
}

// Constructor

Bakery::Bakery(QObject *parent, QDir pluginDir) : QObject(parent)
{
    // Set time limit
    _settings["runProperties/timelimit"] = 0;

    loadPluginsFromDirectory(pluginDir);
}

// Deconstructor

Bakery::~Bakery() { qDeleteAll(_pluginsRunners); }

bool Bakery::loadPlugin(QString path)
{
    // Start process
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.start(path, QProcess::ReadWrite);
    if (!process.waitForStarted(5000))
    {
        BAKERY_CRITICAL(QString("Plugin candidate '%1' failed to start in time").arg(path));
        return false;
    }

    // Send command
    process.write("give_metadata \n");
    process.waitForBytesWritten();

    // Wait for response
    if (!process.waitForReadyRead(2000))
    {
        if (process.state() != QProcess::Running)
        {
            if (process.exitStatus() != QProcess::NormalExit)
            {
                BAKERY_CRITICAL(QString("Failed to capture metadata from plugin candidate '%1'").arg(path));
            }
            else
            {
                BAKERY_CRITICAL(QString("Plugin candidate '%1' crashed when asked to give metadata").arg(path));
            }
        }
        else
        {
            BAKERY_CRITICAL(QString("Plugin candidate '%1' failed to give metadata in time").arg(path));
            process.kill();
        }
        return false;
    }

    // Parse response
    if (!process.canReadLine())
    {
        BAKERY_CRITICAL(QString("Plugin candidate '%1' violated protocol (missing new line character)").arg(path));
    }
    QByteArray buffer = process.readLine();
    QTextStream stream(buffer);
    PluginMetadata meta;
    stream >> meta;
    if (stream.status() != QTextStream::Ok)
    {
        BAKERY_CRITICAL(QString("Plugin candidate '%1' provided invalid metadata").arg(path));
        return false;
    }

    // Wait for process to finish
    if (!process.waitForFinished(2000))
    {
        BAKERY_CRITICAL(QString("Plugin candidate '%1' violated protocol (failed to terminate after sending metadata)").arg(path));
    }

    // Ensure uniqueness of provided name
    if (_pluginsMetadata.contains(meta.uniqueName))
    {
        BAKERY_CRITICAL(QString("Plugin candidate '%1' provided name that is already in use ('%2')").arg(path, meta.uniqueName));
        return false;
    }

    // Store plugin information
    QString key = QString("pluginsEnabled/%1").arg(meta.uniqueName);
    if (!_settings.contains(key))
    {
        _settings[key] = true;
    }
    _pluginsMetadata[meta.uniqueName] = meta;
    _pluginsPaths[meta.uniqueName] = path;

    return true;
}

// Methods

bool Bakery::loadPluginsFromDirectory(QDir directory)
{
    bool foundPlugin = false;

    // Query plugins
    BAKERY_DEBUG(QString("Querying directory '%1' for plugins").arg(directory.absolutePath()));
    foreach (QString file, directory.entryList())
    {
        QString path = directory.absoluteFilePath(file);
        QFileInfo fileInfo(path);
        if (fileInfo.isDir())
        {
            continue;
        }
        if (!fileInfo.isExecutable())
        {
            BAKERY_DEBUG(QString("Plugin candidate '%1' is not executable").arg(path));
            continue;
        }
        if (!loadPlugin(path))
        {
            BAKERY_DEBUG(QString("Plugin candidate '%1' could not be loaded").arg(path));
        }
        else
        {
            foundPlugin = true;
        }
    }
    if (!foundPlugin)
    {
        BAKERY_CRITICAL("No plugins found");
        return false;
    }

    QStringList pluginsNames(_pluginsMetadata.keys());
    BAKERY_DEBUG(QString("Loaded plugin(s): %1").arg(pluginsNames.join(", ")));
    return true;
}

bool Bakery::isPluginLoaded(QString pluginName) const { return _pluginsMetadata.contains(pluginName); }

QHash<QString, PluginOutput> Bakery::computeAllOutputs(PluginInput input, bool synchronous, bool *ok)
{
    _validOutputs.clear();
    if (_pluginsMetadata.size() == 0)
    {
        BAKERY_CRITICAL("No plugins loaded");
        if (synchronous)
        {
            setBool(ok, false);
        }
        return _validOutputs;
    }

    // Run plugins
    foreach (QString pluginName, getEnabledPlugins())
    {
        PluginRunner *runner = new PluginRunner(pluginName, _pluginsPaths[pluginName], input);
        connect(runner, SIGNAL(outputUpdated(QString, PluginOutput)), this, SLOT(_pluginOutputUpdated(QString, PluginOutput)));
        connect(runner, SIGNAL(finished(int, QString, PluginInput, PluginOutput)), this,
                SLOT(_pluginFinished(int, QString, PluginInput, PluginOutput)));
        _pluginsRunners[pluginName] = runner;
        emit pluginStarting(pluginName);
        runner->run();
        if (_settings["runProperties/timelimit"].toInt() != 0)
        {
            runner->terminate(_settings["runProperties/timelimit"].toInt());
        }
    }

    if (synchronous)
    {
        while (!_pluginsRunners.isEmpty())
        {
            QCoreApplication::processEvents();
            QThread::msleep(100);
        }
        setBool(ok, true);
    }

    return _validOutputs;
}

PluginOutput Bakery::computeBestOutput(PluginInput input, bool *ok)
{
    QHash<QString, PluginOutput> outputs = computeAllOutputs(input, true, ok);
    return ok ? findBestOutput(outputs) : PluginOutput();
}

PluginOutput Bakery::computeBestOutput(QIODevice *device, bool *ok)
{
    PluginInput input = loadFromDevice(device, ok);
    if (!ok)
    {
        BAKERY_CRITICAL("Could not parse device");
        return PluginOutput();
    }

    return computeBestOutput(input, ok);
}

bool Bakery::terminatePlugin(QString pluginName, int msec)
{
    _pluginsRunners.value(pluginName)->terminate(msec);
    emit pluginTerminating(pluginName, msec);
    return true;
}

bool Bakery::terminateAllPlugins(int msec)
{
    bool result = true;
    for (qint32 i = 0; i < _pluginsRunners.size(); ++i)
    {
        result &= terminatePlugin(_pluginsRunners.keys()[i], msec);
    }
    return result;
}

bool Bakery::killPlugin(QString pluginName) { return terminatePlugin(pluginName, 0); }

bool Bakery::killAllPlugins() { return terminateAllPlugins(0); }

QList<QString> Bakery::getAllPlugins() { return _pluginsMetadata.keys(); }

QList<QString> Bakery::getEnabledPlugins()
{
    QList<QString> list;
    foreach (QString key, _pluginsMetadata.keys())
    {
        if (_settings[QString("pluginsEnabled/%1").arg(key)].toBool())
        {
            list.append(key);
        }
    }
    return list;
}

QList<QString> Bakery::getDisabledPlugins()
{
    QList<QString> list;
    foreach (QString key, _pluginsMetadata.keys())
    {
        if (!_settings[QString("pluginsEnabled/%1").arg(key)].toBool())
        {
            list.append(key);
        }
    }
    return list;
}

void Bakery::setAllPluginsEnabled(bool enabled)
{
    foreach (QString key, _pluginsMetadata.keys())
    {
        _settings[QString("pluginsEnabled/%1").arg(key)] = enabled;
    }
}

void Bakery::setPluginEnabled(QString pluginName, bool enabled)
{
    if (!isPluginLoaded(pluginName))
    {
        BAKERY_WARNING(QString("Trying to enable plugin '%1' which is not loaded").arg(pluginName));
    }
    _settings[QString("pluginsEnabled/%1").arg(pluginName)] = enabled;
}

bool Bakery::isPluginEnabled(QString pluginName) const
{
    if (_settings.contains(QString("pluginsEnabled/%1").arg(pluginName)))
    {
        return _settings[QString("pluginsEnabled/%1").arg(pluginName)].toBool();
    }
    else
    {
        BAKERY_WARNING(QString("Trying to query enabled state plugin '%1' which is not loaded").arg(pluginName));
        return false;
    }
}

PluginMetadata Bakery::getPluginMetadata(QString pluginName) const
{
    if (!isPluginLoaded(pluginName))
    {
        BAKERY_WARNING("Plugin \"%1\" not available");
        return PluginMetadata();
    }
    return _pluginsMetadata[pluginName];
}

void Bakery::setTimeLimit(qint32 timeLimit) { _settings["runProperties/timelimit"] = timeLimit; }

qint32 Bakery::getTimeLimit() { return _settings["runProperties/timelimit"].toInt(); }

void Bakery::_pluginOutputUpdated(QString pluginName, PluginOutput pluginOutput) { emit pluginOutputUpdated(pluginName, pluginOutput); }

void Bakery::_pluginFinished(int exitCode, QString pluginName, PluginInput pluginInput, PluginOutput pluginOutput)
{
    if (!_pluginsRunners.contains(pluginName))
    {
        BAKERY_CRITICAL(QString("Plugin '%1' reported having finished but was not running").arg(pluginName));
    }

    bool valid = isOutputValidForInput(pluginInput, pluginOutput);
    if (valid)
    {
        _validOutputs[pluginName] = pluginOutput;
    }
    emit pluginFinished(exitCode, pluginName, pluginOutput, valid);

    _pluginsRunners.remove(pluginName);
    if (_pluginsRunners.isEmpty())
    {
        emit allPluginsFinished(_validOutputs);
    }
}
