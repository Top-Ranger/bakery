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

#ifndef SHAPEDIALOG_H
#define SHAPEDIALOG_H

#include "../lib/shape.h"

#include "plugininputdialog.h"
#include "shapewidget.h"

#include <QDialog>

namespace Ui
{
/*!
 * \brief Shape editor. Operates on ShapesListItems.
 * \sa ShapesListItem
 */
class ShapeDialog;
}

/*!
 * \brief Shape editor. Operates on ShapesListItems.
 * \sa ShapesListItem
 */
class ShapeDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     * \param parent QWidget parent.
     */
    explicit ShapeDialog(QWidget *parent = 0);

    /*!
     * \brief Destructor.
     */
    ~ShapeDialog();

    /*!
     * \brief Setter for member _item.
     * \param item ShapesListItem.
     */
    void setItem(ShapesListItem *item);

    /*!
     * \brief Setter for member _usedShapeNames.
     * \param usedShapeNames Used Shape names.
     */
    void setUsedShapeNames(const QList<QString> &usedShapeNames);

    /*!
     * \brief Validates user input and updates _item with successfully validates values.
     */
    void validate();

private slots:
    /*!
     * \brief Updates the Shape editor's resolution.
     * \param index ComboBox item index.
     */
    void resolutionChanged(int index);

    /*!
     * \brief Updates actions.
     */
    void updateControls();

    /*!
     * \brief Updates the Shape state display.
     */
    void shapeWidgetStateChanged();

    /*!
     * \brief Invokes the system's file chooser dialog. If accepted loads the given Shape file.
     */
    void on_btnLoad_clicked();

    /*!
     * \brief Invokes the system's file chooser dialog. If accepted save the edited Shape to the given file.
     */
    void on_btnSave_clicked();

    /*!
     * \brief Normalizes the Shape.
     */
    void on_btnNormalize_clicked();

    /*!
     * \brief Ensures that the Shape is closed and accepts the dialog.
     */
    void on_btnOK_clicked();

private:
    /*!
     * \brief Pointer to Ui container class instance.
     */
    Ui::ShapeDialog *_ui;

    /*!
     * \brief true if user input is valid.
     */
    bool _valid;

    /*!
     * \brief Item to be edited.
     */
    ShapesListItem *_item;

    /*!
     * \brief Stores used Shape names. Used to ensure that Shape names are unique.
     */
    QList<QString> _usedShapeNames;

    /*!
     * \brief Sets the dialog's validity state and status message.
     * \param valid Validity state.
     * \param statusMessage Status message.
     */
    void setValid(bool valid, QString statusMessage = QString());

    /*!
     * \brief Sets the dialog's status message.
     * \param status Status message.
     */
    void setStatus(QString status);
};

#endif // SHAPEDIALOG_H
