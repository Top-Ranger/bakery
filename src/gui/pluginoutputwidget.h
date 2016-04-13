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

#ifndef PLUGINOUTPUTWIDGET_H
#define PLUGINOUTPUTWIDGET_H

#include "../lib/plugins.h"

#include <QWidget>
#include <QtGui>

namespace Ui
{
/*!
 * \brief Used to display PluginOutputs.
 */
class PluginOutputWidget;
}

/*!
 * \brief Used to display PluginOutputs.
 */
class PluginOutputWidget : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Widget state.
     */
    enum State
    {
        Idle,
        Working,
        Terminating,
        Valid,
        Invalid
    };

    /*!
     * \brief PluginOutputWidget constructor.
     * \param parent Parent QWidget.
     * \param pluginName Name of the plugin.
     * \param shapeColors Available shape colors.
     */
    explicit PluginOutputWidget(QWidget *parent = 0, const QString pluginName = QString(),
                                const QHash<QString, QColor> &shapeColors = QHash<QString, QColor>());

    /*!
     * \brief ~PluginOutputWidget destructor.
     */
    ~PluginOutputWidget();

    /*!
     * \brief Dimensions of the paint buffer.
     * \return Dimensions of the paint buffer.
     */
    QSize sizeHint() const;

    /*!
     * \brief Setter for member _output. Updates the view.
     * \param output Output.
     */
    void setOutput(PluginOutput output);

    /*!
     * \brief Setter for member _state. Updates the view.
     * \param state State.
     */
    void setState(State state);

    /*!
     * \brief Setter for member _scale. Updates the view.
     * \param scale Scale.
     */
    void setScale(int scale);

protected:
    /*!
     * \brief Overrides QWidget::paintEvent(QPaintEvent). Transfers the contents of the paint buffer to the widget's canvas.
     * \param event Paint event.
     */
    void paintEvent(QPaintEvent *event);

private:
    /*!
     * \brief Reference to UI container class instance.
     */
    Ui::PluginOutputWidget *_ui;

    /*!
     * \brief Stores the plugin's name.
     */
    QString _pluginName;

    /*!
     * \brief Randomly generated Shape colors.
     * \sa MainWindow::startTask()
     */
    QHash<QString, QColor> _shapeColors;

    /*!
     * \brief The widget is internally double-buffered to prevent flickering.
     */
    QPixmap _buffer;

    /*!
     * \brief PluginOutput to be display. Is updated whenever the corresponding plugin signals that it has updated its output.
     */
    PluginOutput _output;

    /*!
     * \brief Display scale.
     */
    int _scale;

    /*!
     * \brief Interface state.
     */
    State _state;

    /*!
     * \brief Updates the paint buffer.
     */
    void updateView();
};

#endif // PLUGINOUTPUTWIDGET_H
