/*
 * main.cpp
 *
 * Copyright 2002-2012 Vesa Halttunen
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

/*
 * TODO
 *
 * Don't accept input from disabled inputs (ALSA)
 * Create Core MIDI source and allow that to be used as an output
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#ifdef __APPLE__
#include "coremidi.h"
#elif __linux
#include "alsamidi.h"
#else
#include "midi.h"
#endif
#include "player.h"
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTranslator translator;
    translator.load(QString("tutka_") + QLocale::system().name());
    app.installTranslator(&translator);

#ifdef __APPLE__
    MIDI *midi = new CoreMIDI;
#elif __linux
    MIDI *midi = new AlsaMIDI;
#else
    MIDI *midi = new MIDI;
#endif
    Player *player = new Player(midi, argc > 1 ? argv[1] : QString());
    player->setScheduler(Player::SchedulingRTC);
    MainWindow *mainWindow = new MainWindow(player);
    mainWindow->show();
    mainWindow->raise();

    int returnCode = app.exec();

    delete mainWindow;
    delete player;
    delete midi;

    return returnCode;
}
