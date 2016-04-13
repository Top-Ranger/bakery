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

#ifndef TYPEWRITERPLUGIN_H
#define TYPEWRITERPLUGIN_H

#include "../lib/plugins.h"

#include <QObject>

/*!
 * \brief Typewriter plugin.
 */
class TypewriterPlugin : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     * \param parent QObject parent.
     */
    explicit TypewriterPlugin(QObject *parent = 0);

private:
    /*!
     * \brief PluginOutput candidates stored after each typewriting pass that is not terminated.
     */
    QList<PluginOutput> _outputs;

    /*!
     * \brief Is set when the "terminate" command is received after a delay equal to half the given time.
     */
    bool _terminated;

    /*!
     * \brief Computes the resolution.
     * \param shapes Shapes to be considered.
     * \param angles Angles to be considered.
     * \return
     */
    qint32 computeResolution(const QList<Shape> &shapes, const QList<qreal> &angles);

    /*!
     * \brief Computes rotation angles to be used.
     * \param sheetWidth Sheet width. Used to compute Sheet edges.
     * \param sheetHeight Sheet height. Used to compute Sheet edges.
     * \param shapes Shapes to be considered.
     * \return
     */
    QList<qreal> computeAngles(qint32 sheetWidth, qint32 sheetHeight, const QList<Shape> &shapes);

    /*!
     * \brief Returns 1.0.
     * \param sheet Ignored.
     * \return
     */
    static qreal constantMetric(const Sheet &sheet);

    /*!
     * \brief Returns the quotient of the sum of the areas of all Shapes and the product of the area of their convex hull and the hull's
     * bounding rectangle's area.
     * \param sheet Sheet to take Shapes from.
     * \return Utilitization of all Shapes' convex hull (bounds).
     */
    static qreal convexHullUtilitization(const Sheet &sheet);

    /*!
     * \brief Typewriting pass using the given metric and the maximum number of superior solutions.
     * \param input PluginInput.
     * \param metric Metric to be used.
     * \param maximumSuperiors Maximum number of superiors.
     */
    void typewrite(PluginInput input, qreal (*metric)(const Sheet &sheet), int maximumSuperiors);

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

public slots:
    /*!
     * \brief Emits metadataGiven().
     */
    void giveMetadata();

    /*!
     * \brief Commences typewriting passes.
     * \param input Input to be used.
     */
    void bakeSheets(PluginInput input);

    /*!
     * \brief Sets _terminated to true with a delay of (msecs / 2) milliseconds.
     * \param msecs Milliseconds.
     */
    void terminate(int msecs);

private slots:
    /*!
     * \brief Used by terminate().
     */
    void terminated();
};

#endif // TYPEWRITERPLUGIN_H
