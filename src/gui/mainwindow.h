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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pluginoutputwidget.h"
#include "helpers.hpp"
#include "bakery.h"

#include <QListWidgetItem>
#include <QMainWindow>

namespace Ui
{
/*!
 * \brief Bakery GUI main window class.
 */
class MainWindow;
}

/*!
 * \brief Bakery GUI main window class.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * \brief MainWindow constructor
     * \param parent Parent QWidget.
     */
    explicit MainWindow(QWidget *parent = 0);

    /*!
     * \brief MainWindow destructor.
     * Terminates all plugins and waits for all plugin processes to finish.
     */
    ~MainWindow();

private slots:
    /*!
     * \brief Handles signal Bakery::pluginStarting.
     * Creates a corresponding PluginOutputWidget.
     * \param pluginName Plugin name.
     */
    void _pluginStarting(QString pluginName);

    /*!
     * \brief Handles signal Bakery::pluginTerminating.
     * Updates the corresponding PluginOutputWidget.
     * \param pluginName Plugin name.
     * \param msec Timeout. Value is ignored.
     */
    void _pluginTerminating(QString pluginName, int msec);

    /*!
     * \brief Handles signal Bakery::pluginOutputUpdated.
     * Updates the corresponding PluginOutputWidget.
     * \param pluginName Plugin name.
     * \param output Current PluginOutput.
     */
    void _pluginOutputUpdated(QString pluginName, PluginOutput output);

    /*!
     * \brief Handles signal Bakery::pluginFinished.
     * Updates the corresponding PluginOutputWidget according to output validity.
     * \param exitCode Plugin exit code.
     * \param pluginName Plugin name.
     * \param output Final PluginOutput.
     * \param valid PluginOutput validity.
     */
    void _pluginFinished(int exitCode, QString pluginName, PluginOutput output, bool valid);

    /*!
     * \brief Handles signal Bakery::allPluginsFinished.
     * Updates _validOutputs and _bestOutput and unsets working state.
     * \param pluginOutputs All valid PluginOutputs.
     */
    void _allPluginsFinished(QHash<QString, PluginOutput> pluginOutputs);

    /*!
     * \brief Enables/disables actions according to interface state.
     */
    void updateActions();

    /*!
     * \brief Closes the main window.
     */
    void on_actionFileQuit_triggered();

    /*!
     * \brief Is invoked when actionTaskNew is triggered.
     * Invokes the PluginInput dialog. If accepted sets _lastInput to the specified PluginInput starts processing.
     */
    void on_actionTaskNew_triggered();

    /*!
     * \brief If invoked for the first time terminates all plugins softly (i.e. with a 5000 ms timeout). Otherwise terminates all plugins
     * hard (i.e. without timeout).
     */
    void on_actionTaskTerminateAllPlugins_triggered();

    /*!
     * \brief Increments _scaleDelta by _scaleDelta.
     */
    void on_actionViewZoomIn_triggered();

    /*!
     * \brief Decrements _scaleDelta by _scaleDelta.
     */
    void on_actionViewZoomOut_triggered();

    /*!
     * \brief Sets _scaleDelta to _defaultScale.
     */
    void on_actionViewResetZoom_triggered();

    /*!
     * \brief Invokes the default Qt information dialog by calling QMessagebox::aboutQt().
     */
    void on_actionAboutAboutQt_triggered();

    /*!
     * \brief Invokes the save dialog. If accepted save PluginOutput(s) according to user specifications.
     */
    void on_actionTaskSaveOutput_triggered();

    /*!
     * \brief Starts processing of _lastInput.
     */
    void on_actionTaskRepeatLast_triggered();

    /*!
     * \brief Displays licensing information.
     */
    void on_actionAboutAbout_triggered();

private:
    /*!
     * \brief Used to displaying PluginOutputs.
     */
    struct PluginOutputItem
    {
        /*!
         * \brief List widget item.
         */
        QListWidgetItem *listWidgetItem;

        /*!
         * \brief PluginOutput widget.
         */
        PluginOutputWidget *pluginOutputWidget;
    };

    /*!
     * \brief Minimum display scale. Minimum: 1.
     */
    static const int _minScale = 10;

    /*!
     * \brief Maximum display scale.
     */
    static const int _maxScale = 150;

    /*!
     * \brief Amount by which display scale is increased/decreased.
     */
    static const int _scaleDelta = 5;

    /*!
     * \brief Default display scale.
     */
    static const int _defaultScale = 50;

    /*!
     * \brief Pointer to UI container class instance.
     */
    Ui::MainWindow *_ui;

    /*!
     * \brief Reference to Bakery instance.
     */
    Bakery _bakery;

    /*!
     * \brief true if a task is currently processed.
     */
    bool _working;

    /*!
     * \brief true if soft termination was requested.
     */
    bool _terminatingSoft;

    /*!
     * \brief true if hard termination was requested.
     */
    bool _terminatingHard;

    /*!
     * \brief Display scale.
     */
    int _scale;

    /*!
     * \brief Last used PluginInput.
     */
    PluginInput _lastInput;

    /*!
     * \brief Used to store best PluginOutput. Is updated after task has been processed.
     */
    PluginOutput _bestOutput;

    /*!
     * \brief Available Shape colors. Randomly generated before task processing starts.
     */
    QHash<QString, QColor> _shapeColors;

    /*!
     * \brief Hash of all valid PluginOutputs.
     */
    QHash<QString, PluginOutput> _validOutputs;

    /*!
     * \brief Used to store widgets.
     */
    QHash<QString, PluginOutputItem> _pluginOutputItems;

    /*!
     * \brief Appends a message to the log. The current date and time are prefixed.
     * \param message Message to be appended to the log.
     */
    void log(QString message);

    /*!
     * \brief Sets the scale of all PluginOutputWidgets.
     * \param scale Value from _minScale to _maxScale. If out of range a warning is issued and _scale is not updated.
     */
    void setScale(int scale);

    /*!
     * \brief Starts processing of given PluginInput.
     * Resets interface state, defines random shape colors and invokes _bakery.getAllOutputs().
     * \param input PluginInput to be processed.
     */
    void startTask(PluginInput input);
};

#endif // MAINWINDOW_H
