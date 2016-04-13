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

#include "savedialog.h"
#include "ui_savedialog.h"

#include <QFileDialog>

SaveDialog::SaveDialog(QWidget *parent) : QDialog(parent), _ui(new Ui::SaveDialog), _valid(false)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowContextHelpButtonHint);
}

void SaveDialog::setOptions(SaveDialog::Options *options)
{
    _options = options;
    _ui->leOutputDirectoryPath->setText(options->outputDirectoryPath);
    _ui->leResultsFileName->setText(options->resultsFileName);
    _ui->rbSaveAll->setChecked(options->saveAll);
    _ui->rbSaveBest->setChecked(!options->saveAll);
    _ui->cbSaveSVGs->setChecked(options->saveSVGs);

    validate();
}

void SaveDialog::validate()
{
    if (_ui->leResultsFileName->text().isEmpty())
    {
        setValid(false, tr("Results file name is empty."));
        return;
    }
    _options->resultsFileName = _ui->leResultsFileName->text();

    QFileInfo fileInfo(_ui->leOutputDirectoryPath->text());
    if (!fileInfo.isWritable())
    {
        setValid(false, tr("Output directory is not writable."));
        return;
    }
    _options->outputDirectoryPath = _ui->leOutputDirectoryPath->text();

    _options->saveAll = _ui->rbSaveAll->isChecked();
    _options->saveSVGs = _ui->cbSaveSVGs->isChecked();

    setValid(true, tr("Options are valid."));
    updateControls();
}

void SaveDialog::setStatus(QString message)
{
    _ui->lStatus->setText(message);
    _ui->lStatus->setForegroundRole(_valid ? QPalette::Text : QPalette::Highlight);
    _ui->lStatus->setBackgroundRole(_valid ? QPalette::Text : QPalette::Highlight);
}

void SaveDialog::setValid(bool valid, QString statusMessage)
{
    _valid = valid;
    setStatus(statusMessage);
}

SaveDialog::~SaveDialog() { delete _ui; }

void SaveDialog::updateControls() { _ui->btnSave->setEnabled(_valid); }

void SaveDialog::on_btnBrowse_clicked()
{
    QString outputDirectoryPath = QFileDialog::getExistingDirectory(this, tr("Output directory"));
    if (outputDirectoryPath.isEmpty())
    {
        return;
    }
    _ui->leOutputDirectoryPath->setText(outputDirectoryPath);
}

void SaveDialog::on_btnSave_clicked() { accept(); }
