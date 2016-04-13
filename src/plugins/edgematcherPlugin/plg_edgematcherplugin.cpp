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

#include "helpers.hpp"
#include "plg_edgematcherplugin.h"

#include "math.h"
#include <algorithm>

#include <QSet>

Shape EdgeMatcherPlugin::matchEdge(Sheet currentSheet, Shape shapeToMatch, bool &foundMatch)
{
    foundMatch = false;
    Shape bestMatch = Shape();
    qreal bestArea = -1;

    foreach (Shape sheetShape, currentSheet.shapes())
    {
        for (qint32 i_sheetShape = 0; i_sheetShape < sheetShape.size(); ++i_sheetShape)
        {
            for (qint32 i_shapeToMatch = 0; i_shapeToMatch < shapeToMatch.size(); ++i_shapeToMatch)
            {

                Shape transformedShape;
                QLine sheetShapeEdge;
                QLine shapeToMatchEdge;

                if (i_sheetShape == sheetShape.size() - 1)
                {
                    // Last point
                    sheetShapeEdge = QLine(sheetShape[i_sheetShape], sheetShape[0]);
                }
                else
                {
                    sheetShapeEdge = QLine(sheetShape[i_sheetShape], sheetShape[i_sheetShape + 1]);
                }

                if (i_shapeToMatch == shapeToMatch.size() - 1)
                {
                    // Last point
                    shapeToMatchEdge = QLine(shapeToMatch[i_shapeToMatch], shapeToMatch[0]);
                }
                else
                {
                    shapeToMatchEdge = QLine(shapeToMatch[i_shapeToMatch], shapeToMatch[i_shapeToMatch + 1]);
                }

                if (sheetShapeEdge.p1() == sheetShapeEdge.p2() || shapeToMatchEdge.p1() == shapeToMatchEdge.p2())
                {
                    continue;
                }

                qreal angle = BakeryHelpers::linesAngle(sheetShapeEdge, shapeToMatchEdge);

                for (qint32 i_directions = 0; i_directions < 2; ++i_directions)
                {
                    qreal newAngle;
                    switch (i_directions)
                    {
                    case 0:
                        // Rotate for angle one way
                        newAngle = angle;
                        break;
                    case 1:
                        // Rotate for angle the other way  round
                        newAngle = M_PI + angle;
                        break;
                    default:
                        BAKERY_CRITICAL("Unknown direction:" << i_directions);
                        newAngle = angle;
                    }

                    for (qint32 i_matchingPoint = 0; i_matchingPoint < 5; ++i_matchingPoint)
                    {
                        transformedShape = shapeToMatch.rotated(shapeToMatch.centroid(), newAngle);
                        if (i_shapeToMatch == shapeToMatch.size() - 1)
                        {
                            // Last point
                            shapeToMatchEdge = QLine(transformedShape[i_shapeToMatch], transformedShape[0]);
                        }
                        else
                        {
                            shapeToMatchEdge = QLine(transformedShape[i_shapeToMatch], transformedShape[i_shapeToMatch + 1]);
                        }
                        QPoint shapeToMatchEdgeCenter(shapeToMatchEdge.p1() + (shapeToMatchEdge.p2() - shapeToMatchEdge.p1()) / 2);
                        QPoint sheetShapeEdgeCenter(sheetShapeEdge.p1() + (sheetShapeEdge.p2() - sheetShapeEdge.p1()) / 2);

                        switch (i_matchingPoint)
                        {
                        case 0:
                            // center
                            transformedShape.moveTo(sheetShapeEdgeCenter
                                                    - (shapeToMatchEdgeCenter - transformedShape.boundingRect().topLeft()));
                            break;
                        case 1:
                            // p1 - p1
                            transformedShape.moveTo(sheetShapeEdge.p1()
                                                    - (shapeToMatchEdge.p1() - transformedShape.boundingRect().topLeft()));
                            break;
                        case 2:
                            // p1 - p2
                            transformedShape.moveTo(sheetShapeEdge.p2()
                                                    - (shapeToMatchEdge.p1() - transformedShape.boundingRect().topLeft()));
                            break;
                        case 3:
                            // p2 - p1
                            transformedShape.moveTo(sheetShapeEdge.p1()
                                                    - (shapeToMatchEdge.p2() - transformedShape.boundingRect().topLeft()));
                            break;
                        case 4:
                            // p2 - p2
                            transformedShape.moveTo(sheetShapeEdge.p2()
                                                    - (shapeToMatchEdge.p2() - transformedShape.boundingRect().topLeft()));
                            break;
                        default:
                            BAKERY_CRITICAL("Unknown matching point:" << i_matchingPoint);
                        }

                        if (currentSheet.mayPlace(transformedShape))
                        {
                            Sheet tempSheet = currentSheet;
                            tempSheet.append(transformedShape);
                            QRect boundingRect = transformedShape.united(sheetShape).boundingRect();
                            QRect boundingRectSheet = tempSheet.shapesBoundingRect();
                            qreal area = (qreal)boundingRectSheet.width() * (qreal)boundingRectSheet.height()
                                         + (qreal)boundingRect.width() * (qreal)boundingRect.height();
                            if (bestArea == -1 || bestArea > area)
                            {
                                bestMatch = transformedShape;
                                bestArea = area;
                                foundMatch = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return bestMatch;
}

bool EdgeMatcherPlugin::placeFirstShape(Sheet &sheet, Shape &shape)
{
    shape.normalize();
    qreal rotation = 0;
    while (rotation <= 2 * M_PI)
    {
        if (sheet.mayPlace(shape))
        {
            sheet.append(shape);
            return true;
        }
        rotation += M_PI / 8;
        shape.invert();
        shape.rotate(shape.boundingRect().center(), rotation);
        shape.normalize();
    }
    return false;
}

EdgeMatcherPlugin::EdgeMatcherPlugin(QObject *parent) : QObject(parent), _terminated(false) {}

void EdgeMatcherPlugin::giveMetadata()
{
    PluginMetadata meta;
    meta.uniqueName = "edgematcher";
    meta.type = "greedy";
    meta.author = "Philipp Naumann / Marcus Soll";
    meta.license = "LGPL3+";
    emit metadataGiven(meta);
}

void EdgeMatcherPlugin::bakeSheets(PluginInput input)
{
    PluginOutput output;
    Sheet currentSheet(input.sheetWidth, input.sheetHeight);
    qint32 i = 0;
    Shape newShape;

    // Sort shapes
    std::sort(input.shapes.begin(), input.shapes.end(), Shape::lessThanByAreaDesc);
    QSet<QString> nameSet;

    newShape = Shape(input.shapes[0]);
    if (!placeFirstShape(currentSheet, newShape))
    {
        emit finished(output);
        return;
    }
    input.shapes.removeAt(0);
    output.sheets << currentSheet;
    emit outputUpdated(output);

    while (!input.shapes.isEmpty())
    {
        if (i >= input.shapes.size())
        {
            // Restart from the beginning on new sheet
            currentSheet = Sheet(input.sheetWidth, input.sheetHeight);

            newShape = Shape(input.shapes[0]);
            if (!placeFirstShape(currentSheet, newShape))
            {
                emit finished(output);
                return;
            }
            input.shapes.removeAt(0);

            output.sheets << currentSheet;
            emit outputUpdated(output);

            i = 0;
            nameSet.clear();

            if (input.shapes.isEmpty())
            {
                emit finished(output);
                return;
            }
        }

        Shape match;
        bool foundMatch = false;

        match = matchEdge(output.sheets.last(), input.shapes[i], foundMatch);

        if (foundMatch)
        {
            output.sheets.last() << match;
            input.shapes.removeAt(i);
            emit outputUpdated(output);
        }
        else
        {
            // Skip identical Shapes
            nameSet << input.shapes[i].name();
            do
            {
                ++i;
            } while (i < input.shapes.length() && nameSet.contains(input.shapes[i].name()));
        }

        QCoreApplication::processEvents();
        if (_terminated)
        {
            emit finished(output);
            return;
        }
    }

    emit finished(output);
}

void EdgeMatcherPlugin::terminate(int msecs) { QTimer::singleShot(msecs, this, SLOT(terminated(int))); }

void EdgeMatcherPlugin::terminated() { _terminated = true; }

BAKERY_PLUGIN_MAIN(EdgeMatcherPlugin)
