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

#ifndef EDGEMATCHERPLUGIN_H
#define EDGEMATCHERPLUGIN_H

#include <plugins.h>

#include <QObject>
#include <QTimer>

/*!
 * \brief EdgeMatcher plugin.
 */
class EdgeMatcherPlugin : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Tries to match a given Shape on the sheet by matching edges.
     * \param currentSheet Sheet on which to match Shape.
     * \param shapeToMatch Target Shape.
     * \param foundMatch Set to true if match is found.
     * \return If true, transformed Shape to be placed on Sheet.
     */
    static Shape matchEdge(Sheet currentSheet, Shape shapeToMatch, bool &foundMatch);

    /*!
     * \brief Tries to place a single Shape on an empty Sheet.
     * \param sheet Empty Sheet.
     * \param shape Shape to place.
     * \return True if successful.
     */
    static bool placeFirstShape(Sheet &sheet, Shape &shape);

    /*!
     * \brief Constructor.
     * \param parent QObject parent.
     */
    explicit EdgeMatcherPlugin(QObject *parent = 0);

private:
    /*!
     * \brief Set to true if EdgeMatcher should terminate.
     */
    bool _terminated;

signals:
    /*!
     * \brief Is emitted when giveMetadata() is called according to specification.
     * \param meta Metadata.
     * \sa PluginWrapper::giveMetadata()
     */
    void metadataGiven(PluginMetadata meta);

    /*!
     * \brief Is emitted when this plugin's output changes according to specification.
     * \param output Current output.
     * \sa PluginWrapper::outputUpdated(PluginOutput)
     */
    void outputUpdated(PluginOutput output);

    /*!
     * \brief Is emitted when this plugin has finished working according to specification.
     * \param output Final output.
     * \sa PluginWrapper::finished(PluginOutput)
     */
    void finished(PluginOutput output);

private slots:
    /*!
     * \brief Used by terminate().
     */
    void terminated();

public slots:
    /*!
     * \brief Emits metadataGiven().
     */
    void giveMetadata();

    /*!
     * \brief Commences EdgeMatcher passes.
     * \param input Input to be used.
     */
    void bakeSheets(PluginInput input);

    /*!
     * \brief Sets _terminated to true with a delay of msec milliseconds.
     * \param msec Milliseconds.
     */
    void terminate(int msec);
};

#endif // EDGEMATCHERPLUGIN_H
