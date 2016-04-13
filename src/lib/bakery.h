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

#ifndef BAKERY_H
#define BAKERY_H

#include "global.h"
#include "plugins.h"
#include "sheet.h"
#include "shape.h"

#include <QObject>
#include <QDir>
#include <QPluginLoader>
#include <QList>
#include <QIODevice>
#include <QHash>
#include <QMap>
#include <QProcess>

/*!
 * \brief Main interface for libbakery. Handles plugin input/output.
 */
class BAKERYSHARED_EXPORT Bakery : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Parses an input file and returns a corresponding PluginInput.
     * \param device Input device.
     * \param ok Will be set to true if no errors occur. Will be ignored if set to NULL.
     * \return Corresponding PluginInput.
     */
    static PluginInput loadFromDevice(QIODevice *device, bool *ok = NULL);

    /*!
     * \brief Parses an SVG file and returns a corresponding PluginInput.
     *
     * Currently supported SVG elements:
     *  - rect
     *  - circle
     *  - ellipse
     *  - polygon
     *
     *
     * Please note: The SVG path element is not supported because it can create multi-polygons which can not be represented in the defaut
     * input / output format.
     *
     * \param device Input device.
     * \param ok Will be set to true if no errors occur. Will be ignored if set to NULL.
     * \return Corresponding PluginInput.
     */
    static PluginInput loadFromSVG(QIODevice *device, bool *ok = NULL);

    /*!
     * \brief Generates a random PluginInput.
     * \param parameters Parameters for the generation process.
     * \return Generated PluginInput.
     */
    static PluginInput randomInput(RandomPluginInputParameters parameters);

    /*!
     * \brief Saves a single PluginInput to a device.
     * \param input PluginInput to save.
     * \param device Target device.
     * \return true if successful.
     */
    static bool saveToDevice(PluginInput input, QIODevice *device);

    /*!
     * \brief Saves a single PluginOutput to a device.
     * \param output PluginOutput to save.
     * \param device Target device.
     * \return true if successful.
     */
    static bool saveToDevice(PluginOutput output, QIODevice *device);

    /*!
     * \brief Convenience method to save a PluginOutput in a directory.
     *
     * This method also takes care of creating the directories and saving an additional SVG (optional).
     *
     * \param output PluginOutput which should be saved.
     * \param outputDirectoryPath Target directory.
     * \param resultsFileName Target file name.
     * \param svgOutput If true additional SVG output will be saved.
     * \return true if successful.
     */
    static bool saveToDirectory(PluginOutput &output, QString outputDirectoryPath, QString resultsFileName, bool svgOutput);

    /*!
     * \brief Saves a single PluginOutput to files in SVG format.
     *
     * Each shape is saved as a SVG polygon element. The width / height of the sheet is saved in the corresponding SVG attribute. Each sheet
     * is saved in a different file.
     *
     * \param output PluginOutput to save.
     * \param outputDir Target directory.
     * \param filePrefix Prefix of files.
     * \return true if successful.
     */
    static bool saveAsSVG(PluginOutput output, QDir outputDir, QString filePrefix = QString("bakery"));

    /*!
     * \brief Checks if the output could be a valid result of a given input.
     *
     * This is determined by checking the following conditions:
     *  - All sheets in PluginOutput are valid.
     *  - Each shape from PluginInput is on a sheet in PluginOutput (check by name).
     *  - No additional Shape is in PluginOutput.
     *  - Width / height of each Sheet matches required width / height,
     *
     * \param input PluginInput to check.
     * \param output PluginOutput to check.
     * \return true if output is valid and a possible result of the input.
     */
    static bool isOutputValidForInput(const PluginInput &input, const PluginOutput &output);

    /*!
     * \brief Finds the best outputs out of a Hash of PluginOutputs.
     *
     * It is not checked whether the outputs are valid.
     *
     * \param outputs Hash of PluginOutputs.
     * \return Best PluginOutput.
     */
    static PluginOutput findBestOutput(QHash<QString, PluginOutput> &outputs);

    /*!
     * \brief Bakery Construnctor
     * \param parent QObject parent.
     * \param pluginDir Directory from which plugins are loaded.
     */
    Bakery(QObject *parent = 0, QDir pluginDir = QDir("./plugins/"));

    /*!
     * \brief Bakery deconstructor.
     */
    ~Bakery();

    /*!
     * \brief Loads a single plugin.
     * \param path Path to plugin executeable.
     * \return true if successful.
     */
    bool loadPlugin(QString path);

    /*!
     * \brief Tries to load plugins from a directory.
     *
     * A file is considered a plugin candidate if QFileInfo::isExecutable() returns true. This function does not search for plugins
     * recursively.
     *
     * \param directory Directory from which to load plugins.
     * \return true if at least one plugin could be loaded.
     */
    bool loadPluginsFromDirectory(QDir directory);

    /*!
     * \brief Tests if a plugin is loaded.
     * \param pluginName Plugin to test.
     * \return true if plugin is loaded.
     */
    bool isPluginLoaded(QString pluginName) const;

    /*!
     * \brief Runs all enabled plugins on the given PluginInput and returns a hash of all outputs.
     * \param input PluginInput.
     * \param synchronous If set to true this method is blocking.
     * \param ok Will be set to true if no errors occur. Will be ignored if set to NULL.
     * \return Hash with results. Will be empty in non-blocking mode.
     */
    QHash<QString, PluginOutput> computeAllOutputs(PluginInput input, bool synchronous, bool *ok = 0);

    /*!
     * \brief Runs all plugins on the given PluginInput and returns the best output.
     *
     * This method is blocking.
     *
     * \param input PluginInput.
     * \param ok Will be set to true if no errors occur. Will be ignored if set to NULL.
     * \return Best PluginOutput.
     */
    PluginOutput computeBestOutput(PluginInput input, bool *ok);

    /*!
     * \brief Runs all plugins on the a PluginInput loaded from a given device and returns the best output.
     *
     * This method is blocking.
     *
     * \param device The device from which to load the input from.
     * \param ok Will be set to true if no errors occur. Will be ignored if set to NULL.
     * \return Best PluginOutput.
     */
    PluginOutput computeBestOutput(QIODevice *device, bool *ok);

    /*!
     * \brief Terminates a given plugin.
     *
     * The plugin will be notified and has the specified time to finish its calculation before it gets killed.
     *
     * \param pluginName Name of the plugin to terminate.
     * \param msec Time before the plugin gets killed (milliseconds).
     * \return true if successful.
     */
    bool terminatePlugin(QString pluginName, int msec);

    /*!
     * \brief Terminates all plugins.
     *
     * Each plugin will be notified and has the specified time to finish its calculation before it gets killed.
     *
     * \param msec Time before the plugin gets killed (milliseconds).
     * \return true if successful.
     */
    bool terminateAllPlugins(int msec);

    /*!
     * \brief Kills a given plugin.
     *
     * The execution of the plugin is stopped immediately. It has no time to finish calculation or return a stored result.
     *
     * \param pluginName Name of the plugin to kill.
     * \return true if successful.
     */
    bool killPlugin(QString pluginName);

    /*!
     * \brief Kills all plugins
     *
     * The execution of all plugins is stopped immediately. They have no time to finish calculation or return a stored result.
     *
     * \return
     */
    bool killAllPlugins();

    /*!
     * \brief Returns a list of all plugin names.
     * \return List of all plugin names.
     */
    QList<QString> getAllPlugins();

    /*!
     * \brief Returns a list of all enabled plugins.
     * \return List of all enabled plugins.
     */
    QList<QString> getEnabledPlugins();

    /*!
     * \brief Returns a list of all disabled plugins.
     * \return List of all disabled plugins.
     */
    QList<QString> getDisabledPlugins();

    /*!
     * \brief Enables / disables all plugins.
     * \param enabled If set to true all plugins are enabled, else disabled.
     */
    void setAllPluginsEnabled(bool enabled = true);

    /*!
     * \brief Enables / disables a single plugin.
     * \param pluginName Name of plugin.
     * \param enabled If set to true plugin is enabled, else disabled.
     */
    void setPluginEnabled(QString pluginName, bool enabled = true);

    /*!
     * \brief Returns if a plugin is enabled.
     * \param pluginName Name of plugin.
     * \return true if enabled.
     */
    bool isPluginEnabled(QString pluginName) const;

    /*!
     * \brief Returns the metadata of the plugin with the given name.
     * \param pluginName Name of plugin.
     * \return Metadata of plugin with given name.
     */
    PluginMetadata getPluginMetadata(QString pluginName) const;

    /*!
     * \brief Sets time limit before all plugins get killed automatically.
     *
     * If set to 0 plugins are not killed automatically. If set to a value not 0 the terminate signal will be sent immediately.
     *
     * \param timeLimit Time limit in milliseconds.
     */
    void setTimeLimit(qint32 timeLimit);

    /*!
     * \brief Returns the time limit before all plugins get killed automatically.
     *
     * If 0 plugins are not killed automatically. If value not 0 the terminate signal will be sent immediately.
     *
     * \return Time limit in milliseconds.
     */
    qint32 getTimeLimit();

private:
    /*!
     * \brief Hash containing the metadata for all loaded plugins.
     */
    QHash<QString, PluginMetadata> _pluginsMetadata;

    /*!
     * \brief Hash containing the location on the file system of all loaded plugins.
     */
    QHash<QString, QString> _pluginsPaths;

    /*!
     * \brief Hash containing all PluginRunners.
     *
     * Runners need to be cached in class because of non blocking mode.
     */
    QHash<QString, PluginRunner *> _pluginsRunners;

    /*!
     * \brief Hash containing the last valid output of a plugin.
     *
     * Outputs need to be cached in class because of non blocking mode.
     */
    QHash<QString, PluginOutput> _validOutputs;

    /*!
     * \brief Hash containing current settings.
     *
     * Right now the only setting is the status of the plugins (enabled/disabled). Additional settings may be saved later.
     */
    QHash<QString, QVariant> _settings;

private slots:
    /*!
     * \brief Slot which is called when a plugin sends a new output.
     * \param pluginName Name of plugin.
     * \param pluginOutput New plugin output.
     */
    void _pluginOutputUpdated(QString pluginName, PluginOutput pluginOutput);

    /*!
     * \brief Slot which is called when a plugin is finished.
     * \param exitCode Exit code of plugin.
     * \param pluginName Name of plugin.
     * \param pluginInput Used PluginInput.
     * \param pluginOutput Result of plugin.
     */
    void _pluginFinished(int exitCode, QString pluginName, PluginInput pluginInput, PluginOutput pluginOutput);

signals:
    /*!
     * \brief Signal emitted when a plugin is started.
     * \param uniqueName Name of plugin.
     */
    void pluginStarting(QString uniqueName);

    /*!
     * \brief Signal emitted when a plugin sends a new output.
     * \param uniqueName Name of plugin.
     * \param pluginOutput New output.
     */
    void pluginOutputUpdated(QString uniqueName, PluginOutput pluginOutput);

    /*!
     * \brief Signal emitted when a plugin will be terminated.
     * \param uniqueName Name of plugin.
     * \param msec Time before the plugin gets killed (milliseconds).
     */
    void pluginTerminating(QString uniqueName, int msec);

    /*!
     * \brief Signal emitted when a plugin finished its execution.
     * \param exitCode Exit code.
     * \param uniqueName Name of plugin.
     * \param pluginOutput Last received output of plugin.
     * \param valid true if output is valid.
     */
    void pluginFinished(int exitCode, QString uniqueName, PluginOutput pluginOutput, bool valid);

    /*!
     * \brief Signal emitted when all plugins have finished calculation.
     * \param pluginOutputs Hash of all valid outputs.
     */
    void allPluginsFinished(QHash<QString, PluginOutput> pluginOutputs);
};

#endif // BAKERY_H
