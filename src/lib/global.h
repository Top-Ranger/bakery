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

#ifndef BAKERY_GLOBAL_H
#define BAKERY_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QDebug>

#if defined(BAKERY_LIBRARY)
#define BAKERYSHARED_EXPORT Q_DECL_EXPORT
#else
#define BAKERYSHARED_EXPORT Q_DECL_IMPORT
#endif

/*!
 * \mainpage %Bakery documentation
 *
 * %Bakery provides as framework for solving a subset of the packaging problem:
 * - Containers are rectangular and all have the same finite area. Their number is unlimited.
 * - Multi-polygons are not allowed.
 * - Arbitrary rotation and translation are the only allowed congruence mappings.
 *
 * The components of %Bakery are:
 * - libbakery: Core library
 * - bakery-gui: Graphical frontend
 * - bakery-cmd: Command-line interface
 *
 * Integers are used for representing numeric values, especially polygon coordinates.
 * Floating point number are converted by multiplying them with the constant BAKERY_PRECISION.
 *
 * The processing of tasks is plugin-based. A plugin is an executable which is able to
 * communicate with the library by implementing a given protocol. Thus plugins may be
 * written in any programming language.
 */

// Precision
#define BAKERY_PRECISION 1.0E+5 // Allow precise representation of 5 decimal places

// Debugging macros
#define BAKERY_DEBUG(x) qDebug() << "DEBUG in" << Q_FUNC_INFO << ":" << x
#define BAKERY_WARNING(x) qWarning() << "WARNING in" << Q_FUNC_INFO << ":" << x
#define BAKERY_CRITICAL(x) qCritical() << "CRITICAL in" << Q_FUNC_INFO << ":" << x
#define BAKERY_FATAL(x) qFatal("%s", QString("FATAL in %1: %2").arg(Q_FUNC_INFO).arg(x).toLatin1().data())

#endif // BAKERY_GLOBAL_H
