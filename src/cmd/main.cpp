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

#include "../lib/bakery.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QRegularExpression>

#define VERSION "1.0.0"

// Command-line interface main function
int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName("bakery-cmd");
    QCoreApplication::setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("Bakery command line interface");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("input", "Input file path.");

    QCommandLineOption outputDirectoryPathOption(QStringList() << "o"
                                                               << "output-directory-path",
                                                 "Will be created if it does not exist. Default: ./output", "path", "./output");
    parser.addOption(outputDirectoryPathOption);

    QCommandLineOption resultsFileNameOption(QStringList() << "r"
                                                           << "results-file-name",
                                             "Default: results.txt", "name", "results.txt");
    parser.addOption(resultsFileNameOption);

    QCommandLineOption timeLimitOption(QStringList() << "t"
                                                     << "time-limit",
                                       "0 for none. Default: 0", "seconds", "0");
    parser.addOption(timeLimitOption);

    QCommandLineOption listPluginsOption(QStringList() << "l"
                                                       << "list-plugins",
                                         "List available plugins.");
    parser.addOption(listPluginsOption);

    QCommandLineOption allOutputsOption(QStringList() << "a"
                                                      << "all-outputs",
                                        "Save all algorithm outputs to separate subdirectories.");
    parser.addOption(allOutputsOption);

    QCommandLineOption svgOutputOption(QStringList() << "s"
                                                     << "svg-output",
                                       "Save SVG files to output directory.");
    parser.addOption(svgOutputOption);

    QCommandLineOption generateRandomOption(QStringList() << "generate-random",
                                            "Saves <count> randomly generated input files to output directory.", "count", "");
    parser.addOption(generateRandomOption);

    QCommandLineOption disabledPluginsOption(QStringList() << "d"
                                                           << "disabled-plugins",
                                             "Plugins with names matching the regular expression will be disabled.", "regex", "");
    parser.addOption(disabledPluginsOption);

    QCommandLineOption versionOption(QStringList() << "license", "Print license information and exit.");
    parser.addOption(versionOption);

    // Process options
    parser.process(application);
    const QStringList positional = parser.positionalArguments();

    // Print version / license information and exit
    if (parser.isSet(versionOption))
    {
        QTextStream stream(stdout, QIODevice::WriteOnly);
        stream << "Bakery command line interface (version " << VERSION << ")\n";
        stream << "Copyright (C) 2015,2016 Philipp Naumann \n";
        stream << "Copyright (C) 2015,2016 Marcus Soll \n \n";
        stream << "bakery-cmd is free software: you can redistribute it and/or modify \n"
                  "it under the terms of the GNU Lesser General Public License \n"
                  "as published by the Free Software Foundation, either version 3 of the License, or \n"
                  "(at your option) any later version. \n"
                  "\n"
                  "bakery-cmd is distributed in the hope that it will be useful, \n"
                  "but WITHOUT ANY WARRANTY; without even the implied warranty of \n"
                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \n"
                  "GNU Lesser General Public License for more details. \n"
                  "\n"
                  "You should have received a copy of the GNU Lesser General Public License \n"
                  "along with bakery-cmd. If not, see <http://www.gnu.org/licenses/>. \n"
                  "\n"
                  "\n"
                  "The Qt Toolkit is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).\n"
                  "You may use, distribute and copy the Qt GUI Toolkit under the terms of \n"
                  "GNU Lesser General Public License version 3, which supplements GNU General \n"
                  "Public License Version 3. You should have received a copy of the license \n"
                  "along with this application \n \n";
        stream.flush();
        exit(0);
    }

    if (parser.isSet(generateRandomOption))
    {
        bool ok;
        int randomCount = parser.value(generateRandomOption).toInt(&ok);
        if (!ok)
        {
            BAKERY_CRITICAL(QString("Invalid value for number of random input files ('%1')").arg(parser.value(generateRandomOption)));
            return EXIT_FAILURE;
        }
        if (randomCount < 1 || randomCount > 100000)
        {
            BAKERY_CRITICAL(QString("Number of random input files is out of range (1-100000)").arg(randomCount));
            return EXIT_FAILURE;
        }

        QString outputDirectoryPath = parser.value(outputDirectoryPathOption);
        if (!QDir(outputDirectoryPath).exists())
        {
            if (!QDir().mkpath(outputDirectoryPath))
            {
                BAKERY_CRITICAL(QString("Failed to create output directory \"%1\"").arg(outputDirectoryPath));
                exit(1);
            }
        }

        RandomPluginInputParameters parameters;

        for (int i = 0; i < randomCount; ++i)
        {
            QFile outputFile(QDir(outputDirectoryPath).absoluteFilePath(QString("random%1.txt").arg(i, 5, 10, QChar('0'))));
            if (!outputFile.open(QFile::WriteOnly | QFile::Truncate))
            {
                BAKERY_CRITICAL(QString("Failed to open output file '%1'' for writing").arg(outputFile.fileName()));
                exit(1);
            }

            parameters.seed = i;

            PluginInput randomInput = Bakery::randomInput(parameters);
            Bakery::saveToDevice(randomInput, &outputFile);
        }
        exit(0);
    }

    // Instantiate bakery
    Bakery bakery;

    // Available plugins
    if (parser.isSet(listPluginsOption))
    {
        QTextStream stream(stdout, QIODevice::WriteOnly);
        stream << "Available plugins:\n";
        foreach (QString pluginName, bakery.getAllPlugins())
        {
            stream << QString("- %1\n").arg(pluginName);
        }
        return EXIT_SUCCESS;
    }

    // Disabled plugins
    if (parser.isSet(disabledPluginsOption))
    {
        QString pattern(parser.value(disabledPluginsOption));
        QRegularExpression disabledExpression(pattern);
        if (!disabledExpression.isValid())
        {
            BAKERY_CRITICAL(QString("Regular expression '%1' is invalid").arg(pattern));
            return EXIT_FAILURE;
        }
        foreach (QString pluginName, bakery.getAllPlugins())
        {
            if (disabledExpression.match(pluginName).hasMatch())
            {
                BAKERY_DEBUG(QString("Disabling plugin '%1'").arg(pluginName));
                bakery.setPluginEnabled(pluginName, false);
            }
        }
    }
    if (bakery.getEnabledPlugins().isEmpty())
    {
        BAKERY_CRITICAL("There are no plugins available/enabled");
        return EXIT_FAILURE;
    }

    // Positional arguments
    if (positional.size() == 0)
    {
        parser.showHelp();
        return EXIT_FAILURE;
    }

    // Input file
    QString inputFilePath = positional[0];
    bool ok;
    QFile inputFile(inputFilePath);
    if (!inputFile.open(QFile::ReadOnly))
    {
        BAKERY_CRITICAL(QString("Failed to open input file '%1'").arg(inputFilePath));
        return EXIT_FAILURE;
    }
    PluginInput input = Bakery::loadFromDevice(&inputFile, &ok);
    inputFile.close();
    if (!ok)
    {
        BAKERY_CRITICAL(QString("Failed to load plugin input from file '%1'").arg(inputFilePath));
        return EXIT_FAILURE;
    }

    // Time limit
    int timeLimit = 0;
    if (parser.isSet(timeLimitOption))
    {
        timeLimit = parser.value(timeLimitOption).toInt(&ok);
        if (!ok)
        {
            BAKERY_CRITICAL(QString("Invalid value for time limit ('%1')").arg(inputFilePath));
            return EXIT_FAILURE;
        }
        if (timeLimit < 0)
        {
            BAKERY_CRITICAL(QString("Time limit is less than 0").arg(inputFilePath));
            return EXIT_FAILURE;
        }
        BAKERY_DEBUG(QString("Imposing time limit of %1 seconds").arg(timeLimit));
    }
    bakery.setTimeLimit(timeLimit * 1000);

    // Get all outputs
    bool svgOutput = parser.isSet(svgOutputOption);
    QHash<QString, PluginOutput> outputs = bakery.computeAllOutputs(input, true, &ok);
    if (!ok)
    {
        BAKERY_CRITICAL("Failed to get plugin outputs");
        return EXIT_FAILURE;
    }
    if (outputs.isEmpty())
    {
        BAKERY_WARNING(QString("No plugin found a valid solution (time limit: %1)")
                           .arg(timeLimit == 0 ? "none" : QString("%1 second(s)").arg(timeLimit)));
        return EXIT_FAILURE;
    }
    else
    {
        QStringList valid(outputs.keys());
        BAKERY_DEBUG(QString("Valid solutions found by: %1").arg(valid.join(", ")));
    }

    // Save all outputs/best output
    if (parser.isSet(allOutputsOption))
    {
        // All outputs
        QString outputDirectoryPath = parser.value(outputDirectoryPathOption);
        QString resultsFileName = parser.value(resultsFileNameOption);
        for (qint32 i = 0; i < outputs.size(); ++i)
        {
            QString pluginName = outputs.keys()[i];
            if (!Bakery::saveToDirectory(outputs.values()[i], QString(QDir(outputDirectoryPath).absoluteFilePath(pluginName)),
                                         resultsFileName, svgOutput))
            {
                BAKERY_CRITICAL(QString("Failed to save output of plugin '%1'").arg(pluginName));
                return EXIT_FAILURE;
            }
        }
    }
    else
    {
        // Best output only
        PluginOutput output = Bakery::findBestOutput(outputs);
        QString outputDirectoryPath = parser.value(outputDirectoryPathOption);
        QString resultsFileName = parser.value(resultsFileNameOption);
        if (!Bakery::saveToDirectory(output, outputDirectoryPath, resultsFileName, svgOutput))
        {
            BAKERY_CRITICAL("Failed to save output");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
