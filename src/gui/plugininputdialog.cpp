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

#include "plugininputdialog.h"
#include "ui_plugininputdialog.h"

#include "shapedialog.h"
#include "../lib/helpers.hpp"
#include "helpers.hpp"

#include <QPainter>
#include <QStatusBar>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QInputDialog>

PluginInputDialog::PluginInputDialog(Bakery &bakery, QWidget *parent)
    : QDialog(parent), _ui(new Ui::PluginInputDialog), _bakery(bakery), _input(NULL)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowContextHelpButtonHint);

    _ui->listViewShapes->setModel(&_shapesListModel);
    _ui->listViewShapes->setItemDelegate(&_shapesListViewDelegate);
    connect(_ui->listViewShapes->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this, SLOT(updateControls()));
    connect(_ui->listViewShapes->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(updateControls()));

    foreach (QString pluginName, _bakery.getAllPlugins())
    {
        QListWidgetItem *item = new QListWidgetItem();
        PluginMetadata meta = _bakery.getPluginMetadata(pluginName);
        QString toolTip = QString("Author: %1\nType: %2\nLicense: %3").arg(meta.author).arg(meta.type).arg(meta.license);
        item->setText(pluginName);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setCheckState(_bakery.isPluginEnabled(pluginName) ? Qt::Checked : Qt::Unchecked);
        item->setToolTip(toolTip);
        _ui->listWidgetPlugins->addItem(item);
    }

    updateControls();
}

PluginInputDialog::~PluginInputDialog() { delete _ui; }

void PluginInputDialog::setValid(bool valid, QString statusMessage)
{
    _valid = valid;
    setStatus(statusMessage);
}

void PluginInputDialog::setStatus(QString message)
{
    _ui->lStatus->setText(message);
    _ui->lStatus->setForegroundRole(_valid ? QPalette::Text : QPalette::Highlight);
    _ui->lStatus->setBackgroundRole(_valid ? QPalette::Text : QPalette::Highlight);
}

void PluginInputDialog::setAllPluginsCheckState(Qt::CheckState state)
{
    for (int i = 0; i < _ui->listWidgetPlugins->count(); ++i)
    {
        QListWidgetItem *item = _ui->listWidgetPlugins->item(i);
        item->setCheckState(state);
    }
    validate();
    updateControls();
}

void PluginInputDialog::setSelectedPluginsCheckState(Qt::CheckState state)
{
    foreach (QListWidgetItem *item, _ui->listWidgetPlugins->selectedItems())
    {
        item->setCheckState(state);
    }
}

void PluginInputDialog::editShape(int row)
{
    ShapesListItem item(*_shapesListModel.pointer(row));
    ShapeDialog shapeDialog(this);
    shapeDialog.setItem(&item);

    // The current name may be used again
    QList<QString> usedShapeNames(_shapesListModel.names());
    usedShapeNames.removeAll(item.shape.name());

    shapeDialog.setUsedShapeNames(usedShapeNames);
    if (shapeDialog.exec())
    {
        _shapesListModel.replace(row, item);
    }
    repaint();
}

QStringList PluginInputDialog::checkedPlugins() const
{
    QList<QString> checked;
    for (int i = 0; i < _ui->listWidgetPlugins->count(); ++i)
    {
        QListWidgetItem *item = _ui->listWidgetPlugins->item(i);
        if (item->checkState() == Qt::Checked)
        {
            checked << item->text();
        }
    }
    return checked;
}

int PluginInputDialog::selectedShapeRow() const
{
    QModelIndexList indices = _ui->listViewShapes->selectionModel()->selectedIndexes();
    return indices.empty() ? -1 : indices.first().row();
}

void PluginInputDialog::validate()
{
    bool ok;
    qreal sheetWidth = _ui->leSheetWidth->text().toDouble(&ok);
    if (!ok)
    {
        setValid(false, tr("Sheet width is not a number."));
        return;
    }
    if (!(sheetWidth > 0))
    {
        setValid(false, tr("Sheet width is not greater than 0."));
        return;
    }
    _input->sheetWidth = _ui->leSheetWidth->text().toDouble(&ok) * BAKERY_PRECISION;

    qreal sheetHeight = _ui->leSheetHeight->text().toDouble(&ok);
    if (!ok)
    {
        setValid(false, tr("Sheet height is not a number."));
        return;
    }
    if (!(sheetHeight > 0))
    {
        setValid(false, tr("Sheet height is not greater than 0."));
        return;
    }
    _input->sheetHeight = _ui->leSheetHeight->text().toDouble(&ok) * BAKERY_PRECISION;

    if (checkedPlugins().size() == 0)
    {
        setValid(false, tr("No plugins selected."));
        return;
    }

    if (_shapesListModel.empty())
    {
        setValid(false, tr("No shapes."));
        return;
    }
    _input->shapes = _shapesListModel.expand();

    setValid(true, tr("Task is valid."));
}

void PluginInputDialog::updateControls()
{
    if (_input == NULL)
    {
        return;
    }

    validate();

    _ui->btnStart->setEnabled(_valid);
    _ui->btnSave->setEnabled(_valid);
    _ui->listViewShapes->setEnabled(!_shapesListModel.empty());
    bool pluginSelected = _ui->listWidgetPlugins->selectedItems().size() > 0;
    _ui->btnEnable->setEnabled(pluginSelected);
    _ui->btnDisable->setEnabled(pluginSelected);
    bool shapeSelected = selectedShapeRow() != -1;
    _ui->btnEdit->setEnabled(shapeSelected);
    _ui->btnRemove->setEnabled(shapeSelected);
    _ui->btnClear->setEnabled(!_shapesListModel.empty());
}

int ShapesListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _items.size();
}

bool ShapesListModel::removeRow(int row, const QModelIndex &parent)
{
    if (row > _items.size() - 1)
    {
        return false;
    }
    beginRemoveRows(parent, row, row);
    _items.removeAt(row);
    endRemoveRows();
    emit dataChanged(index(0, row), index(0, row));
    return true;
}

void ShapesListModel::append(const ShapesListItem &item)
{
    _items << item;
    emit dataChanged(index(0, 0), index(0, _items.size()));
}

QVariant ShapesListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole)
    {
        return QVariant::fromValue(_items[index.row()]);
    }

    return QVariant();
}

void ShapesListModel::update(const PluginInput &input)
{
    beginResetModel();
    _items.clear();
    endResetModel();

    foreach (Shape shape, input.shapes)
    {
        int index = indexOf(shape.name());
        if (index == -1)
        {
            ShapesListItem item;
            item.shape = shape;
            item.amount = 1;
            _items << item;
        }
        else
        {
            ++_items[index].amount;
        }
    }
    emit dataChanged(index(0, 0), index(0, _items.size() - 1));
}

int ShapesListModel::indexOf(const QString &name) const
{
    for (int i = 0; i < _items.size(); ++i)
    {
        if (_items[i].shape.name() == name)
        {
            return i;
        }
    }
    return -1;
}

bool ShapesListModel::empty() const { return _items.empty(); }

ShapesListItem *ShapesListModel::pointer(int row) { return &_items[row]; }

void ShapesListModel::replace(int row, const ShapesListItem &item)
{
    _items[row] = item;
    emit dataChanged(index(0, row), index(0, row));
}

void ShapesListModel::clear()
{
    int rows = _items.size();
    _items.clear();
    emit dataChanged(index(0, 0), index(0, rows));
}

QList<Shape> ShapesListModel::expand() const
{
    QList<Shape> shapes;
    for (int i = 0; i < _items.size(); ++i)
    {
        for (int j = 0; j < _items[i].amount; j++)
        {
            shapes << _items[i].shape;
        }
    }
    return shapes;
}

QList<QString> ShapesListModel::names() const
{
    QList<QString> result;
    for (int i = 0; i < _items.size(); ++i)
    {
        result << _items[i].shape.name();
    }
    return result;
}

void ShapesListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QBrush foreground(option.palette.text());
    painter->setPen(foreground.color());
    painter->setBrush(foreground);

    ShapesListItem item = qvariant_cast<ShapesListItem>(index.data());
    QString text(QString("Name: %1\nAmount: %2").arg(item.shape.name()).arg(item.amount));
    painter->drawText(option.rect.adjusted(95, 5, 0, 0), Qt::AlignCenter, text);

    QPainterPath path;
    Shape shape = item.shape.normalized();
    QRect bounds = shape.boundingRect();
    qint32 base = qMax(bounds.width(), bounds.height());
    qreal scale = 80.0 / base;
    shape.scale(scale, scale);
    shape.translate(option.rect.topLeft() + QPoint(5, 5));
    path.addPolygon((QPolygon)shape);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(path, foreground);
    painter->setRenderHint(QPainter::Antialiasing, false);
}

QSize ShapesListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), 90);
}

void PluginInputDialog::on_btnEnableAll_clicked() { setAllPluginsCheckState(Qt::Checked); }

void PluginInputDialog::on_btnDisableAll_clicked() { setAllPluginsCheckState(Qt::Unchecked); }

void PluginInputDialog::on_btnLoad_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Input file", ".");
    if (fileName.isEmpty())
    {
        return;
    }
    bool ok;
    PluginInput input = BakeryGUIHelpers::loadFromFile(fileName, &ok);
    if (!ok)
    {
        setStatus(tr("Failed to load input file."));
        return;
    }
    update(input);
    setStatus(tr("Input file loaded."));
}

void PluginInputDialog::on_btnRemove_clicked() { _shapesListModel.removeRow(selectedShapeRow()); }

void PluginInputDialog::on_btnAdd_clicked()
{
    ShapesListItem item;
    item.shape.setName(tr("Shape"));
    ShapeDialog shapeDialog(this);
    shapeDialog.setItem(&item);
    shapeDialog.setUsedShapeNames(_shapesListModel.names());
    if (shapeDialog.exec())
    {
        _shapesListModel.append(item);
    }
}

void PluginInputDialog::setInput(PluginInput *input)
{
    _input = input;
    update(*input);
}

void PluginInputDialog::update(PluginInput input)
{
    _shapesListModel.update(input);
    _ui->leSheetWidth->setText(QString::number(BakeryHelpers::qrealRounded(input.sheetWidth)));
    _ui->leSheetHeight->setText(QString::number(BakeryHelpers::qrealRounded(input.sheetHeight)));
}

void PluginInputDialog::on_btnEdit_clicked() { editShape(selectedShapeRow()); }

void PluginInputDialog::on_btnSave_clicked()
{
    // File name
    QString fileName = QFileDialog::getSaveFileName(this, "Output file", ".");
    if (fileName.isEmpty())
    {
        return;
    }

    // Store
    QFile outputFile(fileName);
    outputFile.open(QFile::ReadWrite | QFile::Text);
    if (!Bakery::saveToDevice(*_input, &outputFile))
    {
        setStatus(tr("Failed to save output file."));
    }
    else
    {
        setStatus(tr("Output file saved."));
    }
}

void PluginInputDialog::on_btnClear_clicked() { _shapesListModel.clear(); }

void PluginInputDialog::on_btnStart_clicked()
{
    for (int i = 0; i < _ui->listWidgetPlugins->count(); ++i)
    {
        QListWidgetItem *item = _ui->listWidgetPlugins->item(i);
        _bakery.setPluginEnabled(item->text(), item->checkState() == Qt::Checked);
    }

    accept();
}

void PluginInputDialog::on_btnEnable_clicked() { setSelectedPluginsCheckState(Qt::Checked); }

void PluginInputDialog::on_btnDisable_clicked() { setSelectedPluginsCheckState(Qt::Unchecked); }

void PluginInputDialog::on_listViewShapes_doubleClicked(const QModelIndex &index) { editShape(index.row()); }

void PluginInputDialog::on_btnLoadSVG_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Input file", ".");
    if (fileName.isEmpty())
    {
        return;
    }
    bool ok;
    PluginInput input = BakeryGUIHelpers::loadFromSVG(fileName, &ok);
    if (!ok)
    {
        setStatus(tr("Failed to load SVG file."));
        return;
    }
    update(input);
    setStatus(tr("SVG file loaded."));
}

void PluginInputDialog::on_btnRandom_clicked()
{
    RandomPluginInputParameters parameters;
    PluginInput random = Bakery::randomInput(parameters);
    update(random);
}
