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

#include "../lib/helpers.hpp"
#include "../lib/bakery.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plugininputdialog.h"
#include "savedialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <random>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      _ui(new Ui::MainWindow),
      _working(false),
      _terminatingSoft(false),
      _terminatingHard(false),
      _lastInput(PluginInput())
{
    _ui->setupUi(this);

    // Bakery signals
    connect(&_bakery, SIGNAL(pluginStarting(QString)), this, SLOT(_pluginStarting(QString)));
    connect(&_bakery, SIGNAL(pluginTerminating(QString, int)), this, SLOT(_pluginTerminating(QString, int)));
    connect(&_bakery, SIGNAL(pluginOutputUpdated(QString, PluginOutput)), this, SLOT(_pluginOutputUpdated(QString, PluginOutput)));
    connect(&_bakery, SIGNAL(pluginFinished(int, QString, PluginOutput, bool)), this,
            SLOT(_pluginFinished(int, QString, PluginOutput, bool)));
    connect(&_bakery, SIGNAL(allPluginsFinished(QHash<QString, PluginOutput>)), this,
            SLOT(_allPluginsFinished(QHash<QString, PluginOutput>)));

    // Default sheet dimensions
    _lastInput.sheetWidth = BakeryHelpers::qrealPrecise(3);
    _lastInput.sheetHeight = BakeryHelpers::qrealPrecise(3);

    log(tr("Welcome to Bakery!"));
    setScale(_defaultScale);
}

MainWindow::~MainWindow()
{
    _bakery.terminateAllPlugins(0);
    while (_working)
    {
        QApplication::processEvents();
        QThread::usleep(100);
    }
    delete _ui;
}

void MainWindow::_pluginStarting(QString pluginName)
{
    PluginOutputItem item;
    item.listWidgetItem = new QListWidgetItem();
    item.pluginOutputWidget = new PluginOutputWidget(this, pluginName, _shapeColors);
    item.pluginOutputWidget->setState(PluginOutputWidget::Working);
    _pluginOutputItems.insert(pluginName, item);
    _ui->outputsListWidget->addItem(item.listWidgetItem);
    _ui->outputsListWidget->setItemWidget(item.listWidgetItem, item.pluginOutputWidget);
    item.listWidgetItem->setSizeHint(item.pluginOutputWidget->sizeHint());

    log(tr("Plugin '%1' started.").arg(pluginName));
}

void MainWindow::_pluginTerminating(QString pluginName, int msec)
{
    Q_UNUSED(msec);

    PluginOutputItem item = _pluginOutputItems.value(pluginName);
    item.pluginOutputWidget->setState(PluginOutputWidget::Terminating);
    item.listWidgetItem->setSizeHint(item.pluginOutputWidget->sizeHint());
}

void MainWindow::_pluginOutputUpdated(QString pluginName, PluginOutput output)
{
    PluginOutputItem item = _pluginOutputItems.value(pluginName);
    item.pluginOutputWidget->setOutput(output);
    item.listWidgetItem->setSizeHint(item.pluginOutputWidget->sizeHint());
}
void MainWindow::_pluginFinished(int exitCode, QString pluginName, PluginOutput output, bool valid)
{
    PluginOutputItem item = _pluginOutputItems.value(pluginName);
    item.pluginOutputWidget->setState(valid ? PluginOutputWidget::Valid : PluginOutputWidget::Invalid);
    item.pluginOutputWidget->setOutput(output);
    item.listWidgetItem->setSizeHint(item.pluginOutputWidget->sizeHint());
    log(tr("Plugin '%1' finished (exit code: %2).").arg(pluginName).arg(exitCode));
}

void MainWindow::_allPluginsFinished(QHash<QString, PluginOutput> pluginOutputs)
{
    _validOutputs = pluginOutputs;
    _working = false;
    if (_validOutputs.isEmpty())
    {
        log(tr("All plugins finished. No plugin found a valid solution."));
    }
    else
    {
        _bestOutput = Bakery::findBestOutput(_validOutputs);
        QStringList valid(_validOutputs.keys());
        log(tr("All plugins finished. Valid solutions found by: %1.").arg(valid.join(", ")));
    }
    updateActions();
}

void MainWindow::updateActions()
{
    bool valid = !_validOutputs.isEmpty();
    _ui->actionFileQuit->setEnabled(!_working);
    _ui->actionTaskNew->setEnabled(!_working);
    _ui->actionTaskRepeatLast->setEnabled(!_working && !_lastInput.shapes.empty());
    _ui->actionTaskSaveOutput->setEnabled(!_working && valid);
    _ui->actionTaskTerminateAllPlugins->setEnabled(_working && !_terminatingHard);
    _ui->actionViewZoomIn->setEnabled(_scale <= _maxScale - _scaleDelta);
    _ui->actionViewZoomOut->setEnabled(_scale >= _minScale + _scaleDelta);
    _ui->actionViewResetZoom->setEnabled(!_working);
}

void MainWindow::log(QString message)
{
    _ui->listWidgetLog->addItem(QString("%1: %2").arg(QDateTime::currentDateTime().toString()).arg(message));
    _ui->listWidgetLog->scrollToBottom();
}

void MainWindow::setScale(int scale)
{
    if (scale < _minScale || scale > _maxScale)
    {
        BAKERY_WARNING("Scale out of range");
        return;
    }

    _scale = scale;
    foreach (PluginOutputItem item, _pluginOutputItems)
    {
        item.pluginOutputWidget->setScale(_scale);
        item.listWidgetItem->setSizeHint(item.pluginOutputWidget->sizeHint());
    }
    updateActions();
}

void MainWindow::startTask(PluginInput input)
{
    // Reset
    _terminatingSoft = false;
    _terminatingHard = false;
    _working = true;
    _shapeColors.clear();

    std::random_device rnd;
    std::uniform_int_distribution<qint32> distribution(0, 255);

    Shape::Unique unique = Shape::reduceToUnique(input.shapes);
    foreach (QString name, unique.names)
    {
        _shapeColors[name] = QColor(distribution(rnd), distribution(rnd), distribution(rnd));
    }
    _pluginOutputItems.clear();
    _ui->outputsListWidget->clear();
    setScale(_defaultScale);

    _bakery.computeAllOutputs(input, false);
}

void MainWindow::on_actionFileQuit_triggered() { close(); }

void MainWindow::on_actionViewZoomIn_triggered() { setScale(_scale + _scaleDelta); }

void MainWindow::on_actionViewZoomOut_triggered() { setScale(_scale - _scaleDelta); }

void MainWindow::on_actionViewResetZoom_triggered() { setScale(_defaultScale); }

void MainWindow::on_actionAboutAboutQt_triggered() { QMessageBox::aboutQt(this); }

void MainWindow::on_actionTaskNew_triggered()
{
    PluginInput input(_lastInput);
    PluginInputDialog dialog(_bakery, this);
    dialog.setInput(&input);
    if (dialog.exec())
    {
        _lastInput = input;
        startTask(input);
    }
}

void MainWindow::on_actionTaskTerminateAllPlugins_triggered()
{
    if (_terminatingSoft)
    {
        _terminatingHard = true;
        log(tr("Terminating all plugins (hard)."));
        _bakery.terminateAllPlugins(0);
    }
    else
    {
        _terminatingSoft = true;
        log(tr("Terminating all plugins (soft)."));
        _bakery.terminateAllPlugins(5000);
    }
    updateActions();
}

void MainWindow::on_actionTaskSaveOutput_triggered()
{
    SaveDialog saveDialog(this);
    SaveDialog::Options options;
    saveDialog.setOptions(&options);
    if (saveDialog.exec())
    {
        if (options.saveAll)
        {
            for (int i = 0; i < _validOutputs.size(); ++i)
            {
                QString pluginName = _validOutputs.keys()[i];
                QString outputDirectoryPath = QDir(options.outputDirectoryPath).absoluteFilePath(pluginName);
                if (!Bakery::saveToDirectory(_validOutputs.values()[i], outputDirectoryPath, options.resultsFileName, options.saveSVGs))
                {
                    log(tr("Failed to save output of plugin '%1' to '%2'.").arg(pluginName).arg(outputDirectoryPath));
                    return;
                }
            }
            log(tr("All outputs saved to \"%1\".").arg(options.outputDirectoryPath));
        }
        else
        {
            if (!Bakery::saveToDirectory(_bestOutput, options.outputDirectoryPath, options.resultsFileName, options.saveSVGs))
            {
                log(tr("Failed to save output to '%1'.").arg(options.outputDirectoryPath));
            }
            else
            {
                log(tr("Best output saved to '%1'.").arg(QDir(options.outputDirectoryPath).absoluteFilePath(options.resultsFileName)));
            }
        }
    }
}

void MainWindow::on_actionTaskRepeatLast_triggered() { startTask(_lastInput); }

void MainWindow::on_actionAboutAbout_triggered()
{
    QMessageBox::about(this, tr("About Bakery"), tr("Copyright (C) 2015,2016 Philipp Naumann \n"
                                                    "Copyright (C) 2015,2016 Marcus Soll \n"
                                                    "\n"
                                                    "Bakery is free software: you can redistribute it and/or modify "
                                                    "it under the terms of the GNU Lesser General Public License "
                                                    "as published by the Free Software Foundation, either version 3 of the License, or "
                                                    "(at your option) any later version. \n"
                                                    "\n"
                                                    "Bakery is distributed in the hope that it will be useful, "
                                                    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                                                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
                                                    "GNU Lesser General Public License for more details. \n"
                                                    "\n"
                                                    "You should have received a copy of the GNU Lesser General Public License "
                                                    "along with Bakery. If not, see <http://www.gnu.org/licenses/>."));
}
