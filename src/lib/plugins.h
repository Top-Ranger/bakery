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

#ifndef BAKERY_PLUGINS_H
#define BAKERY_PLUGINS_H

#include "global.h"
#include "shape.h"
#include "sheet.h"

#include <QList>
#include <QString>
#include <QFile>
#include <QProcess>
#include <QThread>
#include <QCoreApplication>

/*!
 * \brief Contains all information required for plugins to process tasks.
 */
struct BAKERYSHARED_EXPORT PluginInput
{
    /*!
     * \brief Height of sheets
     */
    qint32 sheetWidth;

    /*!
     * \brief Width of sheets
     */
    qint32 sheetHeight;

    /*!
     * \brief List of shapes.
     *
     * Each shape in this list has to be placed on a sheet. Identical shapes are contained as often in this list as they should be placed by
     * the plugins.
     */
    QList<Shape> shapes;

    /*!
     * \brief PluginInput Constructor
     */
    PluginInput() : sheetWidth(0), sheetHeight(0), shapes() {}
};

/*!
 * \brief Contains the results of tasks processed by plugins.
 */
struct BAKERYSHARED_EXPORT PluginOutput
{
    /*!
     * \brief List of computed sheets.
     */
    QList<Sheet> sheets;

    /*!
     * \brief PluginOutput constructor.
     */
    PluginOutput() : sheets() {}
};

/*!
 * \brief Contains plugin metadata.
 */
struct BAKERYSHARED_EXPORT PluginMetadata
{
    /*!
     * \brief The unique name of the plugin.
     *
     * Each plugin has to have an unique name by which the plugin is identified.
     */
    QString uniqueName;

    /*!
     * \brief The type of the plugin.
     *
     * Some examples for types are:
     *  - greedy
     *  - randomized
     *  - search
     *  - metaheuristic search
     */
    QString type;

    /*!
     * \brief The author(s) of the plugin.
     */
    QString author;

    /*!
     * \brief License of the plugin.
     */
    QString license;

    /*!
     * \brief PluginMetadata constructor
     *
     * This constructor sets all values to "<invalid>" or "<unknown>".
     */
    PluginMetadata() : uniqueName("<invalid>"), type("<unknown>"), author("<unknown>"), license("<unknown>") {}

    /*!
     * \brief PluginMetadata constructor.
     *
     * With this constructor it is possible to set all values at the time of construction.
     *
     * \param uniqueName_ Unique name of plugin.
     * \param type_ Type of plugin.
     * \param author_ Author of plugin.
     * \param license_ License of plugin.
     */
    PluginMetadata(QString uniqueName_, QString type_, QString author_, QString license_)
        : uniqueName(uniqueName_), type(type_), author(author_), license(license_)
    {
    }
};

/*!
 * \brief Struct to store parameters for random PluginInput generation.
 */
struct BAKERYSHARED_EXPORT RandomPluginInputParameters
{
    /*!
     * \brief Minimum sheet width.
     */
    qint32 minSheetWidth;

    /*!
     * \brief Maximum sheet width.
     */
    qint32 maxSheetWidth;

    /*!
     * \brief Minimum sheet height.
     */
    qint32 minSheetHeight;

    /*!
     * \brief Maximum sheet height.
     */
    qint32 maxSheetHeight;

    /*!
     * \brief Minimum number of shape types.
     */
    qint32 minShapes;

    /*!
     * \brief Maximum number of shape types.
     */
    qint32 maxShapes;

    /*!
     * \brief Minimum number of shapes of each type.
     */
    qint32 minAmount;

    /*!
     * \brief Maximum number of shapes of each type.
     */
    qint32 maxAmount;

    /*!
     * \brief Minimum number of points of each shape.
     */
    qint32 minPoints;

    /*!
     * \brief Maximum number of points of each shape.
     */
    qint32 maxPoints;

    /*!
     * \brief Minimum shape scale.
     */
    qint32 minScale;

    /*!
     * \brief Maximum shape scale.
     */
    qint32 maxScale;

    /*!
     * \brief Minimum edge angle in degrees.
     */
    qreal minAngle;

    /*!
     * \brief Minimum number of sheets.
     */
    qint32 minSheets;

    /*!
     * \brief Maximum number of sheets.
     */
    qint32 maxSheets;

    /*!
     * \brief Seed for random generation.
     *
     * When the same seed is specified, the same output is generated.
     * If seed is set to -1, a random seed will be used.
     */
    qint32 seed;

    // Default values
    RandomPluginInputParameters()
    {
        minSheetWidth = 2;
        maxSheetWidth = 5;
        minSheetHeight = 2;
        maxSheetHeight = 5;
        minShapes = 2;
        maxShapes = 6;
        minAmount = 5;
        maxAmount = 15;
        minPoints = 3;
        maxPoints = 8;
        minScale = 1;
        maxScale = 3;
        minAngle = 30;
        minSheets = 5;
        maxSheets = 5;
        seed = -1;
    }
};

/*!
 * \brief Non-blocking standard input reader.
 */
class BAKERYSHARED_EXPORT StandardInputReader : public QThread
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param parent QObject parent.
     */
    explicit StandardInputReader(QObject *parent = 0);

    /*!
     * \brief Indefinately reads lines from standard input and emits read() when a line has been read.
     */
    void run();

signals:
    /*!
     * \brief Is emitted when a line has been read.
     * \param data Byte representation of the read line.
     */
    void read(QByteArray data);
};

/*!
 * \brief Wrapper for plugin instances. Provides input and output logic. Handles signals and slots.
 *
 * Plugins are required to implement the slots giveMetadata(), bakeSheets(PluginInput) and terminate(qint32)
 * and emit metadataGiven(PluginMetadata), outputUpdated(PluginOutput) and finished(PluginOutput).
 */
class BAKERYSHARED_EXPORT PluginWrapper : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     * \param instance Plugin instance.
     * \param parent QObject parent.
     */
    explicit PluginWrapper(QObject *instance = 0, QObject *parent = 0);

private:
    /*!
     * \brief Pointer to plugin class instance.
     */
    QObject *_instance;

    /*!
     * \brief Used to receive commands.
     */
    StandardInputReader _standardInputReader;

    /*!
     * \brief Used to send results.
     */
    QFile _standardOutputFile;

    /*!
     * \brief Writes a Latin-1 representation of the given string standard input.
     * \param data String.
     * \return true if successful within 2000 ms.
     */
    bool writeToStandardOutput(QString data);

signals:
    /*!
     * \brief Is emitted when the command "give_metadata" is received via standard input.
     */
    void giveMetadata();

    /*!
     * \brief Is emitted when the command "bake_sheets" is received via standard input.
     */
    void bakeSheets(PluginInput input);

    /*!
     * \brief Is emitted when the command "terminate" is received via standard input.
     */
    void terminate(qint32 msec);

public slots:
    /*!
     * \brief Starts the plugin wrapper main loop by opening the standard output file and starting the standard input reader.
     */
    void run();

private slots:
    /*!
     * \brief Reads a line from standard input, parses commands and emits signals if appropriate.
     * \param data Data from standard input reader.
     */
    void readFromStandardInput(QByteArray data);

    /*!
     * \brief Writes metadata provided by plugins to standard output.
     * Plugins are required to emit the corresponding signal metadataGiven(PluginMetadata).
     * \param meta Metadata.
     */
    void metadataGiven(PluginMetadata meta);

    /*!
     * \brief Writes output provided by plugins to standard output.
     * Plugins are required to emit the corresponding signal outputUpdated(PluginOutput).
     * \param output PluginOutput.
     */
    void outputUpdated(PluginOutput output);

    /*!
     * \brief Writes metadata provided by plugins to standard output and exits the main loop.
     * Plugins are required to emit the corresponding signal finished(PluginOutput).
     * \param output PluginOutput.
     */
    void finished(PluginOutput output);
};

/*!
 * \brief Signal-emitting convenience plugin runner.
 */
class BAKERYSHARED_EXPORT PluginRunner : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor.
     * \param pluginName Name of the plugin.
     * \param pluginPath Path to the plugin executable.
     * \param pluginInput Input to be processed.
     * \param parent QObject parent.
     */
    explicit PluginRunner(QString pluginName = QString(), QString pluginPath = QString(), PluginInput pluginInput = PluginInput(),
                          QObject *parent = 0);

    /*!
     * \brief Starts the plugin process and sends the command "bake_sheets" to the plugin via standard output.
     * \return true if successful.
     */
    bool run();

    /*!
     * \brief Writes a Latin-1 representation of the given string to the plugin's standard input channel.
     * \param data String.
     * \return true if successful.
     */
    bool write(QString data);

    /*!
     * \brief Sends the "terminate" command to the plugin.
     * \param timeout Timeout in milliseconds.
     * \return true if successful.
     */
    bool terminate(int timeout);

private:
    /*!
     * \brief Name of the plugin. Required to be unique.
     */
    QString _pluginName;

    /*!
     * \brief Path to the plugin executable.
     */
    QString _pluginPath;

    /*!
     * \brief PluginInput used for processing.
     */
    PluginInput _pluginInput;

    /*!
     * \brief Resulting PluginOutput.
     */
    PluginOutput _pluginOutput;

    /*!
     * \brief Plugin process.
     */
    QProcess _process;

signals:
    /*!
     * \brief Is emitted when a plugin's output is updated.
     * \param pluginName Name of the plugin.
     * \param output Current output.
     */
    void outputUpdated(QString pluginName, PluginOutput output);

    /*!
     * \brief Is emitted when a plugin has finished working.
     * \param exitCode The plugin process' exit code.
     * \param pluginName Name of the plugin.
     * \param input Processed input (for GUI convenience).
     * \param output Final output.
     */
    void finished(int exitCode, QString pluginName, PluginInput input, PluginOutput output);

private slots:
    /*!
     * \brief Kills the process.
     */
    void kill();

    /*!
     * \brief Reads lines from the process standard output channel, parses them and emits outputUpdated(QString,
     * PluginOutput) if applicable.
     */
    void processReadyRead();

    /*!
     * \brief Emits finished(int, QString, PluginInput, PluginOutput).
     * \param exitCode Process exit code.
     */
    void processFinished(int exitCode);
};

// Convenience plugin main function macro.
#define BAKERY_PLUGIN_MAIN(className)                                                                                                      \
    int main(int argc, char *argv[])                                                                                                       \
    {                                                                                                                                      \
        QCoreApplication application(argc, argv);                                                                                          \
        className instance;                                                                                                                \
        PluginWrapper wrapper(&instance);                                                                                                  \
        wrapper.run();                                                                                                                     \
        return application.exec();                                                                                                         \
    }

// Convenience plugin log text stream macro.
#define LOG_TEXT_STREAM(fileName)                                                                                                          \
    QFile logFile(fileName);                                                                                                               \
    logFile.open(QFile::WriteOnly);                                                                                                        \
    QTextStream log(&logFile);

/*!
 * \relates PluginInput
 * \brief Operator overloading to send a PluginInput object to a text stream.
 * \param stream Target text stream.
 * \param input PluginInput to send.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator<<(QTextStream &stream, const PluginInput &input);

/*!
 * \relates PluginInput
 * \brief Operator overloading to create a PluginInput object from a text stream.
 *
 * Text stream status will be set to QTextStream::ReadCorruptData if the PluginInput could not be created from stream.
 *
 * \param stream Target text stream.
 * \param input Reference to PluginInput in which to create object.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator>>(QTextStream &stream, PluginInput &input);

/*!
 * \relates PluginOutput
 * \brief Operator overloading to send a PluginOutput object to a text stream.
 * \param stream Target text stream.
 * \param output PluginOutput to send.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator<<(QTextStream &stream, const PluginOutput &output);

/*!
 * \relates PluginOutput
 * \brief Operator overloading to create a PluginOutput object from a text stream.
 *
 * Text stream status will be set to QTextStream::ReadCorruptData if the PluginOutput could not be created from stream.
 *
 * \param stream Target text stream.
 * \param output Reference to PluginOutput in which to create object.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator>>(QTextStream &stream, PluginOutput &output);

/*!
 * \relates PluginMetadata
 * \brief Operator overloading to send a PluginMetadata object to a text stream.
 * \param stream Target text stream.
 * \param meta PluginMetadata to send.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator<<(QTextStream &stream, const PluginMetadata &meta);

/*!
 * \relates PluginMetadata
 * \brief Operator overloading to create a PluginMetadata object from a text stream.
 *
 * Text stream status will be set to QTextStream::ReadCorruptData if the PluginMetadata could not be created from stream.
 *
 * \param stream Target text stream.
 * \param meta Reference to PluginMetadata in which to create object.
 * \return Reference to text stream.
 */
BAKERYSHARED_EXPORT QTextStream &operator>>(QTextStream &stream, PluginMetadata &meta);

/*!
 * \relates PluginInput
 * \brief Operator to compare two PluginInput objects.
 *
 * Two PluginInput objects are considered equal if all of the following conditions are met:
 *  - PluginInput::sheetWidth is equal.
 *  - PluginInput::sheetHeight is equal.
 *  - PluginInput::shapes is equal.
 *
 * \param left Left side of comparison.
 * \param right Right side of comparison.
 * \return true if both are equal.
 */
BAKERYSHARED_EXPORT bool operator==(const PluginInput &left, const PluginInput &right);

/*!
 * \relates PluginOutput
 * \brief Operator to compare two PluginOutput objects.
 *
 * Two PluginOutput objects are considered equal if all of the following conditions are met:
 *  - PluginOutput::sheets is equal.
 *
 * \param left Left side of comparison.
 * \param right Right side of comparison.
 * \return true if both are equal.
 */
BAKERYSHARED_EXPORT bool operator==(const PluginOutput &left, const PluginOutput &right);

/*!
 * \relates PluginMetadata
 * \brief Operator to compare two PluginMetadata objects.
 *
 * Two PluginOutput objects are considered equal if all of the following conditions are met:
 *  - PluginMetadata::uniqueName is equal.
 *  - PluginMetadata::author is equal.
 *  - PluginMetadata::type is equal.
 *  - PluginMetadata::license is equal.
 *
 * \param left Left side of comparison.
 * \param right Right side of comparison.
 * \return true if both are equal.
 */
BAKERYSHARED_EXPORT bool operator==(const PluginMetadata &left, const PluginMetadata &right);

/*!
 * \relates PluginOutput
 * \brief Same as BakeryPlugins::outputScore(left) <= BakeryPlugins::outputScore(right).
 * \param left First PluginOutput.
 * \param right Second PluginOutput.
 * \return
 */
BAKERYSHARED_EXPORT bool operator<(const PluginOutput &left, const PluginOutput &right);

/*!
 * \relates PluginInput
 * \brief Operator to compare two PluginInput objects.
 * \param left Left side of comparison.
 * \param right Right side of comparison.
 * \return true if both are not equal.
 * \sa operator==(const PluginInput &left, const PluginInput &right)
 */
inline bool operator!=(const PluginInput &left, const PluginInput &right) { return !(left == right); }

/*!
 * \relates PluginOutput
 * \brief Operator to compare two PluginOutput objects.
 * \param left Left side of comparison.
 * \param right Right side of comparison.
 * \return true if both are not equal.
 * \sa operator==(const PluginOutput &left, const PluginOutput &right)
 */
inline bool operator!=(const PluginOutput &left, const PluginOutput &right) { return !(left == right); }

/*!
 * \relates PluginMetadata
 * \brief Operator to compare two PluginMetadata objects.
 * \param left Left side of comparison.
 * \param right Right side of comparison.
 * \return true if both are not equal.
 * \sa operator==(const PluginMetadata &left, const PluginMetadata &right);
 */
inline bool operator!=(const PluginMetadata &left, const PluginMetadata &right) { return !(left == right); }

namespace BakeryPlugins
{
/*!
 * \brief Computes the score of a PluginOutput. If it has no sheets, the score is 0. Otherwise the score is arithmeticMean([sheet
 * densities]) / [number of sheets]^2.
 * \sa Sheet::density()
 * \return Score.
 */
BAKERYSHARED_EXPORT qreal outputScore(const PluginOutput &output);
}

#endif // BAKERY_PLUGINS_H
