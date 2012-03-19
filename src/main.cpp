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
 * Dialog selection gets removed when inserting/removing
 * Playing sequence allows setting an inexisting block
 * Playing sequence uses block numbers 0-> when they're 1->
 * Position label does not change when modifying current playing sequence length
 * Playing sequence list has no selection by default
 * Playing sequence list allows pressing Delete on the last sequence
 * Section list allows pressing Delete on the last position
 * Section list allows setting an inexisting playing sequence
 * Cursor keys don't work as expected when playing
 * Block list allows setting tracks, length and command pages to 0
 * Track->Delete is not disabled if there's 1 track
 * Preferences->Scheduling mode does not reflect reality if RTC is not available
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
