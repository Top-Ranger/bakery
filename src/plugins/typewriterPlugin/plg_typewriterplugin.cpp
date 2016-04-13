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

#include "plg_typewriterplugin.h"

#include "../lib/helpers.hpp"

#include <QTimer>

TypewriterPlugin::TypewriterPlugin(QObject *parent) : QObject(parent), _terminated(false) {}

qint32 TypewriterPlugin::computeResolution(const QList<Shape> &shapes, const QList<qreal> &angles)
{
    QSet<qint32> distances;
    for (QList<Shape>::ConstIterator i_shape = shapes.begin(); i_shape != shapes.end(); ++i_shape)
    {
        QPoint anchor = i_shape->boundingRect().center();
        for (QList<qreal>::ConstIterator i_angle = angles.begin(); i_angle != angles.end(); ++i_angle)
        {
            Shape rotated = i_shape->rotated(anchor, *i_angle);
            distances.unite(rotated.innerDistances());
        }
    }

    QSet<qint32> harmonizedDistances;
    qint32 modulus = BAKERY_PRECISION / 10;
    for (QSet<qint32>::ConstIterator i_distance = distances.begin(); i_distance != distances.end(); ++i_distance)
    {
        harmonizedDistances << *i_distance - *i_distance % modulus;
    }
    return BakeryHelpers::binaryGCD(QList<qint32>::fromSet(harmonizedDistances));
}

QList<qreal> TypewriterPlugin::computeAngles(qint32 sheetWidth, qint32 sheetHeight, const QList<Shape> &shapes)
{
    QList<QLine> edges;

    edges << QLine(0, 0, sheetWidth, 0);
    edges << QLine(sheetWidth, 0, sheetWidth, sheetHeight);
    edges << QLine(sheetWidth, sheetHeight, 0, sheetHeight);
    edges << QLine(0, sheetHeight, 0, 0);

    for (QList<Shape>::ConstIterator i_shape = shapes.begin(); i_shape != shapes.end(); ++i_shape)
    {
        edges << i_shape->edges();
    }

    QSet<qreal> angles;
    for (QList<QLine>::Iterator i_edge = edges.begin(); i_edge != edges.end(); ++i_edge)
    {
        QLineF edge(*i_edge);
        for (QList<QLine>::Iterator i_edgeInner = i_edge; i_edgeInner != edges.end(); ++i_edgeInner)
        {
            qreal angle = qDegreesToRadians(edge.angleTo(QLineF(*i_edgeInner)));
            angle -= fmod(angle, M_PI / 64);
            angles << angle;
        }
    }
    angles << 0 << M_PI;
    QList<qreal> anglesList = QList<qreal>::fromSet(angles);
    qSort(anglesList);
    return anglesList;
}

qreal TypewriterPlugin::constantMetric(const Sheet &sheet)
{
    Q_UNUSED(sheet);
    return 1.0;
}

qreal TypewriterPlugin::convexHullUtilitization(const Sheet &sheet)
{
    qreal shapesArea = (qreal)sheet.shapesArea();
    Shape hull = sheet.shapesHull();
    qreal hullArea = (qreal)hull.area();
    QRect hullBounds = hull.boundingRect();
    qreal hullBoundsArea = (qreal)hullBounds.width() * hullBounds.height() / BAKERY_PRECISION;
    return shapesArea / hullArea / hullBoundsArea;
}

void TypewriterPlugin::typewrite(PluginInput input, qreal (*metric)(const Sheet &), int maximumSuperiors)
{
    if (_terminated)
    {
        return;
    }

    PluginOutput output;

    // Unique
    Shape::Unique unique = Shape::reduceToUnique(input.shapes);

    // Sort and normalize
    std::sort(input.shapes.begin(), input.shapes.end(), Shape::lessThanByAreaDesc);
    for (QList<Shape>::Iterator i_shape = input.shapes.begin(); i_shape != input.shapes.end(); ++i_shape)
    {
        i_shape->normalize();
    }

    // Angles and resolution
    QList<qreal> angles = computeAngles(input.sheetWidth, input.sheetHeight, unique.shapes);
    qint32 resolution = computeResolution(unique.shapes, angles);

    QList<Shape> failed;
    QList<QString> failedNames;
    output.sheets << Sheet(input.sheetWidth, input.sheetHeight);
    while (!input.shapes.isEmpty() && !_terminated)
    {
        Shape shape = input.shapes.takeFirst();
        QPoint anchor = shape.boundingRect().center();
        Sheet bestSheet;
        qreal highestSheetScore = -1;
        qint32 superiors = maximumSuperiors;
        if (!failedNames.contains(shape.name()))
        {
            for (QList<qreal>::Iterator i_angle = angles.begin(); i_angle != angles.end() && superiors > 0 && !_terminated; i_angle++)
            {
                Shape rotatedShape = shape.rotated(anchor, *i_angle);
                for (qint32 y = 0; y < input.sheetHeight && superiors > 0 && !_terminated; y += resolution)
                {
                    for (qint32 x = 0; x < input.sheetWidth && superiors > 0 && !_terminated; x += resolution)
                    {
                        Sheet candidateSheet(output.sheets.last());
                        rotatedShape.moveTo(x, y);
                        if (candidateSheet.mayPlace(rotatedShape))
                        {
                            candidateSheet << rotatedShape;
                            qreal candidateSheetScore = (metric)(candidateSheet);
                            if (highestSheetScore == -1 || candidateSheetScore > highestSheetScore)
                            {
                                bestSheet = candidateSheet;
                                highestSheetScore = candidateSheetScore;
                                --superiors;
                            }
                        }
                        QCoreApplication::processEvents();
                    }
                }
            }
        }
        if (highestSheetScore == -1)
        {
            failed << shape;
            failedNames << shape.name();
        }
        else
        {
            output.sheets.replace(output.sheets.size() - 1, bestSheet);
            emit outputUpdated(output);
        }
        if (output.sheets.last().isEmpty())
        {
            output.sheets.removeLast();
            break;
        }
        if (input.shapes.isEmpty() && !failed.isEmpty())
        {
            output.sheets << Sheet(input.sheetWidth, input.sheetHeight);
            emit outputUpdated(output);

            input.shapes << failed;
            failed.clear();
            failedNames.clear();
        }
    }

    if (!_terminated)
    {
        _outputs << output;
    }
}

void TypewriterPlugin::giveMetadata()
{
    emit metadataGiven(PluginMetadata("typewriter", "greedy", "Philipp Naumann / Marcus Soll", "LGPL3+"));
}

void TypewriterPlugin::bakeSheets(PluginInput input)
{
    typewrite(input, TypewriterPlugin::convexHullUtilitization, 50);
    typewrite(input, TypewriterPlugin::constantMetric, 1);

    if (!_outputs.isEmpty())
    {
        std::sort(_outputs.begin(), _outputs.end());
        emit finished(_outputs.last());
    }
    else
    {
        emit finished(PluginOutput());
    }
}

void TypewriterPlugin::terminate(int msecs) { QTimer::singleShot(msecs / 2, this, SLOT(terminated())); }

void TypewriterPlugin::terminated() { _terminated = true; }

BAKERY_PLUGIN_MAIN(TypewriterPlugin)
