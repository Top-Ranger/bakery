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

#include "pluginoutputwidget.h"
#include "ui_pluginoutputwidget.h"

PluginOutputWidget::PluginOutputWidget(QWidget *parent, const QString pluginName, const QHash<QString, QColor> &shapeColors)
    : QWidget(parent), _ui(new Ui::PluginOutputWidget), _pluginName(pluginName), _shapeColors(shapeColors), _scale(50)
{
    _ui->setupUi(this);
}

PluginOutputWidget::~PluginOutputWidget() { delete _ui; }

void PluginOutputWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, _buffer.size().width(), _buffer.size().height(), _buffer);
}

void PluginOutputWidget::setScale(int scale)
{
    _scale = scale;
    updateView();
}

QSize PluginOutputWidget::sizeHint() const { return _buffer.size(); }

void PluginOutputWidget::setOutput(PluginOutput output)
{
    _output = output;
    updateView();
}

void PluginOutputWidget::setState(PluginOutputWidget::State state)
{
    _state = state;
    updateView();
}

void PluginOutputWidget::updateView()
{
    // Constants
    int margin = 5;
    int headerHeight = 25;

    // Computed
    int sheetsCount = _output.sheets.size();
    double dimension = (double)_scale / 100.0 * 500.0;
    double widthRounded = 0, heightRounded = 0;
    int sheetWidth = 0, sheetHeight = 0, bufferWidth = 300, bufferHeight = headerHeight + margin * 2;
    if (sheetsCount > 0)
    {
        widthRounded = (double)(_output.sheets[0].width() / BAKERY_PRECISION);
        heightRounded = (double)(_output.sheets[0].height() / BAKERY_PRECISION);
        sheetWidth = widthRounded > heightRounded ? (int)dimension : (int)(dimension * widthRounded / heightRounded);
        sheetHeight = heightRounded > widthRounded ? (int)dimension : (int)(dimension * heightRounded / widthRounded);
        bufferWidth = qMax(400, (sheetWidth + margin) * sheetsCount + margin);
        bufferHeight = headerHeight + sheetHeight + margin * 3;
    }

    // Initialize and clear buffer
    _buffer = QPixmap(bufferWidth, bufferHeight);
    _buffer.fill(Qt::transparent);
    QPainter painter(&_buffer);

    // Paint header
    QRect headerRect(margin, margin, bufferWidth - margin * 2, headerHeight);
    QColor headerColor;
    QColor headerTextColor(0, 0, 0);
    QString headerText = _pluginName;
    QStringList tags;
    tags << tr("%1 sheet(s)").arg(_output.sheets.size());
    switch (_state)
    {
    case Idle:
        tags << tr("idle");
        headerColor = QColor(255, 255, 255);
        break;
    case Working:
        tags << tr("working");
        headerColor = QColor(255, 255, 64);
        break;
    case Terminating:
        tags << tr("terminating");
        headerColor = QColor(255, 255, 64);
        break;
    case Valid:
        tags << tr("valid");
        headerColor = QColor(64, 255, 64);
        break;
    case Invalid:
        tags << tr("invalid");
        headerTextColor = QColor(255, 255, 255);
        headerColor = QColor(255, 64, 64);
        break;
    }
    tags << tr("score: %1").arg(BakeryPlugins::outputScore(_output));
    headerText += " - " + tags.join(", ");

    painter.fillRect(headerRect, headerColor);
    painter.drawRect(headerRect);
    painter.setPen(QPen(headerTextColor));
    painter.drawText(headerRect.adjusted(5, 0, 0, 0), Qt::AlignVCenter, headerText);

    int index = 0;
    painter.setPen(palette().foreground().color());
    foreach (Sheet sheet, _output.sheets)
    {
        QRect shapeRect(index * (sheetWidth + margin) + margin, headerHeight + margin * 2, sheetWidth, sheetHeight);
        painter.fillRect(shapeRect, palette().background());
        foreach (Shape shape, sheet.shapes())
        {
            // Create painter path
            QPainterPath path;
            double scaleX = 1 / BAKERY_PRECISION / widthRounded * sheetWidth;
            double scaleY = 1 / BAKERY_PRECISION / heightRounded * sheetHeight;
            Shape transformed = shape.scaled(scaleX, scaleY);
            transformed.translate(shapeRect.topLeft());
            path.addPolygon((QPolygon)transformed);

            // Draw shape
            QBrush background = QBrush(_shapeColors[shape.name()]);
            painter.fillPath(path, background);
            painter.setPen(QPen(QColor(0, 0, 0)));
            painter.drawPolygon((QPolygon)transformed);
        }

        // Draw sheet rectangle
        painter.drawRect(shapeRect);

        ++index;
    }

    repaint();
}
