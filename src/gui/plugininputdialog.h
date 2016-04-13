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

#ifndef PLUGININPUTDIALOG_H
#define PLUGININPUTDIALOG_H

#include "../lib/bakery.h"

#include <QDialog>
#include <QAbstractListModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

namespace Ui
{
/*!
 * \brief Used to manipulate PluginInputs.
 */
class PluginInputDialog;
}

/*!
 * \brief Struct for shapes list item model.
 */
struct ShapesListItem
{
    /*!
     * \brief Shape.
     */
    Shape shape;

    /*!
     * \brief Amount. 1 by default.
     */
    int amount;

    // Default values
    ShapesListItem() { amount = 1; }
};

/*!
 * \brief Model used to store and manipulate Shapes list information.
 */
class ShapesListModel : public QAbstractListModel
{
public:
    /*!
     * \brief Returns the Shapes list model row count.
     * \param parent Parent model index (ignored).
     * \return
     */
    int rowCount(const QModelIndex &parent) const;

    /*!
     * \brief Removes the given row from the shapes list model.
     * \param row Row.
     * \param parent Parent model index.
     * \return true if successful, false otherwise.
     */
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    /*!
     * \brief Appends an item to the Shapes list model.
     * \param item Item.
     */
    void append(const ShapesListItem &item);

    /*!
     * \brief Returns the Shapes list model data at the given index.
     * \param index Index.
     * \param role Role.
     * \return Model data.
     */
    QVariant data(const QModelIndex &index, int role) const;

    /*!
     * \brief Updates the Shapes list model according the given PluginInput.
     * Resets the model. Creates and appends Shapes list items for every Shape.
     * \param input PluginInput.
     */
    void update(const PluginInput &input);

    /*!
     * \brief Returns the Shapes list model index of the Shape with the given name.
     * \param name Shape name.
     * \return The list model index if the model contains a Shape of the given name, -1 otherwise.
     */
    int indexOf(const QString &name) const;

    /*!
     * \brief Checks if the Shapes list model is empty.
     * \return true if the Shapes list model is empty, false otherwise.
     */
    bool empty() const;

    /*!
     * \brief Returns a pointer to the given row of the Shapes list model.
     * \param row Row index.
     * \return
     */
    ShapesListItem *pointer(int row);

    /*!
     * \brief Replaces the Shapes list model item at the given index.
     * \param row Row index.
     * \param item Item.
     */
    void replace(int row, const ShapesListItem &item);

    /*!
     * \brief Clears the Shapes list model and emits dataChanged().
     */
    void clear();

    /*!
     * \brief Returns a list of shapes list model shapes where each Shape's frequency is equal to its amount.
     * \return Expanded model.
     */
    QList<Shape> expand() const;

    /*!
     * \brief Returns a list of unique Shapes names according to Shapes list model.
     * \return List of unique Shapes names.
     */
    QList<QString> names() const;

private:
    /*!
     * \brief Model items.
     */
    QList<ShapesListItem> _items;
};

/*!
 * \brief Shapes list view delegate.
 */
class ShapesListViewDelegate : public QStyledItemDelegate
{
public:
    /*!
     * \brief Overrides QStyledItemDelegate::paint().
     * \param painter Pointer to active painter.
     * \param option Current option.
     * \param index Shapes list model index.
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /*!
     * \brief Overrides QStyledItemDelegate::sizeHint().
     * \param option Current option.
     * \param index Shapes list model index.
     * \return Size hint.
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

/*!
 * \brief Used to manipulate PluginInputs.
 */
class PluginInputDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief PluginInputDialog constructor.
     * \param bakery Reference to current Bakery instance.
     * \param parent Parent QWidget.
     */
    explicit PluginInputDialog(Bakery &bakery, QWidget *parent = 0);

    /*!
     * \brief PluginInputDialog destructor.
     */
    ~PluginInputDialog();

    /*!
     * \brief Sets the pointer to the PluginInput to be edited.
     * Updates Shapes list model and interface state.
     * \param input Pointer to PluginInput.
     */
    void setInput(PluginInput *input);

    /*!
     * \brief Updates the Shapes list model according to the given PluginInput and updates the interface state.
     * \param input The PluginInput.
     */
    void update(PluginInput input);

private slots:
    /*!
     * \brief Enables/disables actions according to interface state.
     */
    void updateControls();

    /*!
     * \brief Sets the check state of all plugins to checked.
     */
    void on_btnEnableAll_clicked();

    /*!
     * \brief Sets the check state of all plugins to unchecked.
     */
    void on_btnDisableAll_clicked();

    /*!
     * \brief Invokes the system's file chooser dialog. If accepted updates Shapes list model and interface state.
     */
    void on_btnLoad_clicked();

    /*!
     * \brief Invokes the system's file chooser dialog. If accepted uses BakeryGUIHelpers::loadFromSVG() to parse the selected SVG file.
     */
    void on_btnLoadSVG_clicked();

    /*!
     * \brief Invokes Bakery::randomInput to generate a random PluginInput and updates Shapes list model and interface state accordingly.
     */
    void on_btnRandom_clicked();

    /*!
     * \brief Removed the selected Shape from the Shapes list model.
     */
    void on_btnRemove_clicked();

    /*!
     * \brief Invokes the Shape editor dialog. If accepted adds the Shape to the Shapes list model.
     */
    void on_btnAdd_clicked();

    /*!
     * \brief Invokes the Shape editor dialog for the selected Shape.
     */
    void on_btnEdit_clicked();

    /*!
     * \brief Invokes the system's file chooser dialog. If accepted use Bakery::saveToDevice() to store the PluginInput.
     */
    void on_btnSave_clicked();

    /*!
     * \brief Clears the Shapes list model.
     */
    void on_btnClear_clicked();

    /*!
     * \brief Enables/disables plugins according to user specifications and sets the dialog's accepted state. The main window uses this
     * value to determine is processing needs to be started.
     */
    void on_btnStart_clicked();

    /*!
     * \brief Enables the selected plugin.
     */
    void on_btnEnable_clicked();

    /*!
     * \brief Disables the selected plugin.
     */
    void on_btnDisable_clicked();

    /*!
     * \brief Invokes the Shape editor if a Shape is selected by double-clicking the widget.
     */
    void on_listViewShapes_doubleClicked(const QModelIndex &index);

private:
    /*!
     * \brief Pointer to UI container class instance.
     */
    Ui::PluginInputDialog *_ui;

    /*!
     * \brief Reference to Bakery instance.
     */
    Bakery &_bakery;

    /*!
     * \brief true if user input is valid.
     */
    bool _valid;

    /*!
     * \brief PluginInput to be edited.
     */
    PluginInput *_input;

    /*!
     * \brief Shapes list model.
     */
    ShapesListModel _shapesListModel;

    /*!
     * \brief Shapes list view delegate.
     */
    ShapesListViewDelegate _shapesListViewDelegate;

    /*!
     * \brief Validates user input and updates interface state.
     * _input is updated with all correct values.
     */
    void validate();

    /*!
     * \brief Sets validity state.
     * \param valid true if valid, false otherwise.
     * \param statusMessage Status message.
     */
    void setValid(bool valid, QString statusMessage = QString());

    /*!
     * \brief Sets the status message.
     * \param message Status message.
     */
    void setStatus(QString message);

    /*!
     * \brief Sets selected plugin's check state.
     * \param state Checked if true, unchecked otherwise.
     */
    void setAllPluginsCheckState(Qt::CheckState state);

    /*!
     * \brief Sets selected plugin's check state.
     * \param state Checked if true, unchecked otherwise.
     */
    void setSelectedPluginsCheckState(Qt::CheckState state);

    /*!
     * \brief Invokes the Shape editor.
     * \param row Shapes list model row index.
     */
    void editShape(int row);

    /*!
     * \brief Returns a list of checked plugins' names.
     * \return List of plugins' names.
     */
    QStringList checkedPlugins() const;

    /*!
     * \brief Returns the Shapes list model row index of the selected Shape.
     * \return Index of selected Shape, -1 if no Shape is selected.
     */
    int selectedShapeRow() const;
};

Q_DECLARE_METATYPE(ShapesListItem)
Q_DECLARE_METATYPE(ShapesListItem *)

#endif // PLUGININPUTDIALOG_H
