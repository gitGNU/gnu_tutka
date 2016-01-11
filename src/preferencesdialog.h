/*
 * preferencesdialog.h
 *
 * Copyright 2002-2016 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QHash>

namespace Ui {
    class PreferencesDialog;
}

class Player;
class OutputMidiInterfacesTableModel;
class InputMidiInterfacesTableModel;
class Scheduler;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(Player *player, QWidget *parent = 0);
    ~PreferencesDialog();

public slots:
    void makeVisible();

private slots:
    void setSchedulingMode(const QString &name);
    void setScheduler(int index);
    void enableInterfaces();
    void saveSettings();

private:
    Player *player;
    Ui::PreferencesDialog *ui;
    QSettings settings;
    OutputMidiInterfacesTableModel *outputMidiInterfacesTableModel;
    InputMidiInterfacesTableModel *inputMidiInterfacesTableModel;
    QHash<int, Scheduler *> schedulers;
};

#endif // PREFERENCESDIALOG_H
