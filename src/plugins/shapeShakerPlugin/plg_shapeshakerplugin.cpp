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
#include "plg_shapeshakerplugin.h"

#include "math.h"
#include <algorithm>

#include <QSet>

static std::mt19937 rnd;

qreal ShapeShakerPlugin::getRandomValue(qreal min, qreal max)
{
    std::uniform_real_distribution<qreal> distribution(min, max);
    return distribution(rnd);
}

bool ShapeShakerPlugin::placeShape(Sheet &sheet, Shape &shape)
{
    Shape newShape;

    // Try to place shape at a random location
    for (qint32 i = 0; i < 1000; ++i)
    {
        newShape = shape;
        newShape.rotate(newShape.centroid(), getRandomValue(0, 2 * M_PI));
        newShape.moveTo(getRandomValue(0, sheet.height()), getRandomValue(0, sheet.width()));
        if (sheet.mayPlace(newShape))
        {
            sheet.append(newShape);
            return true;
        }
    }

    // If no shape could be placed try to place one in the top left corner
    newShape = shape;
    newShape.normalize();
    qreal rotation = 0;
    while (rotation <= 2 * M_PI)
    {
        if (sheet.mayPlace(newShape))
        {
            sheet.append(newShape);
            return true;
        }
        rotation += M_PI / 8;
        newShape.invert();
        newShape.rotate(newShape.boundingRect().center(), rotation);
        newShape.normalize();
    }
    return false;
}

void ShapeShakerPlugin::shake(qint32 rounds, Sheet &sheet)
{
    qint32 offset = qMin(sheet.width(), sheet.height());
    qint32 offsetReduce = qMax(offset / rounds, 1);
    qreal angleOffset = M_PI;
    qreal angleOffsetReduce = angleOffset / rounds;
    for (qint32 i = 0; i < rounds; ++i)
    {
        for (qint32 i_shape = 0; i_shape < sheet.shapes().size(); ++i_shape)
        {
            // move and rotate every shape
            qreal angle = getRandomValue(-1 * angleOffset, angleOffset);
            qint32 moveX = getRandomValue(-1 * offset, offset);
            qint32 moveY = getRandomValue(-1 * offset, offset);
            Shape tempShape(sheet.shapes()[i_shape]);
            tempShape.rotate(tempShape.centroid(), angle);
            tempShape.moveTo(tempShape.position() + QPoint(moveX, moveY));
            Sheet tempSheet = sheet;
            tempSheet.shapes().takeAt(i_shape);
            if (tempSheet.mayPlace(tempShape))
            {
                sheet.shapes()[i_shape] = tempShape;
            }
        }
        offset = qMax(offset - offsetReduce, 1);
        angleOffset -= angleOffsetReduce;
    }
}

void ShapeShakerPlugin::applyGravity(qint32 rounds, Sheet &sheet)
{
    qint32 offset = -1 * qMin(sheet.width(), sheet.height());
    qint32 offsetReduce = qMin(offset / rounds, -1);
    qreal angleOffset = M_PI_4;
    qreal angleOffsetReduce = angleOffset / rounds;
    for (qint32 i = 0; i < rounds; ++i)
    {
        for (qint32 i_shape = 0; i_shape < sheet.shapes().size(); ++i_shape)
        {
            // move every shape
            qreal angle = getRandomValue(-1 * angleOffset, angleOffset);
            qint32 moveX = getRandomValue(offset, 0);
            qint32 moveY = getRandomValue(-10, 10);
            Shape tempShape(sheet.shapes()[i_shape]);
            tempShape.rotate(tempShape.centroid(), angle);
            tempShape.moveTo(tempShape.position() + QPoint(moveX, moveY));
            Sheet tempSheet = sheet;
            tempSheet.shapes().takeAt(i_shape);
            if (tempSheet.mayPlace(tempShape))
            {
                sheet.shapes()[i_shape] = tempShape;
            }
        }
        offset = qMin(offset - offsetReduce, -1);
        angleOffset -= angleOffsetReduce;
    }
}

ShapeShakerPlugin::ShapeShakerPlugin(QObject *parent) : QObject(parent), _terminated(false) {}

ShapeShakerPlugin::~ShapeShakerPlugin() {}

void ShapeShakerPlugin::giveMetadata()
{
    PluginMetadata meta;
    meta.uniqueName = "shapeshaker";
    meta.type = "randomized";
    meta.author = "Philipp Naumann / Marcus Soll";
    meta.license = "LGPL3+";
    emit metadataGiven(meta);
}

void ShapeShakerPlugin::bakeSheets(PluginInput input)
{
    PluginOutput output;
    Sheet currentSheet(input.sheetWidth, input.sheetHeight);

    // Initial
    bool placed;

    // Iterations
    while (!input.shapes.empty())
    {
        for (qint32 i = 0; i < 10; ++i)
        {
            do
            {
                qint32 randomShapeNumber = getRandomValue(0, input.shapes.size() - 1);
                placed = placeShape(currentSheet, input.shapes[randomShapeNumber]);
                if (placed)
                {
                    input.shapes.removeAt(randomShapeNumber);
                }
            } while (placed && !input.shapes.empty());
            if (input.shapes.empty())
            {
                break;
            }
            shake(150, currentSheet);
            applyGravity(300, currentSheet);
        }

        // Try one last time to place shapes - this time we want to try each type of shape
        QSet<QString> nonFitting;
        QList<Shape>::Iterator iterator = input.shapes.begin();
        while (iterator != input.shapes.end())
        {
            if (nonFitting.contains(iterator->name()))
            {
                ++iterator;
                continue;
            }
            if (placeShape(currentSheet, *iterator))
            {
                iterator = input.shapes.erase(iterator);
            }
            else
            {
                nonFitting << iterator->name();
                ++iterator;
            }
        }

        // Apply gravity a bit more - to increase density
        applyGravity(600, currentSheet);

        // Create new sheet
        if (currentSheet.size() == 0)
        {
            emit finished(output);
            return;
        }

        output.sheets << currentSheet;
        emit outputUpdated(output);
        currentSheet = Sheet(input.sheetWidth, input.sheetHeight);

        QCoreApplication::processEvents();
        if (_terminated)
        {
            emit finished(output);
            return;
        }
    }

    emit finished(output);
}

void ShapeShakerPlugin::terminate(int msecs) { QTimer::singleShot(msecs, this, SLOT(terminated(int))); }

void ShapeShakerPlugin::terminated() { _terminated = true; }

BAKERY_PLUGIN_MAIN(ShapeShakerPlugin)
