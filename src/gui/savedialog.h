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

#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include <QDialog>

namespace Ui
{
/*!
 * \brief Used to store PluginOutputs.
 */
class SaveDialog;
}

/*!
 * \brief Used to store PluginOutputs.
 */

class SaveDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Save options.
     */
    struct Options
    {
        /*!
         * \brief Path to the output directory.
         */
        QString outputDirectoryPath;

        /*!
         * \brief Name of the results file.
         */
        QString resultsFileName;

        /*!
         * \brief If true save the output of all plugins.
         */
        bool saveAll;

        /*!
         * \brief If true save SVG files.
         */
        bool saveSVGs;

        // Default options
        Options()
        {
            outputDirectoryPath = ".";
            resultsFileName = "results.txt";
            saveAll = false;
            saveSVGs = false;
        }
    };

    /*!
     * \brief Constructor.
     * \param parent QWidget parent.
     */
    explicit SaveDialog(QWidget *parent = 0);

    /*!
     * \brief Setter for member _options;
     * \param options Pointer to SaveDialog::Options struct.
     */
    void setOptions(Options *options);

    /*!
     * \brief Destructor.
     */
    ~SaveDialog();

private slots:
    /*!
     * \brief Validates user input and updates _options with successfully validated values.
     */
    void validate();

    /*!
     * \brief Updates actions.
     */
    void updateControls();

    /*!
     * \brief Invokes the system's file chooser dialog. If accepted updates _options.
     */
    void on_btnBrowse_clicked();

    /*!
     * \brief Accepts the dialog.
     */
    void on_btnSave_clicked();

private:
    /*!
     * \brief Pointer to UI container class instance.
     */
    Ui::SaveDialog *_ui;

    /*!
     * \brief Save options to be edited.
     */
    Options *_options;

    /*!
     * \brief true if user input is valid.
     */
    bool _valid;

    /*!
     * \brief Sets the dialog's status message.
     * \param message Status message.
     */
    void setStatus(QString message);

    /*!
     * \brief Sets the dialog's validity state and status message.
     * \param valid Validity state.
     * \param statusMessage Status message.
     */
    void setValid(bool valid, QString statusMessage);
};

#endif // SAVEDIALOG_H
