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

#include "plugins.h"
#include "bakery.h"
#include "helpers.hpp"

#include <QTextStream>
#include <QTimer>
#include <QProcess>
#include <QTemporaryFile>
#include <QCoreApplication>

QTextStream &operator<<(QTextStream &stream, const PluginInput &input)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    stream << "plugininput_begin"
           << " ";
    stream << BAKERY_PRECISION;
    stream << " ";
    stream << input.sheetWidth << " ";
    stream << input.sheetHeight << " ";
    stream << input.shapes.size() << " ";
    stream << "shapelist_begin"
           << " ";
    for (QList<Shape>::ConstIterator i_shapes = input.shapes.constBegin(); i_shapes != input.shapes.constEnd(); ++i_shapes)
    {
        stream << *i_shapes;
    }
    stream << "shapelist_end"
           << " ";
    stream << "plugininput_end"
           << " ";
    return stream;
}

QTextStream &operator>>(QTextStream &stream, PluginInput &input)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    QString inputString;
    bool ok;
    qint32 precision, numShapes;

    // Initializer
    stream >> inputString;
    if (inputString != "plugininput_begin")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-plugininput into a plugininput (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Precision
    stream >> inputString;
    precision = inputString.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read precision");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }
    if (precision != BAKERY_PRECISION)
    {
        BAKERY_CRITICAL("Precision is not BAKERY_PRECISION");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Width
    stream >> inputString;
    input.sheetWidth = inputString.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read width");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Height
    stream >> inputString;
    input.sheetHeight = inputString.toDouble(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read height");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Number of shapes
    stream >> inputString;
    numShapes = inputString.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read number of shapes");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Shapes list initialiser
    stream >> inputString;
    if (inputString != "shapelist_begin")
    {
        BAKERY_CRITICAL("Can not find shape list (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Read shapes
    input.shapes.clear();
    for (qint32 i = 0; i < numShapes; ++i)
    {
        Shape shape;
        stream >> shape;
        if (stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL("Can not read shape" << (i + 1));
            return stream;
        }
        input.shapes << shape;
    }

    // Shapes list finalizer
    stream >> inputString;
    if (inputString != "shapelist_end")
    {
        BAKERY_CRITICAL("Can not find shape list (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // PluginInput end
    stream >> inputString;
    if (inputString != "plugininput_end")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-plugininput into a plugininput (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    return stream;
}

QTextStream &operator<<(QTextStream &stream, const PluginOutput &output)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    stream << "pluginoutput_begin"
           << " ";
    stream << output.sheets.size() << " ";
    stream << "sheetlist_begin"
           << " ";
    for (QList<Sheet>::ConstIterator i_sheets = output.sheets.constBegin(); i_sheets != output.sheets.constEnd(); ++i_sheets)
    {
        stream << *i_sheets;
    }
    stream << "sheetlist_end"
           << " ";
    stream << "pluginoutput_end"
           << " ";
    return stream;
}

QTextStream &operator>>(QTextStream &stream, PluginOutput &output)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    QString input;
    bool ok;
    qint32 numSheets;

    // Initializer
    stream >> input;
    if (input != "pluginoutput_begin")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-PluginOutput into a PluginOutput (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Number of sheets
    stream >> input;
    numSheets = input.toInt(&ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read number of sheets");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Sheetlist initialiser
    stream >> input;
    if (input != "sheetlist_begin")
    {
        BAKERY_CRITICAL("Can not find sheet list (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Read sheetss
    output.sheets.clear();
    for (qint32 i = 0; i < numSheets; ++i)
    {
        Sheet sheet;
        stream >> sheet;
        if (stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL("Can not read sheet" << (i + 1));
            return stream;
        }
        output.sheets << sheet;
    }

    // Sheetlist finalizer
    stream >> input;
    if (input != "sheetlist_end")
    {
        BAKERY_CRITICAL("Can not find sheet list (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // PluginOutput finalizer
    stream >> input;
    if (input != "pluginoutput_end")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-PluginOutput into a PluginOutput (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    return stream;
}

QTextStream &operator<<(QTextStream &stream, const PluginMetadata &meta)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    stream << "pluginmetadata_begin"
           << " ";
    BakeryHelpers::writeText(stream, meta.uniqueName);
    BakeryHelpers::writeText(stream, meta.type);
    BakeryHelpers::writeText(stream, meta.author);
    BakeryHelpers::writeText(stream, meta.license);
    stream << "pluginmetadata_end"
           << " ";

    return stream;
}

QTextStream &operator>>(QTextStream &stream, PluginMetadata &meta)
{
    if (Q_UNLIKELY(stream.status() != QTextStream::Ok))
    {
        BAKERY_CRITICAL("QTextStream status is not Ok");
        return stream;
    }

    QString input;
    bool ok;

    // Initializer
    stream >> input;
    if (input != "pluginmetadata_begin")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-PluginMetadata into a PluginMetadata (missing initializer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Name
    meta.uniqueName = BakeryHelpers::readText(stream, &ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read name");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Type
    meta.type = BakeryHelpers::readText(stream, &ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read type");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // Author
    meta.author = BakeryHelpers::readText(stream, &ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read author");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // License
    meta.license = BakeryHelpers::readText(stream, &ok);
    if (Q_UNLIKELY(!ok))
    {
        BAKERY_CRITICAL("Can not read license");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    // PluginMetadata finalizer
    stream >> input;
    if (input != "pluginmetadata_end")
    {
        BAKERY_CRITICAL("Trying to deserialize a non-PluginMetadata into a PluginMetadata (missing finalizer)");
        stream.setStatus(QTextStream::ReadCorruptData);
        return stream;
    }

    return stream;
}

bool operator==(const PluginInput &left, const PluginInput &right)
{
    if (left.sheetWidth != right.sheetWidth)
    {
        return false;
    }

    if (left.sheetHeight != right.sheetHeight)
    {
        return false;
    }

    return left.shapes == right.shapes;
}

bool operator==(const PluginOutput &left, const PluginOutput &right) { return left.sheets == right.sheets; }

bool operator==(const PluginMetadata &left, const PluginMetadata &right)
{
    return left.uniqueName == right.uniqueName && left.type == right.type && left.author == right.author && left.license == right.license;
}

/*
qreal BakeryPlugins::outputScore(const PluginOutput &output)
{
    if (output.sheets.isEmpty())
    {
        return 0;
    }
    qreal density = 0;
    foreach (Sheet sheet, output.sheets)
    {
        density += sheet.density();
    }
    return density * 100 / (output.sheets.size() * output.sheets.size());
}
*/

qreal BakeryPlugins::outputScore(const PluginOutput &output)
{
    if (output.sheets.isEmpty())
    {
        return 0;
    }
    qreal utilitization = 0;
    foreach (Sheet sheet, output.sheets)
    {
        utilitization += sheet.utilitization();
    }
    return utilitization * 100 / output.sheets.size();
}

bool operator<(const PluginOutput &left, const PluginOutput &right)
{
    return BakeryPlugins::outputScore(left) <= BakeryPlugins::outputScore(right);
}

StandardInputReader::StandardInputReader(QObject *parent) : QThread(parent) {}

void StandardInputReader::run()
{
    QFile standardInputFile;
    standardInputFile.open(stdin, QFile::ReadOnly);
    forever
    {
        QByteArray data = standardInputFile.readLine();
        emit read(data);
    }
}

PluginWrapper::PluginWrapper(QObject *instance, QObject *parent) : QObject(parent), _instance(instance), _standardInputReader(this)
{
    connect(this, SIGNAL(giveMetadata()), _instance, SLOT(giveMetadata()));
    connect(this, SIGNAL(bakeSheets(PluginInput)), _instance, SLOT(bakeSheets(PluginInput)));
    connect(this, SIGNAL(terminate(qint32)), _instance, SLOT(terminate(qint32)));
    connect(&_standardInputReader, SIGNAL(read(QByteArray)), this, SLOT(readFromStandardInput(QByteArray)));
    connect(_instance, SIGNAL(metadataGiven(PluginMetadata)), this, SLOT(metadataGiven(PluginMetadata)));
    connect(_instance, SIGNAL(outputUpdated(PluginOutput)), this, SLOT(outputUpdated(PluginOutput)));
    connect(_instance, SIGNAL(finished(PluginOutput)), this, SLOT(finished(PluginOutput)));
}

void PluginWrapper::run()
{
    _standardOutputFile.open(stdout, QFile::WriteOnly);
    _standardInputReader.start();
}

bool PluginWrapper::writeToStandardOutput(QString data)
{
    _standardOutputFile.write(QString(data + "\n").toLatin1().data());
    _standardOutputFile.flush();
    return _standardOutputFile.waitForBytesWritten(2000);
}

void PluginWrapper::readFromStandardInput(QByteArray data)
{
    QTextStream stream(data);
    QString command;
    stream >> command;
    command = command.trimmed();
    if (command == "give_metadata")
    {
        emit giveMetadata();
        return;
    }
    if (command == "bake_sheets")
    {
        PluginInput input;
        stream >> input;
        emit bakeSheets(input);
        return;
    }
    if (command == "terminate")
    {
        int msec;
        stream >> msec;
        emit terminate(msec);
        return;
    }
}

void PluginWrapper::metadataGiven(PluginMetadata meta)
{
    QString data;
    QTextStream stream(&data);
    stream << meta;
    writeToStandardOutput(data);
    QCoreApplication::exit();
}

void PluginWrapper::outputUpdated(PluginOutput output)
{
    QString data;
    QTextStream stream(&data);
    stream << output;
    writeToStandardOutput(data);
}

void PluginWrapper::finished(PluginOutput output)
{
    QString data;
    QTextStream stream(&data);
    stream << output;
    writeToStandardOutput(data);
    QCoreApplication::exit();
}

PluginRunner::PluginRunner(QString pluginName, QString pluginPath, PluginInput pluginInput, QObject *parent)
    : QObject(parent), _pluginName(pluginName), _pluginPath(pluginPath), _pluginInput(pluginInput)
{
    connect(&_process, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
    connect(&_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
}

bool PluginRunner::run()
{
    _process.start(_pluginPath);
    if (!_process.waitForStarted(5000))
    {
        BAKERY_CRITICAL(QString("Plugin process '%1' failed to start").arg(_pluginPath));
        return false;
    }

    QString data;
    QTextStream stream(&data);
    stream << "bake_sheets " << _pluginInput;
    return write(data);
}

bool PluginRunner::write(QString data)
{
    _process.write(QString(data + "\n").toLatin1().data());
    return _process.waitForBytesWritten(2000);
}

bool PluginRunner::terminate(int timeout)
{
    if (!write(QString("terminate %1 ").arg(timeout)))
    {
        return false;
    }
    QTimer::singleShot(timeout, this, SLOT(kill()));
    return true;
}

void PluginRunner::kill() { _process.kill(); }

void PluginRunner::processReadyRead()
{
    while (_process.canReadLine())
    {
        QByteArray buffer = _process.readLine();
        QTextStream stream(buffer);
        PluginOutput output;
        stream >> output;
        if (stream.status() != QTextStream::Ok)
        {
            BAKERY_CRITICAL(QString("Plugin '%1': invalid output received").arg(_pluginName));
            kill();
            return;
        }
        _pluginOutput = output;
        emit outputUpdated(_pluginName, output);
    }
}

void PluginRunner::processFinished(int exitCode) { emit finished(exitCode, _pluginName, _pluginInput, _pluginOutput); }
