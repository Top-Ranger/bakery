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

#include "shapedialog.h"
#include "ui_shapedialog.h"

#include <QScrollBar>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>

ShapeDialog::ShapeDialog(QWidget *parent) : QDialog(parent), _ui(new Ui::ShapeDialog), _valid(false)
{
    _ui->setupUi(this);
    connect(_ui->shapeWidget, SIGNAL(shapeUpdated()), this, SLOT(updateControls()));
    connect(_ui->shapeWidget, SIGNAL(stateChanged()), this, SLOT(shapeWidgetStateChanged()));
    connect(_ui->scrollAreaShapeWidget->horizontalScrollBar(), SIGNAL(valueChanged(int)), _ui->shapeWidget, SLOT(repaint()));
    connect(_ui->scrollAreaShapeWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), _ui->shapeWidget, SLOT(repaint()));
}

ShapeDialog::~ShapeDialog() { delete _ui; }

void ShapeDialog::on_btnLoad_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Shape file", ".");
    if (fileName.isEmpty())
    {
        return;
    }
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        setStatus(tr("Failed to load shape file."));
        return;
    }
    QTextStream stream(&file);
    Shape shape;
    stream >> shape;
    if (stream.status() != QTextStream::Ok)
    {
        setStatus(tr("Failed to load shape file."));
        return;
    }
    _item->shape = shape;
    setItem(_item);
    setStatus(tr("Shape file loaded."));
}

void ShapeDialog::on_btnSave_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Shape file", ".");
    if (fileName.isEmpty())
    {
        return;
    }
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        setStatus(tr("Failed to save shape file."));
        return;
    }
    QTextStream stream(&file);
    stream << _item->shape;
    setStatus(tr("Shape file saved."));
}

void ShapeDialog::resolutionChanged(int index)
{
    qreal resolution;
    switch (index)
    {
    case 0:
        resolution = 1;
        break;
    case 1:
        resolution = 0.1;
        break;
    case 2:
        resolution = 0.01;
        break;
    default:
        resolution = 0.1;
        break;
    }
    _ui->shapeWidget->setResolution(resolution);
}

void ShapeDialog::validate()
{
    if (_item->shape.size() < 3)
    {
        setValid(false, tr("Shape has less than 3 points."));
        return;
    }
    Shape closedShape(_item->shape);
    closedShape.ensureClosed();
    if (closedShape.area() == 0)
    {
        setValid(false, tr("Shape area is 0."));
        return;
    }
    if (_ui->lineEditName->text().isEmpty())
    {
        setValid(false, tr("Shape name is empty."));
        return;
    }
    if (_usedShapeNames.contains(_ui->lineEditName->text()))
    {
        setValid(false, tr("Shape name is already in use."));
        return;
    }
    _item->shape.setName(_ui->lineEditName->text());

    if (_ui->lineEditAmount->text().isEmpty())
    {
        setValid(false, tr("Amount is empty."));
        return;
    }
    bool ok;
    int amount = _ui->lineEditAmount->text().toInt(&ok);
    if (!ok)
    {
        setValid(false, tr("Amount is not an integer."));
        return;
    }
    if (amount < 1)
    {
        setValid(false, tr("Amount is less than 1."));
        return;
    }
    _item->amount = amount;

    setValid(true, tr("Shape is valid."));
}

void ShapeDialog::updateControls()
{
    validate();

    _ui->btnOK->setEnabled(_valid);
    _ui->btnSave->setEnabled(_valid);
}

void ShapeDialog::shapeWidgetStateChanged()
{
    QString status;
    QPointF h = _ui->shapeWidget->hoverCoordinate();
    int d = _ui->shapeWidget->dragPointIndex();
    if (h != QPointF(-1, -1))
    {
        status = QString("%1, %2%3").arg(h.x()).arg(h.y()).arg(d != -1 ? " (dragging)" : "");
    }
    _ui->lShapeWidgetStatus->setText(status);
}

void ShapeDialog::setItem(ShapesListItem *item)
{
    _item = item;
    _ui->shapeWidget->setShape(&_item->shape);
    _ui->lineEditName->setText(_item->shape.name() == Shape::defaultName() ? QString() : _item->shape.name());
    _ui->lineEditAmount->setText(QString::number(_item->amount));
    connect(_ui->shapeWidget, SIGNAL(shapeUpdated()), this, SLOT(updateControls()));

    updateControls();
}

void ShapeDialog::setUsedShapeNames(const QList<QString> &usedShapeNames) { _usedShapeNames = usedShapeNames; }

void ShapeDialog::setValid(bool valid, QString statusMessage)
{
    _valid = valid;
    setStatus(statusMessage);
}

void ShapeDialog::setStatus(QString message)
{
    _ui->lStatus->setText(message);
    _ui->lStatus->setForegroundRole(_valid ? QPalette::Text : QPalette::Highlight);
    _ui->lStatus->setBackgroundRole(_valid ? QPalette::Text : QPalette::Highlight);
}

void ShapeDialog::on_btnOK_clicked()
{
    _item->shape.ensureClosed();
    accept();
}

void ShapeDialog::on_btnNormalize_clicked()
{
    _ui->shapeWidget->normalizeShape();
    repaint();
}
