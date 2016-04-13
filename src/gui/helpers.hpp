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

#ifndef BAKERY_GUI_HELPERS_HPP
#define BAKERY_GUI_HELPERS_HPP

#include "../lib/bakery.h"
#include "../lib/plugins.h"
#include <QFile>

namespace BakeryGUIHelpers
{
/*!
 * \brief Loads a PluginInput from a plugin input file at the given path.
 * \param path File path.
 * \param ok true if successful.
 * \return PluginInput.
 */
inline PluginInput loadFromFile(QString path, bool *ok)
{
    QFile inputFile(path);
    inputFile.open(QFile::ReadOnly);
    PluginInput input = Bakery::loadFromDevice(&inputFile, ok);
    inputFile.close();
    if (!*ok)
    {
        return PluginInput();
    }
    return input;
}

/*!
 * \brief Loads a PluginInput from a SVG input file at the given path.
 * \param path File path.
 * \param ok true if successful.
 * \return PluginInput.
 */
inline PluginInput loadFromSVG(QString path, bool *ok)
{
    QFile inputFile(path);
    inputFile.open(QFile::ReadOnly);
    PluginInput input = Bakery::loadFromSVG(&inputFile, ok);
    inputFile.close();
    if (!*ok)
    {
        return PluginInput();
    }
    return input;
}
}

#endif // BAKERY_GUI_HELPERS_HPP
