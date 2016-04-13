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

#ifndef TREESECTORPLUGIN_H
#define TREESECTORPLUGIN_H

#include <plugins.h>

#include <QObject>
#include <QTimer>
#include <random>

/*!
 * \brief ShapeShaker plugin.
 */
class ShapeShakerPlugin : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Returns a random real value in [min, max].
     *
     * The value is pseudo-random. You should seed the random number generator with qsrand() before calling the method the first time.
     *
     * \param min Minimum of returned value.
     * \param max Maximum of returned value.
     * \return Random value.
     */
    static qreal getRandomValue(qreal min, qreal max);

    /*!
     * \brief Tries to placed a Shape on the Sheet.
     *
     * The method tries the following and accepts the first valid placements:
     *  - Place a Shape on 1000 random locations on the Sheet.
     *  - Place a Shape in the top left corner.
     *
     * \param sheet Sheet on which Shape should be placed.
     * \param shape Shape to place.
     * \return True if successful.
     */
    static bool placeShape(Sheet &sheet, Shape &shape);

    /*!
     * \brief Shakes the Sheet by moving all Shapes in random directions.
     *
     * The shaking takes place in rounds. The amount of allowed movement and rotation is decreased in each round.
     *
     * \param rounds Number of shaking rounds.
     * \param sheet Sheet to shake.
     */
    static void shake(qint32 rounds, Sheet &sheet);

    /*!
     * \brief Applies gravity to Sheet
     *
     * This is done by moving all Shapes to the x=0 side. This takes place in rounds. The amount of allowed movement and rotation is
     * decreased in each round.
     *
     * \param rounds Number of rounds.
     * \param sheet Sheet on which to apply gravity.
     */
    static void applyGravity(qint32 rounds, Sheet &sheet);

    /*!
     * \brief Constructor.
     * \param parent QObject parent.
     */
    explicit ShapeShakerPlugin(QObject *parent = 0);

    /*!
     * \brief Deconstructor.
     */
    ~ShapeShakerPlugin();

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
     * \brief Commences ShapeShaker passes.
     * \param input Input to be used.
     */
    void bakeSheets(PluginInput input);

    /*!
     * \brief Sets _terminated to true with a delay of msec milliseconds.
     * \param msec Milliseconds.
     */
    void terminate(int msec);
};

#endif // TREESECTORPLUGIN_H
